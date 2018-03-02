#include <stdlib.h>
#include "../hdr/portab.h"
#include "globals.h"
#include "proto.h"
#include "thunks_priv.h"
#include "thunks.h"

static struct fdpp_api *fdpp;
static struct fdthunk_api *api_calls;

struct asm_dsc_s {
    UWORD num;
    UWORD off;
    UWORD seg;
};
static struct asm_dsc_s *asm_tab;
static int asm_tab_len;
static struct far_s *sym_tab;
static int sym_tab_len;
static struct far_s *near_wrp;
static int num_wrps;

static void FdppSetAsmCalls(struct asm_dsc_s *tab, int size)
{
    asm_tab = tab;
    asm_tab_len = size / sizeof(struct asm_dsc_s);
}

#define SEMIC ;
#define __ASM(t, v) __ASMSYM(t) __##v
#define __ASM_FAR(t, v) __ASMFAR(t) __##v
#define __ASM_ARR(t, v, l) __ASMARSYM(t, __##v, l)
#define __ASM_ARRI(t, v) __ASMARISYM(t, __##v)
#define __ASM_ARRI_F(t, v) __ASMARIFSYM(t, __##v)
#define __ASM_FUNC(v) __ASMFSYM(void) __##v
#include "glob_asm.h"
#undef __ASM
#undef __ASM_FAR
#undef __ASM_ARR
#undef __ASM_ARRI
#undef __ASM_ARRI_F
#undef __ASM_FUNC

static union asm_thunks_u {
  struct _thunks {
#define __ASM(t, v) struct far_s * __##v
#define __ASM_FAR(t, v) struct far_s * __##v
#define __ASM_ARR(t, v, l) struct far_s * __##v
#define __ASM_ARRI(t, v) struct far_s * __##v
#define __ASM_ARRI_F(t, v) struct far_s * __##v
#define __ASM_FUNC(v) struct far_s * __##v
#include "glob_asm.h"
#undef __ASM
#undef __ASM_FAR
#undef __ASM_ARR
#undef __ASM_ARRI
#undef __ASM_ARRI_F
#undef __ASM_FUNC
  } thunks;
  struct far_s * arr[sizeof(struct _thunks) / sizeof(struct far_s *)];
} asm_thunks = {{
#undef SEMIC
#define SEMIC ,
#define __ASM(t, v) __ASMREF(__##v)
#define __ASM_FAR(t, v) __ASMREF(__##v)
#define __ASM_ARR(t, v, l) __ASMREF(__##v)
#define __ASM_ARRI(t, v) __ASMREF(__##v)
#define __ASM_ARRI_F(t, v) __ASMREF(__##v)
#define __ASM_FUNC(v) __ASMREF(__##v)
#include "glob_asm.h"
#undef __ASM
#undef __ASM_FAR
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

static void *so2lin(uint16_t seg, uint16_t off)
{
    return fdpp->mem_base() + (seg << 4) + off;
}

void *resolve_segoff(struct far_s fa)
{
    return so2lin(fa.seg, fa.off);
}

static int FdppSetAsmThunks(struct far_s *ptrs, int size)
{
#define _countof(a) (sizeof(a)/sizeof(*(a)))
    int i;
    int len = size / (sizeof(struct far_s));
    int exp = _countof(asm_thunks.arr);

    if (len != exp) {
        fdprintf("len=%i expected %i\n", len, exp);
        return -1;
    }
    for (i = 0; i < len; i++)
        *asm_thunks.arr[i] = ptrs[i];

    sym_tab = (struct far_s *)malloc(size);
    memcpy(sym_tab, ptrs, size);
    sym_tab_len = len;
    return 0;
}

struct fdpp_symtab {
    uint16_t symtab;
    uint16_t symtab_len;
    uint16_t calltab;
    uint16_t calltab_len;
    uint16_t num_wrps;
    struct far_s near_wrp[0];
};

static void FdppSetSymTab(struct vm86_regs *regs, struct fdpp_symtab *symtab)
{
    int err;
    struct asm_dsc_s *asmtab =
            (struct asm_dsc_s *)so2lin(regs->ds, symtab->calltab);
    struct far_s *thtab = (struct far_s *)so2lin(regs->ds, symtab->symtab);

    FdppSetAsmCalls(asmtab, symtab->calltab_len);
    err = FdppSetAsmThunks(thtab, symtab->symtab_len);
    _assert(!err);
    num_wrps = symtab->num_wrps;
    near_wrp = (struct far_s *)malloc(sizeof(struct far_s) * num_wrps);
    memcpy(near_wrp, symtab->near_wrp, sizeof(struct far_s) * num_wrps);
}

#define _ARG(n, t, ap) (*(t *)(ap + n))
#define _ARG_PTR(n, t, ap) // unimplemented, will create syntax error
#define _ARG_PTR_FAR(n, t, ap)  ({ \
    UDWORD __d = *(UDWORD *)(ap + n); \
    FP_FROM_D(t, __d); \
})

static UDWORD FdppThunkCall(int fn, UBYTE *sp, UBYTE *r_len)
{
    UDWORD ret = 0;
    UBYTE rsz = 0;
#define _RET ret
#define _RSZ rsz
#define _SP sp

    switch (fn) {
        #include "thunk_calls.h"
    }

    if (r_len)
        *r_len = rsz;
    return ret;
}

void FdppCall(struct vm86_regs *regs)
{
    switch (regs->ebx & 0xff) {
    case 0:
        FdppSetSymTab(regs,
                (struct fdpp_symtab *)so2lin(regs->ss, regs->esp + 6));
        break;
    case 1:
        regs->eax = FdppThunkCall(regs->ecx,
                (UBYTE *)so2lin(regs->ss, regs->esp),
                NULL);
        break;
    }

    s_regs = *regs;
}

void do_abort(const char *file, int line)
{
    fdpp->abort_handler(file, line);
}

void FdppInit(struct fdpp_api *api)
{
    fdpp = api;
    api_calls = &api->thunks;
}

void fdprintf(const char *format, ...)
{
    va_list vl;

    va_start(vl, format);
    fdpp->print_handler(format, vl);
    va_end(vl);
}

void cpu_relax(void)
{
    fdpp->cpu_relax();
}


// TODO: align args by 2 bytes
static uint32_t do_asm_call_far(int num, uint8_t *sp, uint8_t len)
{
    int i;

    for (i = 0; i < asm_tab_len; i++) {
        if (asm_tab[i].num == num) {
            fdpp->asm_call(&s_regs, asm_tab[i].seg, asm_tab[i].off, sp, len);
            return (s_regs.edx << 16) | (s_regs.eax & 0xffff);
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

static uint32_t do_asm_call(int num, uint8_t *sp, uint8_t len)
{
    int i;

    for (i = 0; i < asm_tab_len; i++) {
        if (asm_tab[i].num == num) {
            uint16_t wrp = find_wrp(asm_tab[i].seg);
            s_regs.eax = asm_tab[i].off;
            s_regs.ecx = len >> 1;
            fdpp->asm_call(&s_regs, asm_tab[i].seg, wrp, sp, len);
            return (s_regs.edx << 16) | (s_regs.eax & 0xffff);
        }
    }
    _assert(0);
    return -1;
}

static uint8_t *clean_stk(size_t len)
{
    uint8_t *ret = (uint8_t *)so2lin(s_regs.ss, s_regs.esp);
    s_regs.esp += len;
    return ret;
}

uint16_t getCS(void)
{
    return s_regs.cs;
}

void setDS(uint16_t seg)
{
    s_regs.ds = seg;
}

void setES(uint16_t seg)
{
    s_regs.es = seg;
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
#define __ARG_PTR_FAR(t) t FAR *
#define __ARG_A(t) t
#define __ARG_PTR_A(t) PTR_MEMB(t)
#define __ARG_PTR_FAR_A(t) __DOSFAR(t)

#define __CNV_PTR_FAR(t, d, f, l) t d = (f)
#define __CNV_PTR(t, d, f, l) \
    _MK_FAR_SZ(__##d, f, sizeof(*f)); \
    t d = __MK_NEAR(__##d)
#define __CNV_PTR_CHAR(t, d, f, l) \
    _MK_FAR_STR(__##d, f); \
    t d = __MK_NEAR(__##d)
#define __CNV_PTR_VOID(t, d, f, l) \
    _MK_FAR_SZ(__##d, f, l); \
    t d = __MK_NEAR(__##d)
#define __CNV_SIMPLE(t, d, f, l) t d = (f)

#define _CNV(c, at, nl, n) c(at, _a##n, a##n, a##nl)

#define _THUNK_F_0_v(n, f) \
void f(void) \
{ \
    _assert(n < asm_tab_len); \
    do_asm_call_far(n, NULL, 0); \
}

#define _THUNK0_v(n, f) \
void f(void) \
{ \
    _assert(n < asm_tab_len); \
    do_asm_call(n, NULL, 0); \
}

#define _THUNK_F_1_v(n, f, t1, at1, c1, l1) \
void f(t1 a1) \
{ \
    _CNV(c1, at1, l1, 1); \
    struct { \
	at1 a1; \
    } PACKED _args = { _a1 }; \
    _assert(n < asm_tab_len); \
    do_asm_call_far(n, (UBYTE *)&_args, sizeof(_args)); \
    clean_stk(sizeof(_args)); \
}

#define _THUNK2_v(n, f, t1, at1, c1, l1, t2, at2, c2, l2) \
void f(t1 a1, t2 a2) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    struct { \
	at1 a1; \
	at2 a2; \
    } PACKED _args = { _a1, _a2 }; \
    _assert(n < asm_tab_len); \
    do_asm_call(n, (UBYTE *)&_args, sizeof(_args)); \
    clean_stk(sizeof(_args)); \
}

#define _THUNK3_v(n, f, t1, at1, c1, l1, t2, at2, c2, l2, t3, at3, c3, l3) \
void f(t1 a1, t2 a2, t3 a3) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    struct { \
	at1 a1; \
	at2 a2; \
	at2 a3; \
    } PACKED _args = { _a1, _a2, _a3 }; \
    _assert(n < asm_tab_len); \
    do_asm_call(n, (UBYTE *)&_args, sizeof(_args)); \
    clean_stk(sizeof(_args)); \
}

#define _THUNK4(n, r, f, t1, at1, c1, l1, t2, at2, c2, l2, t3, at3, c3, l3, t4, at4, c4, l4) \
r f(t1 a1, t2 a2, t3 a3, t4 a4) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    _CNV(c4, at4, l4, 4); \
    struct { \
	at1 a1; \
	at2 a2; \
	at3 a3; \
	at4 a4; \
    } PACKED _args = { _a1, _a2, _a3, _a4 }; \
    _assert(n < asm_tab_len); \
    return do_asm_call(n, (UBYTE *)&_args, sizeof(_args)); \
    clean_stk(sizeof(_args)); \
}

#define _THUNK_F_P_0_v(n, f) \
void f(void) \
{ \
    _assert(n < asm_tab_len); \
    do_asm_call_far(n, NULL, 0); \
}

#define _THUNK_P_0_v(n, f) \
void f(void) \
{ \
    _assert(n < asm_tab_len); \
    do_asm_call(n, NULL, 0); \
}

#define _THUNK_F_P_0_vp(n, f) \
void FAR *f(void) \
{ \
    uint32_t __ret; \
    _assert(n < asm_tab_len); \
    __ret = do_asm_call_far(n, NULL, 0); \
    return FP_FROM_D(void, __ret); \
}

#define _THUNK_P_0(n, r, f) \
r f(void) \
{ \
    _assert(n < asm_tab_len); \
    return do_asm_call(n, NULL, 0); \
}

#define _THUNK_F_P_1_v(n, f, t1, at1, c1, l1) \
void f(t1 a1) \
{ \
    _CNV(c1, at1, l1, 1); \
    struct { \
	at1 a1; \
    } PACKED _args = { _a1 }; \
    _assert(n < asm_tab_len); \
    do_asm_call_far(n, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_1_v(n, f, t1, at1, c1, l1) \
void f(t1 a1) \
{ \
    _CNV(c1, at1, l1, 1); \
    struct { \
	at1 a1; \
    } PACKED _args = { _a1 }; \
    _assert(n < asm_tab_len); \
    do_asm_call(n, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_1(n, r, f, t1, at1, c1, l1) \
r f(t1 a1) \
{ \
    _CNV(c1, at1, l1, 1); \
    struct { \
	at1 a1; \
    } PACKED _args = { _a1 }; \
    _assert(n < asm_tab_len); \
    return do_asm_call(n, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_F_P_2(n, r, f, t1, at1, c1, l1, t2, at2, c2, l2) \
r f(t1 a1, t2 a2) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    struct { \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { _a2, _a1 }; \
    _assert(n < asm_tab_len); \
    return do_asm_call_far(n, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_2(n, r, f, t1, at1, c1, l1, t2, at2, c2, l2) \
r f(t1 a1, t2 a2) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    struct { \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { _a2, _a1 }; \
    _assert(n < asm_tab_len); \
    return do_asm_call(n, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_F_P_3(n, r, f, t1, at1, c1, l1, t2, at2, c2, l2, t3, at3, c3, l3) \
r f(t1 a1, t2 a2, t3 a3) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    struct { \
	at3 a3; \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { _a3, _a2, _a1 }; \
    _assert(n < asm_tab_len); \
    return do_asm_call_far(n, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_3(n, r, f, t1, at1, c1, l1, t2, at2, c2, l2, t3, at3, c3, l3) \
r f(t1 a1, t2 a2, t3 a3) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    struct { \
	at3 a3; \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { _a3, _a2, _a1 }; \
    _assert(n < asm_tab_len); \
    return do_asm_call(n, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_4_v(n, f, t1, at1, c1, l1, t2, at2, c2, l2, t3, at3, c3, l3, t4, at4, c4, l4) \
void f(t1 a1, t2 a2, t3 a3, t4 a4) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    _CNV(c4, at4, l4, 4); \
    struct { \
	at4 a4; \
	at3 a3; \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { _a4, _a3, _a2, _a1 }; \
    _assert(n < asm_tab_len); \
    do_asm_call(n, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_4(n, r, f, t1, at1, c1, l1, t2, at2, c2, l2, t3, at3, c3, l3, t4, at4, c4, l4) \
r f(t1 a1, t2 a2, t3 a3, t4 a4) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    _CNV(c4, at4, l4, 4); \
    struct { \
	at4 a4; \
	at3 a3; \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { _a4, _a3, _a2, _a1 }; \
    _assert(n < asm_tab_len); \
    return do_asm_call(n, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_5(n, r, f, t1, at1, c1, l1, t2, at2, c2, l2, t3, at3, c3, l3, t4, at4, c4, l4, t5, at5, c5, l5) \
r f(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    _CNV(c4, at4, l4, 4); \
    _CNV(c5, at5, l5, 5); \
    struct { \
	at5 a5; \
	at4 a4; \
	at3 a3; \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { _a5, _a4, _a3, _a2, _a1 }; \
    _assert(n < asm_tab_len); \
    return do_asm_call(n, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_F_P_6(n, r, f, t1, at1, c1, l1, t2, at2, c2, l2, t3, at3, c3, l3, t4, at4, c4, l4, t5, at5, c5, l5, t6, at6, c6, l6) \
r f(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    _CNV(c4, at4, l4, 4); \
    _CNV(c5, at5, l5, 5); \
    _CNV(c6, at6, l6, 6); \
    struct { \
	at6 a6; \
	at5 a5; \
	at4 a4; \
	at3 a3; \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { _a6, _a5, _a4, _a3, _a2, _a1 }; \
    _assert(n < asm_tab_len); \
    return do_asm_call_far(n, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_6(n, r, f, t1, at1, c1, l1, t2, at2, c2, l2, t3, at3, c3, l3, t4, at4, c4, l4, t5, at5, c5, l5, t6, at6, c6, l6) \
r f(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6) \
{ \
    _CNV(c1, at1, l1, 1); \
    _CNV(c2, at2, l2, 2); \
    _CNV(c3, at3, l3, 3); \
    _CNV(c4, at4, l4, 4); \
    _CNV(c5, at5, l5, 5); \
    _CNV(c6, at6, l6, 6); \
    struct { \
	at6 a6; \
	at5 a5; \
	at4 a4; \
	at3 a3; \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { _a6, _a5, _a4, _a3, _a2, _a1 }; \
    _assert(n < asm_tab_len); \
    return do_asm_call(n, (UBYTE *)&_args, sizeof(_args)); \
}

#include "thunk_asms.h"


#define _THUNK_API_v(n) \
void n(void) \
{ \
    api_calls->n(); \
}

#define _THUNK_API_0(r, n) \
r n(void) \
{ \
    return api_calls->n(); \
}

#define _THUNK_API_1v(n, t1, a1) \
void n(t1 a1) \
{ \
    api_calls->n(a1); \
}

#include "thunkapi_tmpl.h"


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
    int i;
    for (i = 0; i < sym_tab_len; i++) {
        if (resolve_segoff(sym_tab[i]) == ptr)
            return sym_tab[i];
    }
    _assert(0);
    return (struct far_s){0, 0};
}

uint32_t thunk_call_void(struct far_s fa)
{
    fdpp->asm_call(&s_regs, fa.seg, fa.off, NULL, 0);
    return (s_regs.edx << 16) | (s_regs.eax & 0xffff);
}
