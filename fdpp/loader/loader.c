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
    unsigned start;
    uint16_t load_seg;
};

void *FdppKernelLoad(const char *dname, int *len, struct fdpp_bss_list **bss,
                     uint32_t *_start)
{
    struct fdpp_bss_list *bl;
    int start, end;
    int bstart, bend;
    int ibstart, ibend;
    char *kname;
    void *handle;
    struct krnl_hndl *h;
    int i, ls, rc;

    rc = asprintf(&kname, "%s/%s", dname, _S(KRNL_ELFNAME));
    assert(rc != -1);
    handle = fdelf_open(kname);
    if (!handle) {
        fprintf(stderr, "failed to open %s\n", kname);
        free(kname);
        return NULL;
    }
    free(kname);
    start = fdelf_getsymoff(handle, "_start");
    if (start == -1)
        goto err_close;
    *_start = start;
    end = fdelf_getsymoff(handle, "__InitTextEnd");
    if (end == -1)
        goto err_close;
    bstart = fdelf_getsymoff(handle, "__bss_start");
    if (bstart == -1)
        goto err_close;
    bend = fdelf_getsymoff(handle, "__bss_end");
    if (bend == -1)
        goto err_close;
    ibstart = fdelf_getsymoff(handle, "__ibss_start");
    if (ibstart == -1)
        goto err_close;
    ibend = fdelf_getsymoff(handle, "__ibss_end");
    if (ibend == -1)
        goto err_close;

    ls = fdelf_getsymoff(handle, "_LOADSEG");
    if (ls == -1 || ls > 0xffff)
        goto err_close;
    /* if we are relocatable (ls==0), then skip garbage before "start" */
    *len = end - (ls ? 0 : start);

    bl = (struct fdpp_bss_list *)malloc(sizeof(*bl) +
            sizeof(struct fdpp_bss_ent) * 2);
    i = 0;
    if (bend != bstart) {
        bl->ent[i].off = bstart - start;
        bl->ent[i].len = bend - bstart;
        i++;
    }
    if (ibend != ibstart) {
        bl->ent[i].off = ibstart - start;
        bl->ent[i].len = ibend - ibstart;
        i++;
    }
    bl->num = i;
    *bss = bl;

    h = (struct krnl_hndl *)malloc(sizeof(*h));
    h->elf = handle;
    h->start = start;
    h->load_seg = ls;
    return h;

err_close:
    fdelf_close(handle);
    return NULL;
}

uint16_t FdppGetLoadSeg(void *handle)
{
    struct krnl_hndl *h = (struct krnl_hndl *)handle;

    return h->load_seg;
}

const void *FdppKernelReloc(void *handle, uint16_t seg, uint16_t *r_seg,
        reloc_hook_t hook)
{
    struct krnl_hndl *h = (struct krnl_hndl *)handle;
    char *la = fdelf_getloadaddr(h->elf);

    assert(!(h->start & 0xf));
    /* if relocatable then skip up to "start" to save DOS mem */
    if (!h->load_seg) {
        seg -= h->start >> 4;
        fdelf_reloc(h->elf, seg);
    } else {
        seg = h->load_seg;
    }
    hook(seg, fdelf_getsymoff, h->elf);

    *r_seg = seg;
    return (la + (h->load_seg ? 0 : h->start));
}

void FdppKernelFree(void *handle)
{
    struct krnl_hndl *h = (struct krnl_hndl *)handle;

    fdelf_close(h->elf);
    free(h);
}
