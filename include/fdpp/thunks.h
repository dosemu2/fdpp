/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2018  Stas Sergeev (stsp)
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

#ifndef THUNKS_H
#define THUNKS_H

#include <stdint.h>
#include <stdarg.h>

#define FDPP_API_VER 35

#ifndef FDPP
struct fdpp_far_s {
    uint16_t off;
    uint16_t seg;
};
typedef struct fdpp_far_s fdpp_far_t;
#else
#define fdpp_far_t far_t
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct vm86_regs;
int FdppCall(struct vm86_regs *regs);
int FdppCtrl(int idx, struct vm86_regs *regs);

enum { FDPP_PRINT_LOG, FDPP_PRINT_TERMINAL, FDPP_PRINT_SCREEN };
enum { ASM_CALL_OK, ASM_CALL_ABORT };
enum { FDPP_RET_ABORT = -1, FDPP_RET_OK, FDPP_RET_NORET };

struct fdpp_api {
    uint8_t *(*so2lin)(uint16_t seg, uint16_t off);
    void (*exit)(int rc);
    void (*abort)(const char *file, int line);
    void (*print)(int prio, const char *format, va_list ap);
    void (*debug)(const char *msg);
    void (*panic)(const char *msg);
    int (*cpu_relax)(void);
    int (*ping)(void);
    int (*asm_call)(struct vm86_regs *regs, uint16_t seg,
        uint16_t off, uint8_t *sp, uint8_t len);
    void (*asm_call_noret)(struct vm86_regs *regs, uint16_t seg,
        uint16_t off, uint8_t *sp, uint8_t len);
    void (*fmemcpy)(fdpp_far_t d, fdpp_far_t s, size_t n);
    void (*n_fmemcpy)(fdpp_far_t d, const void *s, size_t n);
    void (*fmemset)(fdpp_far_t d, int ch, size_t n);
    void (*relocate_notify)(uint16_t oldseg, uint16_t newseg,
        uint16_t startoff, uint32_t blklen);
    void (*mark_mem)(fdpp_far_t p, uint16_t size, int type);
    void (*prot_mem)(fdpp_far_t p, uint16_t size, int type);
    int (*is_dos_space)(const void *ptr);
};
int FdppInit(struct fdpp_api *api, int ver, int *req_ver);
void FdppLoaderHook(uint16_t seg, int (*getsymoff)(void *, const char *),
        void *arg);
const char *FdppVersionString(void);

#ifdef __cplusplus
}
#endif

#endif
