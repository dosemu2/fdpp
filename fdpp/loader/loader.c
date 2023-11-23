/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2018-2023  Stas Sergeev (stsp)
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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "elf_priv.h"
#include "loader.h"

#define __S(x) #x
#define _S(x) __S(x)
const char *FdppKernelDir(void)
{
    return _S(FDPPKRNLDIR);
}

const char *FdppKernelMapName(void)
{
    return _S(KRNL_MAP_NAME);
}

struct krnl_hndl {
    void *elf;
    const void *start;
    unsigned load_off;
};

void *FdppKernelLoad(const char *dname, int *len, struct fdpp_bss_list **bss,
                     uint32_t *_start)
{
    struct fdpp_bss_list *bl;
    void *start, *end;
    void *bstart, *bend;
    void *ibstart, *ibend;
    char *kname;
    void *handle;
    struct krnl_hndl *h;
    int i, s, rc;

    rc = asprintf(&kname, "%s/%s", dname, _S(KRNL_ELFNAME));
    assert(rc != -1);
    handle = elf_open(kname);
    if (!handle) {
        fprintf(stderr, "failed to open %s\n", kname);
        free(kname);
        return NULL;
    }
    free(kname);
    start = elf_getsym(handle, "_start");
    s = elf_getsymoff(handle, "_start");
    if (s == -1)
        goto err_close;
    *_start = s;
    end = elf_getsym(handle, "__InitTextEnd");
    if (!end)
        goto err_close;
    bstart = elf_getsym(handle, "__bss_start");
    if (!bstart)
        goto err_close;
    bend = elf_getsym(handle, "__bss_end");
    if (!bend)
        goto err_close;
    ibstart = elf_getsym(handle, "__ibss_start");
    if (!ibstart)
        goto err_close;
    ibend = elf_getsym(handle, "__ibss_end");
    if (!ibend)
        goto err_close;
    *len = (uintptr_t)end - (uintptr_t)start;

    bl = (struct fdpp_bss_list *)malloc(sizeof(*bl) +
            sizeof(struct fdpp_bss_ent) * 2);
    i = 0;
    if (bend != bstart) {
        bl->ent[i].off = (uintptr_t)bstart - (uintptr_t)start;
        bl->ent[i].len = (uintptr_t)bend - (uintptr_t)bstart;
        i++;
    }
    if (ibend != ibstart) {
        bl->ent[i].off = (uintptr_t)ibstart - (uintptr_t)start;
        bl->ent[i].len = (uintptr_t)ibend - (uintptr_t)ibstart;
        i++;
    }
    bl->num = i;
    *bss = bl;

    h = (struct krnl_hndl *)malloc(sizeof(*h));
    h->elf = handle;
    h->start = start;
    h->load_off = s;
    return h;

err_close:
    elf_close(handle);
    return NULL;
}

const void *FdppKernelReloc(void *handle, uint16_t seg, uint16_t *r_seg,
        reloc_hook_t hook)
{
    struct krnl_hndl *h = (struct krnl_hndl *)handle;

    assert(!(h->load_off & 0xf));
    seg -= h->load_off >> 4;
    elf_reloc(h->elf, seg);

    hook(seg, elf_getsymoff, h->elf);

    *r_seg = seg;
    return h->start;
}

void FdppKernelFree(void *handle)
{
    struct krnl_hndl *h = (struct krnl_hndl *)handle;

    elf_close(h->elf);
    free(h);
}
