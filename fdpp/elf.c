/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2021  Stas Sergeev (stsp)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <libelf.h>
#include <gelf.h>
#include "elf_priv.h"

struct elfstate {
    char *addr;
    size_t mapsize;
    Elf *elf;
    Elf_Scn *symtab_scn;
    GElf_Shdr symtab_shdr;
    uint32_t load_offs;
};

static int do_getsym(struct elfstate *state, const char *name, GElf_Sym *r_sym)
{
    Elf_Data *data;
    int count, i;

    data = elf_getdata(state->symtab_scn, NULL);
    count = state->symtab_shdr.sh_size / state->symtab_shdr.sh_entsize;

    for (i = 0; i < count; i++) {
        GElf_Sym sym;
        gelf_getsym(data, i, &sym);
        if (strcmp(elf_strptr(state->elf, state->symtab_shdr.sh_link,
                sym.st_name), name) == 0) {
            *r_sym = sym;
            return 0;
        }
    }

    return -1;
}

static void do_elf_dl(struct elfstate *state, uint16_t seg, Elf_Scn *rel_scn,
                      GElf_Shdr *rel_shdr)
{
    Elf_Data *rel_data;
    Elf_Data *st_data;
    int rel_count, st_count, i;

    rel_data = elf_getdata(rel_scn, NULL);
    rel_count = rel_shdr->sh_size / rel_shdr->sh_entsize;
    st_data = elf_getdata(state->symtab_scn, NULL);
    st_count = state->symtab_shdr.sh_size / state->symtab_shdr.sh_entsize;

    for (i = 0; i < rel_count; i++) {
        uint16_t *val;
        GElf_Rel rel;
        GElf_Sym sym;
        char *name, *name2, *p;
        int rc;

        gelf_getrel(rel_data, i, &rel);
        /* look for R_386_SEG16 */
        if (GELF_R_TYPE(rel.r_info) != R_386_16)
            continue;
        if (GELF_R_SYM(rel.r_info) >= st_count) {
            fprintf(stderr, "bad reloc %lx %i %li off=%lx\n",
                GELF_R_TYPE(rel.r_info), st_count,
                GELF_R_SYM(rel.r_info), rel.r_offset);
            return;
        }
        gelf_getsym(st_data, GELF_R_SYM(rel.r_info), &sym);
#ifndef STT_RELC
#define STT_RELC 8
#endif
        if (GELF_ST_TYPE(sym.st_info) != STT_RELC)
            continue;

        name = elf_strptr(state->elf, state->symtab_shdr.sh_link, sym.st_name);
        /* make sure its a SEG16 reloc symbol */
        if (strncmp(name, ">>:", 3) != 0 || !strstr(name, "04"))
            continue;
        p = strstr(name, ":s");
        if (!p)
            continue;
        p = strchr(p + 1, ':');
        if (!p)
            continue;
        name2 = strdup(p + 1);
        p = strchr(name2, ':');
        if (p)
            *p = '\0';
        rc = do_getsym(state, name2, &sym);
        free(name2);
        if (rc)
            continue;

        /* not relocating against abs symbol */
        if (sym.st_shndx == SHN_ABS)
            continue;
        val = (uint16_t *)(state->addr + state->load_offs + rel.r_offset);
        *val += seg;
    }
}

static void elf_dl(struct elfstate *state, uint16_t seg)
{
    Elf_Scn *rel_scn = NULL;
    while ((rel_scn = elf_nextscn(state->elf, rel_scn)) != NULL) {
        GElf_Shdr rel_shdr;
        gelf_getshdr(rel_scn, &rel_shdr);
        if (rel_shdr.sh_type == SHT_REL)
            do_elf_dl(state, seg, rel_scn, &rel_shdr);
    }
}

void *elf_open(const char *name)
{
    Elf         *elf;
    Elf_Scn     *scn = NULL;
    GElf_Shdr   shdr;
    GElf_Phdr   phdr;
    int         fd;
    int         i;
    int load_offs;
    struct stat st;
    char        *addr;
    size_t      mapsize;
    struct elfstate *ret;

    elf_version(EV_CURRENT);

    fd = open(name, O_RDONLY);
    if (fd == -1) {
        perror("open()");
        return NULL;
    }
    fstat(fd, &st);
    mapsize = (st.st_size + getpagesize() - 1) & ~(getpagesize() - 1);
    addr = (char *)mmap(NULL, mapsize, PROT_READ | PROT_WRITE, MAP_PRIVATE,
        fd, 0);
    close(fd);
    if (addr == MAP_FAILED) {
        perror("mmap()");
        return NULL;
    }
    elf = elf_memory(addr, st.st_size);
    if (!elf) {
        fprintf(stderr, "elf_memory() failed\n");
        goto err2;
    }

    for (i = 0, load_offs = -1; gelf_getphdr(elf, i, &phdr); i++) {
        if (phdr.p_type != PT_LOAD)
            continue;
        load_offs = phdr.p_offset;
        break;
    }
    if (load_offs == -1) {
        fprintf(stderr, "elf: not found PT_LOAD\n");
        goto err;
    }

    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        gelf_getshdr(scn, &shdr);
        if (shdr.sh_type == SHT_SYMTAB) {
            /* found a symbol table, go print it. */
            break;
        }
    }
    if (!scn) {
        fprintf(stderr, "elf: not found SHT_SYMTAB\n");
        goto err;
    }

    ret = (struct elfstate *)malloc(sizeof(*ret));
    ret->addr = addr;
    ret->mapsize = mapsize;
    ret->elf = elf;
    ret->symtab_scn = scn;
    ret->symtab_shdr = shdr;
    ret->load_offs = load_offs;
    return ret;

err:
    elf_end(elf);
err2:
    munmap(addr, mapsize);
    return NULL;
}

void elf_reloc(void *arg, uint16_t seg)
{
    struct elfstate *state = (struct elfstate *)arg;

    elf_dl(state, seg);
}

void elf_close(void *arg)
{
    struct elfstate *state = (struct elfstate *)arg;

    elf_end(state->elf);
    munmap(state->addr, state->mapsize);
    free(state);
}

static int do_getsymoff(struct elfstate *state, const char *name)
{
    GElf_Sym sym;
    int err = do_getsym(state, name, &sym);
    return (err ? -1 : sym.st_value);
}

void *elf_getsym(void *arg, const char *name)
{
    struct elfstate *state = (struct elfstate *)arg;
    int off = do_getsymoff(state, name);
    if (off == -1)
        return NULL;
    return state->addr + state->load_offs + off;
}

int elf_getsymoff(void *arg, const char *name)
{
    struct elfstate *state = (struct elfstate *)arg;
    return do_getsymoff(state, name);
}

void *elf_getloadaddr(void *arg)
{
    struct elfstate *state = (struct elfstate *)arg;
    return state->addr + state->load_offs;
}

int elf_getloadoff(void *arg)
{
    struct elfstate *state = (struct elfstate *)arg;
    return state->load_offs;
}
