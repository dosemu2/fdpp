/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2023  @stsp
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

#ifndef LOADER_H
#define LOADER_H

#include <stdint.h>

#define FDPP_LDR_VER 2

const char *FdppKernelDir(void);
const char *FdppKernelMapName(void);

struct fdpp_bss_ent {
    int off;
    int len;
};

struct fdpp_bss_list {
    int num;
    struct fdpp_bss_ent ent[0];
};

void *FdppKernelLoad(const char *dname, int *len, struct fdpp_bss_list **bss,
                     uint32_t *_start);
uint16_t FdppGetLoadSeg(void *handle);
typedef void (*reloc_hook_t)(uint16_t, int (*)(void *, const char *), void *);
const void *FdppKernelReloc(void *handle, uint16_t seg, uint16_t *r_seg,
                            reloc_hook_t hook);
void FdppKernelFree(void *handle);

#endif
