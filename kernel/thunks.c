#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "../hdr/portab.h"
#include "globals.h"
#include "proto.h"
#include "thunks_priv.h"
#include "thunks.h"

static struct fdpp_api *fdpp;

struct asm_dsc_s {
    UWORD num;
    UWORD off;
    UWORD seg;
};
#define asm_tab ((struct asm_dsc_s *)resolve_segoff(asm_tab_fp))
static int asm_tab_len;
static struct far_s asm_tab_fp;
static struct farhlp sym_tab;
static struct far_s *near_wrp;
static int num_wrps;
static jmp_buf *noret_jmp;

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
    return fdpp->mem_base() + (seg << 4) + off;
}

void *resolve_segoff(struct far_s fa)
{
    return so2lin(fa.seg, fa.off);
}

static int FdppSetAsmThunks(struct far_s *ptrs, int len)
{
#define _countof(a) (sizeof(a)/sizeof(*(a)))
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
    ptr = (uint8_t *)resolve_segoff(asm_tab_fp);
    if (ptr >= start_p && ptr < end_p) {
        asm_tab_fp.seg += delta;
        reloc++;
    }
    for (i = 0; i < num_wrps; i++) {
        ptr = (uint8_t *)resolve_segoff(near_wrp[i]);
        if (ptr >= start_p && ptr < end_p) {
            near_wrp[i].seg += delta;
            reloc++;
        }
    }
    fdprintf("processed %i relocs\n", reloc);
    t = asm_tab;
    reloc = 0;
    for (i = 0; i < asm_tab_len; i++) {
        ptr = (uint8_t *)so2lin(t[i].seg, t[i].off);
        if (ptr >= start_p && ptr < end_p) {
            t[i].seg += delta;
            reloc++;
        }
    }
    fdprintf("processed %i relocs\n", reloc);
}

static void FdppSetSymTab(struct vm86_regs *regs, struct fdpp_symtab *symtab)
{
    int err;
    struct far_s *thtab = (struct far_s *)resolve_segoff(symtab->symtab);
    int stab_len = symtab->symtab_len / sizeof(struct far_s);

    num_wrps = symtab->num_wrps;
    near_wrp = (struct far_s *)malloc(sizeof(struct far_s) * num_wrps);
    memcpy(near_wrp, symtab->near_wrp, sizeof(struct far_s) * num_wrps);
    asm_tab_fp = symtab->calltab;
    asm_tab_len = symtab->calltab_len / sizeof(struct asm_dsc_s);
    /* now relocate */
    if (symtab->cur_cs > symtab->orig_cs) {
        int i;
        int reloc;
        uint8_t *start_p = (uint8_t *)resolve_segoff(symtab->text_start);
        uint8_t *end_p = (uint8_t *)resolve_segoff(symtab->text_end);
        uint16_t delta = symtab->cur_cs - symtab->orig_cs;
        do_relocs(start_p, end_p, delta);
        /* sym_tab table is patched in non-relocated code, never used later */
        reloc = 0;
        for (i = 0; i < stab_len; i++) {
            uint8_t *ptr = (uint8_t *)resolve_segoff(thtab[i]);
            if (ptr >= start_p && ptr < end_p) {
                thtab[i].seg += delta;
                reloc++;
            }
        }
        fdprintf("processed %i relocs\n", reloc);
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
    jmp_buf jmp, *prev_jmp = noret_jmp;
    s_regs = *regs;
    UBYTE len;
    UDWORD res;

    switch (regs->ebx & 0xff) {
    case 0:
        FdppSetSymTab(regs,
                (struct fdpp_symtab *)so2lin(regs->ss, regs->esp + 6));
        break;
    case 1:
        noret_jmp = &jmp;
        if (setjmp(jmp))
            break;
        res = FdppThunkCall(regs->ecx,
                (UBYTE *)so2lin(regs->ss, regs->edx), &len);
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

    noret_jmp = prev_jmp;
}

void do_abort(const char *file, int line)
{
    fdpp->abort(file, line);
}

void FdppInit(struct fdpp_api *api)
{
    fdpp = api;
}

void fdvprintf(const char *format, va_list vl)
{
    fdpp->print(format, vl);
}

void fdprintf(const char *format, ...)
{
    va_list vl;

    va_start(vl, format);
    fdvprintf(format, vl);
    va_end(vl);
}

void cpu_relax(void)
{
    fdpp->cpu_relax();
}

#define FLG_FAR 1
#define FLG_NORET 2

static uint32_t _do_asm_call_far(int num, uint8_t *sp, uint8_t len,
        FdppAsmCall_t call)
{
    int i;

    for (i = 0; i < asm_tab_len; i++) {
        if (asm_tab[i].num == num) {
            call(&s_regs, asm_tab[i].seg, asm_tab[i].off, sp, len);
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

static uint32_t _do_asm_call(int num, uint8_t *sp, uint8_t len,
        FdppAsmCall_t call)
{
    int i;

    for (i = 0; i < asm_tab_len; i++) {
        if (asm_tab[i].num == num) {
            uint16_t wrp = find_wrp(asm_tab[i].seg);
            s_regs.eax = asm_tab[i].off;
            /* argpack should be aligned */
            _assert(!(len & 1));
            s_regs.ecx = len >> 1;
            call(&s_regs, asm_tab[i].seg, wrp, sp, len);
            return (s_regs.edx << 16) | (s_regs.eax & 0xffff);
        }
    }
    _assert(0);
    return -1;
}

static void asm_call_noret(struct vm86_regs *regs, uint16_t seg,
        uint16_t off, uint8_t *sp, uint8_t len)
{
    fdpp->asm_call_noret(regs, seg, off, sp, len);
    longjmp(*noret_jmp, 1);
}

static uint32_t do_asm_call(int num, uint8_t *sp, uint8_t len, int flags)
{
    uint32_t ret;
    FdppAsmCall_t call = ((flags & FLG_NORET) ?
            asm_call_noret : fdpp->asm_call);
    if (flags & FLG_FAR)
        ret = _do_asm_call_far(num, sp, len, call);
    else
        ret = _do_asm_call(num, sp, len, call);
    return ret;
}

static uint8_t *clean_stk(size_t len)
{
    uint8_t *ret = (uint8_t *)so2lin(s_regs.ss, s_regs.esp);
    s_regs.esp += len;
    return ret;
}

uint16_t getCS(void)
{
    /* we pass CS in SI */
    return s_regs.esi;
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
    _MK_FAR_SZ(__##d, f, sizeof(t)); \
    t d = __MK_NEAR2(__##d, t)
#define __CNV_PTR_CHAR(t, d, f, l) \
    _MK_FAR_STR(__##d, f); \
    t d = __MK_NEAR(__##d)
#define __CNV_PTR_VOID(t, d, f, l) \
    _MK_FAR_SZ(__##d, f, l); \
    t d = __MK_NEAR(__##d)
#define __CNV_SIMPLE(t, d, f, l) t d = (f)

#define _CNV(c, at, nl, n) c(at, _a##n, a##n, a##nl)

#define _THUNK0_v(n, f, z) \
void f(void) \
{ \
    _assert(n < asm_tab_len); \
    do_asm_call(n, NULL, 0, z); \
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

#define _THUNK4(n, r, f, t1, at1, aat1, c1, l1, t2, at2, aat2, \
    c2, l2, t3, at3, aat3, c3, l3, t4, at4, aat4, c4, l4, z) \
r f(t1 a1, t2 a2, t3 a3, t4 a4) \
{ \
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
    return do_asm_call(n, (UBYTE *)&_args, sizeof(_args), z); \
    clean_stk(sizeof(_args)); \
}

#define _THUNK_P_0_v(n, f, z) \
void f(void) \
{ \
    _assert(n < asm_tab_len); \
    do_asm_call(n, NULL, 0, z); \
}

#define _THUNK_P_0_vp(n, f, z) \
void FAR *f(void) \
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

#include "thunk_asms.h"


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

uint32_t thunk_call_void(struct far_s fa)
{
    fdpp->asm_call(&s_regs, fa.seg, fa.off, NULL, 0);
    return (s_regs.edx << 16) | (s_regs.eax & 0xffff);
}

void int3(void)
{
    fdpp->debug("int3");
}

void RelocHook(UWORD old_seg, UWORD new_seg, UDWORD len)
{
    int i;
    int reloc = 0;
    int miss = 0;
    uint8_t *start_p = (uint8_t *)so2lin(old_seg, 0);
    uint8_t *end_p = (uint8_t *)so2lin(old_seg + (len >> 4), len & 0xf);
    uint16_t delta = new_seg - old_seg;
    do_relocs(start_p, end_p, delta);
    for (i = 0; i < _countof(asm_thunks.arr); i++) {
        uint8_t *ptr = (uint8_t *)resolve_segoff(*asm_thunks.arr[i]);
        if (ptr >= start_p && ptr < end_p) {
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
    fdprintf("processed %i relocs (%i missed)\n", reloc, miss);
}
