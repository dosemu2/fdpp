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

#include <stdlib.h>
#include <string.h>
#include "../hdr/portab.h"
#include "globals.h"
#include "proto.h"

#include "dispatch.hpp"
#include "objtrace.hpp"
#include "thunks_priv.h"
#include "thunks.h"

static struct fdpp_api *fdpp;

struct asm_dsc_s {
    UWORD num;
    UWORD off;
    UWORD seg;
};
struct asm_dsc_s *asm_tab;
static int asm_tab_len;
static farhlp sym_tab;
static struct far_s *near_wrp;
static int num_wrps;
static int recur_cnt;

enum { ASM_OK, ASM_NORET, ASM_ABORT };

typedef void (*FdppAsmCall_t)(struct vm86_regs *regs, uint16_t seg,
        uint16_t off, uint8_t *sp, uint8_t len);

#define _E
#include "glob_tmpl.h"
#undef _E

static union asm_thunks_u {
  struct _thunks {
#define __ASM(t, v) struct far_s * __##v
#define __ASM_FAR(t, v) struct far_s * __##v
#define __ASM_NEAR(t, v) struct far_s * __##v
#define __ASM_ARR(t, v, l) struct far_s * __##v
#define __ASM_ARRI(t, v) struct far_s * __##v
#define __ASM_ARRI_F(t, v) struct far_s * __##v
#define __ASM_FUNC(v) struct far_s * __##v
#define SEMIC ;
#include <glob_asm.h>
#undef __ASM
#undef __ASM_FAR
#undef __ASM_NEAR
#undef __ASM_ARR
#undef __ASM_ARRI
#undef __ASM_ARRI_F
#undef __ASM_FUNC
#undef SEMIC
  } thunks;
  struct far_s * arr[sizeof(struct _thunks) / sizeof(struct far_s *)];
} asm_thunks = {{
#define SEMIC ,
#define __ASM(t, v) __ASMREF(__##v)
#define __ASM_FAR(t, v) __ASMREF(__##v)
#define __ASM_NEAR(t, v) __ASMREF(__##v)
#define __ASM_ARR(t, v, l) __ASMREF(__##v)
#define __ASM_ARRI(t, v) __ASMREF(__##v)
#define __ASM_ARRI_F(t, v) __ASMREF(__##v)
#define __ASM_FUNC(v) __ASMREF(__##v)
#include <glob_asm.h>
#undef __ASM
#undef __ASM_FAR
#undef __ASM_NEAR
#undef __ASM_ARR
#undef __ASM_ARRI
#undef __ASM_ARRI_F
#undef __ASM_FUNC
#undef SEMIC
}};

struct vm86_regs {
/*
 * normal regs, with special meaning for the segment descriptors..
 */
	int ebx;
	int ecx;
	int edx;
	int esi;
	int edi;
	int ebp;
	int eax;
	int __null_ds;
	int __null_es;
	int __null_fs;
	int __null_gs;
	int orig_eax;
	int eip;
	unsigned short cs, __csh;
	int eflags;
	int esp;
	unsigned short ss, __ssh;
/*
 * these are specific to v86 mode:
 */
	unsigned short es, __esh;
	unsigned short ds, __dsh;
	unsigned short fs, __fsh;
	unsigned short gs, __gsh;
};

static struct vm86_regs s_regs;

union dword {
  uint32_t d;
  struct { uint16_t l, h; } w;
  struct { uint8_t l, h, b2, b3; } b;
};

#define LO_WORD(dwrd)    (((union dword *)&(dwrd))->w.l)
#define HI_WORD(dwrd)    (((union dword *)&(dwrd))->w.h)
#define LO_BYTE(dwrd)    (((union dword *)&(dwrd))->b.l)
#define HI_BYTE(dwrd)    (((union dword *)&(dwrd))->b.h)
#define _AL(regs)         LO_BYTE(regs->eax)
#define _AH(regs)         HI_BYTE(regs->eax)
#define _AX(regs)         LO_WORD(regs->eax)
#define _BL(regs)         LO_BYTE(regs->ebx)
#define _BH(regs)         HI_BYTE(regs->ebx)
#define _BX(regs)         LO_WORD(regs->ebx)
#define _CL(regs)         LO_BYTE(regs->ecx)
#define _CH(regs)         HI_BYTE(regs->ecx)
#define _CX(regs)         LO_WORD(regs->ecx)
#define _DL(regs)         LO_BYTE(regs->edx)
#define _DH(regs)         HI_BYTE(regs->edx)
#define _DX(regs)         LO_WORD(regs->edx)

static void *so2lin(uint16_t seg, uint16_t off)
{
    return fdpp->so2lin(seg, off);
}

void *resolve_segoff(struct far_s fa)
{
    return so2lin(fa.seg, fa.off);
}

static int FdppSetAsmThunks(struct far_s *ptrs, int len)
{
    int i;
    int exp = _countof(asm_thunks.arr);

    if (len != exp) {
        fdprintf("len=%i expected %i\n", len, exp);
        return -1;
    }

    farhlp_init(&sym_tab);
    for (i = 0; i < len; i++) {
        *asm_thunks.arr[i] = ptrs[i];
        /* there are conflicts, for example InitTextStart will collide
         * with the first sym. So use _replace. */
        store_far_replace(&sym_tab, resolve_segoff(ptrs[i]), ptrs[i]);
    }

    return 0;
}

struct fdpp_symtab {
    struct far_s text_start;
    struct far_s text_end;
    uint16_t orig_cs;
    uint16_t cur_cs;
    struct far_s symtab;
    uint16_t symtab_len;
    struct far_s calltab;
    uint16_t calltab_len;
    uint16_t num_wrps;
    struct far_s near_wrp[0];
};

static void do_relocs(uint8_t *start_p, uint8_t *end_p, uint16_t delta)
{
    int i;
    int reloc;
    struct asm_dsc_s *t;
    uint8_t *ptr;

    reloc = 0;
    for (i = 0; i < num_wrps; i++) {
        ptr = (uint8_t *)resolve_segoff(near_wrp[i]);
        if (ptr >= start_p && ptr <= end_p) {
            if (delta)
                near_wrp[i].seg += delta;
            else
                near_wrp[i].seg = near_wrp[i].off = 0;
            reloc++;
        }
    }
    fdlogprintf("processed %i relocs\n", reloc);
    t = asm_tab;
    reloc = 0;
    for (i = 0; i < asm_tab_len; i++) {
        ptr = (uint8_t *)so2lin(t[i].seg, t[i].off);
        if (ptr >= start_p && ptr <= end_p) {
            if (delta)
                t[i].seg += delta;
            else
                t[i].seg = t[i].off = 0;
            reloc++;
        }
    }
    fdlogprintf("processed %i relocs\n", reloc);
}

static void FdppSetSymTab(struct vm86_regs *regs, struct fdpp_symtab *symtab)
{
    int err;
    struct far_s *thtab = (struct far_s *)resolve_segoff(symtab->symtab);
    int stab_len = symtab->symtab_len / sizeof(struct far_s);

    num_wrps = symtab->num_wrps;
    free(near_wrp);
    near_wrp = (struct far_s *)malloc(sizeof(struct far_s) * num_wrps);
    memcpy(near_wrp, symtab->near_wrp, sizeof(struct far_s) * num_wrps);
    free(asm_tab);
    asm_tab = (struct asm_dsc_s *)malloc(symtab->calltab_len);
    memcpy(asm_tab, resolve_segoff(symtab->calltab), symtab->calltab_len);
    asm_tab_len = symtab->calltab_len / sizeof(struct asm_dsc_s);
    /* now relocate */
    if (symtab->cur_cs > symtab->orig_cs) {
        int i;
        int reloc;
        uint8_t *start_p = (uint8_t *)resolve_segoff(symtab->text_start);
        uint8_t *end_p = (uint8_t *)resolve_segoff(symtab->text_end);
        uint16_t delta = symtab->cur_cs - symtab->orig_cs;
        fdlogprintf("init reloc %hx --> %hx, %tx\n", symtab->orig_cs,
                symtab->cur_cs, end_p - start_p);
        do_relocs(start_p, end_p, delta);
        /* sym_tab table is patched in non-relocated code, never used later */
        reloc = 0;
        for (i = 0; i < stab_len; i++) {
            uint8_t *ptr = (uint8_t *)resolve_segoff(thtab[i]);
            if (ptr >= start_p && ptr <= end_p) {
                thtab[i].seg += delta;
                reloc++;
            }
        }
        fdlogprintf("processed %i relocs\n", reloc);
    }

    err = FdppSetAsmThunks(thtab, stab_len);
    _assert(!err);
}

#define _ARG(n, t, ap) (*(t *)(ap + n))
#define _ARG_PTR(n, t, ap) // unimplemented, will create syntax error
#define _ARG_PTR_FAR(n, t, ap)  ({ \
    UDWORD __d = *(UDWORD *)(ap + n); \
    FP_FROM_D(t, __d); \
})

static UDWORD FdppThunkCall(int fn, UBYTE *sp, UBYTE *r_len)
{
    UDWORD ret;
    UBYTE rsz = 0;
    enum DispStat stat;

#define _SP sp
#define _DISP_CMN(f, c) { \
    fdlogprintf("dispatch " #f "\n"); \
    objtrace_enter(); \
    c; \
    objtrace_leave(); \
    fdlogprintf("dispatch " #f " done, %i\n", recur_cnt); \
}
#define _DISPATCH(r, f, ...) _DISP_CMN(f, { \
    ret = fdpp_dispatch(&stat, f, ##__VA_ARGS__); \
    if (stat == DISP_OK) \
        rsz = (r); \
    else \
        _assert(ret != ASM_NORET); \
})
#define _DISPATCH_v(f, ...) _DISP_CMN(f, { \
    ret = fdpp_dispatch_v(&stat, f, ##__VA_ARGS__); \
})

    switch (fn) {
        #include <thunk_calls.h>

        default:
            fdprintf("unknown fn %i\n", fn);
            _fail();
            return 0;
    }

    if (r_len)
        *r_len = rsz;
    return ret;
}

static void _FdppCall(struct vm86_regs *regs)
{
    s_regs = *regs;
    UBYTE len;
    UDWORD res;

    if (!fdpp)
        return;

    switch (LO_BYTE(regs->ebx)) {
    case DOS_SUBHELPER_DL_SET_SYMTAB:
        if (HI_BYTE(regs->eax) != FDPP_KERNEL_VERSION) {
            fdloudprintf("\nfdpp version mismatch: expected %i, got %i\n",
                    FDPP_KERNEL_VERSION, HI_BYTE(regs->eax));
            _fail();
        }
        FdppSetSymTab(regs,
                (struct fdpp_symtab *)so2lin(regs->ss, LO_WORD(regs->esp) + 6));
        break;
    case DOS_SUBHELPER_DL_CCALL:
        res = FdppThunkCall(LO_WORD(regs->ecx),
                (UBYTE *)so2lin(regs->ss, LO_WORD(regs->edx)), &len);
        switch (len) {
        case 0:
            break;
        case 1:
            _AL(regs) = res;
            break;
        case 2:
            _AX(regs) = res;
            break;
        case 4:
            _AX(regs) = res & 0xffff;
            _DX(regs) = res >> 16;
            break;
        default:
            _fail();
            break;
        }
        break;
    }
}

void FdppCall(struct vm86_regs *regs)
{
    recur_cnt++;
    _FdppCall(regs);
    recur_cnt--;
}

void do_abort(const char *file, int line)
{
    fdpp->abort(file, line);
}

int FdppInit(struct fdpp_api *api, int ver, int *req_ver)
{
    *req_ver = FDPP_API_VER;
    if (ver != FDPP_API_VER)
        return -1;
    fdpp = api;
    return 0;
}

void fdvprintf(const char *format, va_list vl)
{
    fdpp->print(FDPP_PRINT_TERMINAL, format, vl);
}

static void fdlogvprintf(const char *format, va_list vl)
{
    fdpp->print(FDPP_PRINT_LOG, format, vl);
}

void fdprintf(const char *format, ...)
{
    va_list vl;

    va_start(vl, format);
    fdvprintf(format, vl);
    va_end(vl);
}

void fdlogprintf(const char *format, ...)
{
    va_list vl;

    va_start(vl, format);
    fdlogvprintf(format, vl);
    va_end(vl);
}

void fdloudprintf(const char *format, ...)
{
    va_list vl;

    va_start(vl, format);
    fdpp->print(FDPP_PRINT_LOG, format, vl);
    va_end(vl);
    va_start(vl, format);
    fdpp->print(FDPP_PRINT_TERMINAL, format, vl);
    va_end(vl);
    va_start(vl, format);
    fdpp->print(FDPP_PRINT_SCREEN, format, vl);
    va_end(vl);
}

void cpu_relax(void)
{
    fdpp->cpu_relax();
}

#define _TFLG_NONE 0
#define _TFLG_FAR 1
#define _TFLG_NORET 2

static void _call_wrp(FdppAsmCall_t call, struct vm86_regs *regs, 
        uint16_t seg, uint16_t off, uint8_t *sp, uint8_t len)
{
    _assert(seg || off);
    call(regs, seg, off, sp, len);
}

static uint32_t _do_asm_call_far(int num, uint8_t *sp, uint8_t len,
        FdppAsmCall_t call)
{
    int i;

    for (i = 0; i < asm_tab_len; i++) {
        if (asm_tab[i].num == num) {
            _call_wrp(call, &s_regs, asm_tab[i].seg, asm_tab[i].off, sp, len);
            return (LO_WORD(s_regs.edx) << 16) | LO_WORD(s_regs.eax);
        }
    }
    _assert(0);
    return -1;
}

static uint16_t find_wrp(uint16_t seg)
{
    int i;

    for (i = 0; i < num_wrps; i++) {
        if (near_wrp[i].seg == seg)
            return near_wrp[i].off;
    }
    _assert(0);
    return -1;
}

static uint32_t _do_asm_call(int num, uint8_t *sp, uint8_t len,
        FdppAsmCall_t call)
{
    int i;

    for (i = 0; i < asm_tab_len; i++) {
        if (asm_tab[i].num == num) {
            uint16_t wrp = find_wrp(asm_tab[i].seg);
            LO_WORD(s_regs.eax) = asm_tab[i].off;
            /* argpack should be aligned */
            _assert(!(len & 1));
            LO_WORD(s_regs.ecx) = len >> 1;
            _call_wrp(call, &s_regs, asm_tab[i].seg, wrp, sp, len);
            return (LO_WORD(s_regs.edx) << 16) | LO_WORD(s_regs.eax);
        }
    }
    _assert(0);
    return -1;
}

static void asm_call(struct vm86_regs *regs, uint16_t seg,
        uint16_t off, uint8_t *sp, uint8_t len)
{
    int rc = fdpp->asm_call(regs, seg, off, sp, len);
    switch (rc) {
    case ASM_CALL_OK:
        break;
    case ASM_CALL_ABORT:
        fdlogprintf("reboot jump, %i\n", recur_cnt);
        fdpp_noret(ASM_ABORT);
        break;
    }
}

static void asm_call_noret(struct vm86_regs *regs, uint16_t seg,
        uint16_t off, uint8_t *sp, uint8_t len)
{
    fdpp->asm_call_noret(regs, seg, off, sp, len);
    objtrace_mark();
    fdlogprintf("noret jump, %i\n", recur_cnt);
    fdpp_noret(ASM_NORET);
}

static uint32_t do_asm_call(int num, uint8_t *sp, uint8_t len, int flags)
{
    uint32_t ret;
    FdppAsmCall_t call = ((flags & _TFLG_NORET) ? asm_call_noret : asm_call);
    if (flags & _TFLG_FAR)
        ret = _do_asm_call_far(num, sp, len, call);
    else
        ret = _do_asm_call(num, sp, len, call);
    return ret;
}

static uint8_t *clean_stk(size_t len)
{
    uint8_t *ret = (uint8_t *)so2lin(s_regs.ss, LO_WORD(s_regs.esp));
    s_regs.esp += len;
    return ret;
}

uint16_t getCS(void)
{
    /* we pass CS in SI */
    return LO_WORD(s_regs.esi);
}

void setDS(uint16_t seg)
{
    s_regs.ds = seg;
}

void setES(uint16_t seg)
{
    s_regs.es = seg;
}

uint16_t data_seg(void)
{
    return FP_SEG(__ASMADDR(DATASTART));
}

#define VIF_MASK	0x00080000	/* virtual interrupt flag */
#define VIF VIF_MASK
#define set_IF() (s_regs.eflags |= VIF)
#define clear_IF() (s_regs.eflags &= ~VIF)

void disable(void)
{
    clear_IF();
}

void enable(void)
{
    set_IF();
}

#define __ARG(t) t
#define __ARG_PTR(t) t *
#define __ARG_PTR_FAR(t) __FAR(t)
#define __ARG_A(t) t
#define __ARG_PTR_A(t) NEAR_PTR_DO(t)
#define __ARG_PTR_FAR_A(t) __DOSFAR(t)

#define __CNV_PTR_FAR(t, d, f, l) t d = (f)
#define __CNV_PTR(t, d, f, l) \
    _MK_FAR_SZ(__##d, f, sizeof(*f)); \
    t d = __MK_NEAR2(__##d, t)
#define __CNV_PTR_CCHAR(t, d, f, l) \
    _MK_FAR_STR(__##d, f); \
    t d = __MK_NEAR(__##d)
#define __CNV_PTR_ARR(t, d, f, l) \
    _MK_FAR_SZ(__##d, f, l); \
    t d = __MK_NEAR(__##d)
#define __CNV_PTR_VOID(t, d, f, l) \
    _MK_FAR_SZ(__##d, f, l); \
    t d = __MK_NEAR(__##d)
#define __CNV_SIMPLE(t, d, f, l) t d = (f)

#define _CNV(c, at, l, n) c(at, _a##n, a##n, l)
#define _L_REF(nl) a##nl
#define _L_IMM(n, l) (sizeof(*_L_REF(n)) * (l))

#define _THUNK0_v(n, f, z) \
void f(void) \
{ \
    _assert(n < asm_tab_len); \
    do_asm_call(n, NULL, 0, z); \
}

#define _THUNK0(n, r, f, z) \
r f(void) \
{ \
    _assert(n < asm_tab_len); \
    return do_asm_call(n, NULL, 0, z); \
}

#define _THUNK1_v(n, f, t1, at1, aat1, c1, l1, z) \
void f(t1 a1) \
{ \
    _CNV(c1, at1, l1, 1); \
    struct { \
        aat1 a1; \
    } PACKED _args = { _a1 }; \
    _assert(n < asm_tab_len); \
    do_asm_call(n, (UBYTE *)&_args, sizeof(_args), z); \
    clean_stk(sizeof(_args)); \
}

#define _THUNK2_v(n, f, t1, at1, aat1, c1, l1, t2, at2, aat2, c2, l2, z) \
void f(t1 a1, t2 a2) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    struct { \
	aat1 a1; \
	aat2 a2; \
    } PACKED _args = { _a1, _a2 }; \
    _assert(n < asm_tab_len); \
    do_asm_call(n, (UBYTE *)&_args, sizeof(_args), z); \
    clean_stk(sizeof(_args)); \
}

#define _THUNK2(n, r, f, t1, at1, aat1, c1, l1, t2, at2, aat2, c2, l2, z) \
r f(t1 a1, t2 a2) \
{ \
    r _ret; \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    struct { \
	aat1 a1; \
	aat2 a2; \
    } PACKED _args = { _a1, _a2 }; \
    _assert(n < asm_tab_len); \
    _ret = do_asm_call(n, (UBYTE *)&_args, sizeof(_args), z); \
    clean_stk(sizeof(_args)); \
    return _ret; \
}

#define _THUNK3_v(n, f, t1, at1, aat1, c1, l1, t2, at2, aat2, c2, l2, \
    t3, at3, aat3, c3, l3, z) \
void f(t1 a1, t2 a2, t3 a3) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    struct { \
	aat1 a1; \
	aat2 a2; \
	aat2 a3; \
    } PACKED _args = { _a1, _a2, _a3 }; \
    _assert(n < asm_tab_len); \
    do_asm_call(n, (UBYTE *)&_args, sizeof(_args), z); \
    clean_stk(sizeof(_args)); \
}

#define _THUNK3(n, r, f, t1, at1, aat1, c1, l1, t2, at2, aat2, \
    c2, l2, t3, at3, aat3, c3, l3, z) \
r f(t1 a1, t2 a2, t3 a3) \
{ \
    r _ret; \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    struct { \
	aat1 a1; \
	aat2 a2; \
	aat3 a3; \
    } PACKED _args = { _a1, _a2, _a3 }; \
    _assert(n < asm_tab_len); \
    _ret = do_asm_call(n, (UBYTE *)&_args, sizeof(_args), z); \
    clean_stk(sizeof(_args)); \
    return _ret; \
}

#define _THUNK4(n, r, f, t1, at1, aat1, c1, l1, t2, at2, aat2, \
    c2, l2, t3, at3, aat3, c3, l3, t4, at4, aat4, c4, l4, z) \
r f(t1 a1, t2 a2, t3 a3, t4 a4) \
{ \
    r _ret; \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    _CNV(c4, at4, l4, 4); \
    struct { \
	aat1 a1; \
	aat2 a2; \
	aat3 a3; \
	aat4 a4; \
    } PACKED _args = { _a1, _a2, _a3, _a4 }; \
    _assert(n < asm_tab_len); \
    _ret = do_asm_call(n, (UBYTE *)&_args, sizeof(_args), z); \
    clean_stk(sizeof(_args)); \
    return _ret; \
}

#define _THUNK_P_0_v(n, f, z) \
void f(void) \
{ \
    _assert(n < asm_tab_len); \
    do_asm_call(n, NULL, 0, z); \
}

#define _THUNK_P_0_vpf(n, f, z) \
__FAR(void) f(void) \
{ \
    uint32_t __ret; \
    _assert(n < asm_tab_len); \
    __ret = do_asm_call(n, NULL, 0, z); \
    return FP_FROM_D(void, __ret); \
}

#define _THUNK_P_0(n, r, f, z) \
r f(void) \
{ \
    _assert(n < asm_tab_len); \
    return do_asm_call(n, NULL, 0, z); \
}

#define _THUNK_P_1_v(n, f, t1, at1, aat1, c1, l1, z) \
void f(t1 a1) \
{ \
    _CNV(c1, at1, l1, 1); \
    struct { \
	aat1 a1; \
    } PACKED _args = { _a1 }; \
    _assert(n < asm_tab_len); \
    do_asm_call(n, (UBYTE *)&_args, sizeof(_args), z); \
}

#define _THUNK_P_1(n, r, f, t1, at1, aat1, c1, l1, z) \
r f(t1 a1) \
{ \
    _CNV(c1, at1, l1, 1); \
    struct { \
	aat1 a1; \
    } PACKED _args = { _a1 }; \
    _assert(n < asm_tab_len); \
    return do_asm_call(n, (UBYTE *)&_args, sizeof(_args), z); \
}

#define _THUNK_P_2(n, r, f, t1, at1, aat1, c1, l1, t2, at2, aat2, c2, l2, z) \
r f(t1 a1, t2 a2) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    struct { \
	aat2 a2; \
	aat1 a1; \
    } PACKED _args = { _a2, _a1 }; \
    _assert(n < asm_tab_len); \
    return do_asm_call(n, (UBYTE *)&_args, sizeof(_args), z); \
}

#define _THUNK_P_2_v(n, f, t1, at1, aat1, c1, l1, t2, at2, aat2, c2, l2, z) \
void f(t1 a1, t2 a2) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    struct { \
	aat2 a2; \
	aat1 a1; \
    } PACKED _args = { _a2, _a1 }; \
    _assert(n < asm_tab_len); \
    do_asm_call(n, (UBYTE *)&_args, sizeof(_args), z); \
}

#define _THUNK_P_3(n, r, f, t1, at1, aat1, c1, l1, t2, at2, aat2, \
    c2, l2, t3, at3, aat3, c3, l3, z) \
r f(t1 a1, t2 a2, t3 a3) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    struct { \
	aat3 a3; \
	aat2 a2; \
	aat1 a1; \
    } PACKED _args = { _a3, _a2, _a1 }; \
    _assert(n < asm_tab_len); \
    return do_asm_call(n, (UBYTE *)&_args, sizeof(_args), z); \
}

#define _THUNK_P_4_v(n, f, t1, at1, aat1, c1, l1, t2, at2, aat2, \
    c2, l2, t3, at3, aat3, c3, l3, t4, at4, aat4, c4, l4, z) \
void f(t1 a1, t2 a2, t3 a3, t4 a4) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    _CNV(c4, at4, l4, 4); \
    struct { \
	aat4 a4; \
	aat3 a3; \
	aat2 a2; \
	aat1 a1; \
    } PACKED _args = { _a4, _a3, _a2, _a1 }; \
    _assert(n < asm_tab_len); \
    do_asm_call(n, (UBYTE *)&_args, sizeof(_args), z); \
}

#define _THUNK_P_4(n, r, f, t1, at1, aat1, c1, l1, t2, at2, aat2, \
    c2, l2, t3, at3, aat3, c3, l3, t4, at4, aat4, c4, l4, z) \
r f(t1 a1, t2 a2, t3 a3, t4 a4) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    _CNV(c4, at4, l4, 4); \
    struct { \
	aat4 a4; \
	aat3 a3; \
	aat2 a2; \
	aat1 a1; \
    } PACKED _args = { _a4, _a3, _a2, _a1 }; \
    _assert(n < asm_tab_len); \
    return do_asm_call(n, (UBYTE *)&_args, sizeof(_args), z); \
}

#define _THUNK_P_5(n, r, f, t1, at1, aat1, c1, l1, t2, at2, aat2, \
    c2, l2, t3, at3, aat3, c3, l3, t4, at4, aat4, c4, l4, t5, at5, \
    aat5, c5, l5, z) \
r f(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    _CNV(c4, at4, l4, 4); \
    _CNV(c5, at5, l5, 5); \
    struct { \
	aat5 a5; \
	aat4 a4; \
	aat3 a3; \
	aat2 a2; \
	aat1 a1; \
    } PACKED _args = { _a5, _a4, _a3, _a2, _a1 }; \
    _assert(n < asm_tab_len); \
    return do_asm_call(n, (UBYTE *)&_args, sizeof(_args), z); \
}

#define _THUNK_P_6(n, r, f, t1, at1, aat1, c1, l1, t2, at2, aat2, \
    c2, l2, t3, at3, aat3, c3, l3, t4, at4, aat4, c4, l4, t5, at5, aat5, \
    c5, l5, t6, at6, aat6, c6, l6, z) \
r f(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    _CNV(c4, at4, l4, 4); \
    _CNV(c5, at5, l5, 5); \
    _CNV(c6, at6, l6, 6); \
    struct { \
	aat6 a6; \
	aat5 a5; \
	aat4 a4; \
	aat3 a3; \
	aat2 a2; \
	aat1 a1; \
    } PACKED _args = { _a6, _a5, _a4, _a3, _a2, _a1 }; \
    _assert(n < asm_tab_len); \
    return do_asm_call(n, (UBYTE *)&_args, sizeof(_args), z); \
}

#include <thunk_asms.h>


UBYTE peekb(UWORD seg, UWORD ofs)
{
    return *(UBYTE *)so2lin(seg, ofs);
}

UWORD peekw(UWORD seg, UWORD ofs)
{
    return *(UWORD *)so2lin(seg, ofs);
}

UDWORD peekl(UWORD seg, UWORD ofs)
{
    return *(UDWORD *)so2lin(seg, ofs);
}

void pokeb(UWORD seg, UWORD ofs, UBYTE b)
{
    *(UBYTE *)so2lin(seg, ofs) = b;
}

void pokew(UWORD seg, UWORD ofs, UWORD w)
{
    *(UWORD *)so2lin(seg, ofs) = w;
}

void pokel(UWORD seg, UWORD ofs, UDWORD l)
{
    *(UDWORD *)so2lin(seg, ofs) = l;
}

/* never create far pointers.
 * only look them up in symtab. */
struct far_s lookup_far_st(const void *ptr)
{
    return lookup_far(&sym_tab, ptr);
}

void thunk_call_void(struct far_s fa)
{
    asm_call(&s_regs, fa.seg, fa.off, NULL, 0);
}

void thunk_call_void_noret(struct far_s fa)
{
    asm_call_noret(&s_regs, fa.seg, fa.off, NULL, 0);
}

void int3(void)
{
    fdpp->debug("int3");
}

void fdexit(int rc)
{
    fdpp->exit(rc);
}

void panic(const BYTE * s)
{
    fdpp->panic(s);
}

void fpanic(const BYTE * s, ...)
{
    char buf[128];
    va_list l;
    va_start(l, s);
    _vsnprintf(buf, sizeof(buf), s, l);
    va_end(l);
    fdpp->panic(buf);
}

void RelocHook(UWORD old_seg, UWORD new_seg, UWORD offs, UDWORD len)
{
    unsigned i;
    int reloc = 0;
    int miss = 0;
    uint8_t *start_p = (uint8_t *)so2lin(old_seg, offs);
    uint8_t *end_p = (uint8_t *)so2lin(old_seg + (len >> 4), (len & 0xf) + offs);
    uint16_t delta = new_seg - old_seg;
    fdlogprintf("relocate %hx --> %hx, %x\n", old_seg, new_seg, len);
    do_relocs(start_p, end_p, delta);
    for (i = 0; i < _countof(asm_thunks.arr); i++) {
        uint8_t *ptr = (uint8_t *)resolve_segoff(*asm_thunks.arr[i]);
        if (ptr >= start_p && ptr <= end_p) {
            int rm;
            far_t f = lookup_far_unref(&sym_tab, ptr, &rm);
            if (!f.seg && !f.off)
                miss++;
            else
                _assert(rm);
            asm_thunks.arr[i]->seg += delta;
            store_far_replace(&sym_tab, resolve_segoff(*asm_thunks.arr[i]),
                    *asm_thunks.arr[i]);
            reloc++;
        }
    }
    fdlogprintf("processed %i relocs (%i missed)\n", reloc, miss);
}

void PurgeHook(void *ptr, UDWORD len)
{
    unsigned i;
    int reloc = 0;
    int miss = 0;
    uint8_t *start_p = (uint8_t *)ptr;
    uint8_t *end_p = start_p + len;
    fdlogprintf("purge %p %x\n", ptr, len);
    do_relocs(start_p, end_p, 0);
    for (i = 0; i < _countof(asm_thunks.arr); i++) {
        uint8_t *ptr = (uint8_t *)resolve_segoff(*asm_thunks.arr[i]);
        if (ptr >= start_p && ptr <= end_p) {
            int rm;
            far_t f = lookup_far_unref(&sym_tab, ptr, &rm);
            if (!f.seg && !f.off)
                miss++;
            else
                _assert(rm);
            asm_thunks.arr[i]->seg = asm_thunks.arr[i]->off = 0;
            reloc++;
        }
    }
    fdlogprintf("purged %i relocs (%i missed)\n", reloc, miss);
}

#define __S(x) #x
#define _S(x) __S(x)
const char *FdppDataDir(void)
{
    return _S(DATADIR);
}

const char *FdppKernelName(void)
{
    return _S(KRNL_NAME);
}

const char *FdppVersionString(void)
{
    return KERNEL_VERSION_STRING;
}
