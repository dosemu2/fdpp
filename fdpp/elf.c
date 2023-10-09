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

/* TODO: write proper d/l support! Get rid of the external reloc table! */

struct elfstate {
    char *addr;
    size_t mapsize;
    Elf *elf;
    Elf_Scn *symtab_scn;
    GElf_Shdr symtab_shdr;
    uint32_t load_offs;
};

static void elf_dl(char *addr, uint16_t seg)
{
    const int reloc_tab[] = {
        #include "rel.h"
    };
    size_t i;
#define _countof(array) (sizeof(array) / sizeof(array[0]))

    for (i = 0; i < _countof(reloc_tab); i++)
        memcpy(&addr[reloc_tab[i]-1], &seg, sizeof(seg));
}

void *elf_open(const char *name)
{
    Elf         *elf;
    Elf_Scn     *scn = NULL;
    GElf_Shdr   shdr;
    GElf_Phdr   phdr;
    int         fd;
    int         i;
    uint32_t    load_offs;
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

    for (i = 0, load_offs = 0; gelf_getphdr(elf, i, &phdr); i++) {
        if (phdr.p_type != PT_LOAD)
            continue;
        load_offs = phdr.p_offset;
        break;
    }
    if (!load_offs) {
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

    elf_dl(state->addr, seg);
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
    Elf_Data *data;
    int count, i;

    data = elf_getdata(state->symtab_scn, NULL);
    count = state->symtab_shdr.sh_size / state->symtab_shdr.sh_entsize;

    for (i = 0; i < count; i++) {
        GElf_Sym sym;
        gelf_getsym(data, i, &sym);
        if (strcmp(elf_strptr(state->elf, state->symtab_shdr.sh_link,
                sym.st_name), name) == 0)
            return sym.st_value;
    }

    return -1;
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
