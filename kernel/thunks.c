#include "../hdr/portab.h"
#include "globals.h"
#include "proto.h"
#include "thunks.h"

static FdAsmCall_t asm_call;
struct far_s {
    UWORD off;
    UWORD seg;
};
static struct far_s *asm_tab;
static int asm_tab_len;

void FdSetAsmCalls(FdAsmCall_t call, void *tab, int len)
{
    asm_call = call;
    asm_tab = (struct far_s *)tab;
    asm_tab_len = len;
}

#define SEMIC ;
#define __ASM(t, v) t * __##v
#define __ASM_ARR(t, v, l) t (* __##v)[l]
#define __ASM_FUNC(v) void (* v)(void)
#include "glob_asm.h"
#undef __ASM
#undef __ASM_ARR
#undef __ASM_FUNC

static union asm_thunks_u {
  struct {
#define __ASM(t, v) t ** __##v
#define __ASM_ARR(t, v, l) t (** __##v)[l]
#define __ASM_FUNC(v) void (** v)(void)
#include "glob_asm.h"
#undef __ASM
#undef __ASM_ARR
#undef __ASM_FUNC
  } thunks;
  void ** arr[0];
} asm_thunks = {{
#undef SEMIC
#define SEMIC ,
#define __ASM(t, v) &__##v
#define __ASM_ARR(t, v, l) &__##v
#define __ASM_FUNC(v) &v
#include "glob_asm.h"
#undef __ASM
#undef __ASM_ARR
#undef __ASM_FUNC
#undef SEMIC
}};

int FdSetAsmThunks(void **ptrs, int len)
{
#define _countof(a) (sizeof(a)/sizeof(*(a)))
    int i;

    if (len != _countof(asm_thunks.arr))
        return -1;
    for (i = 0; i < len; i++)
        *asm_thunks.arr[i] = ptrs[i];
    return 0;
}

#define _ARG(n, t, ap) (*(t *)(ap + n))
#define _ARG_PTR(n, t, ap) // unimplemented, will create syntax error
#define _ARG_PTR_FAR(n, t, ap)  ((t FAR *)(uintptr_t)*(UDWORD *)(ap + n))

UDWORD FdThunkCall(int fn, UBYTE *sp, UBYTE *r_len)
{
    UDWORD ret = 0;
#define _RET ret
#define _RSZ (*r_len)
#define _SP sp

    switch (fn) {
        #include "thunk_calls.h"
    }

    return ret;
}

#define _ASSERT(c) if (!(c)) abort_handler(__FILE__, __LINE__)
static void abort_handler_dummy(const char *file, int line)
{
}
static void (*abort_handler)(const char *file, int line) = abort_handler_dummy;

void FdSetAbortHandler(void (*handler)(const char *, int))
{
    abort_handler = handler;
}

#define __ARG(t) t
#define __ARG_PTR(t) t *
#define __ARG_PTR_FAR(t) t FAR *

#define PACKED __attribute__((packed))

#define _THUNK0_v(n, f) \
void f(void) \
{ \
    _ASSERT(n < asm_tab_len); \
    asm_call(asm_tab[n].seg, asm_tab[n].off, NULL, 0); \
}

#define _THUNK1_v(n, f, t1, at1) \
void f(t1 a1) \
{ \
    struct { \
	at1 a1; \
    } PACKED _args = { (at1)a1 }; \
    _ASSERT(n < asm_tab_len); \
    asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK2_v(n, f, t1, at1, t2, at2) \
void f(t1 a1, t2 a2) \
{ \
    struct { \
	at1 a1; \
	at2 a2; \
    } PACKED _args = { (at1)a1, (at2)a2 }; \
    _ASSERT(n < asm_tab_len); \
    asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK3_v(n, f, t1, at1, t2, at2, t3, at3) \
void f(t1 a1, t2 a2, t3 a3) \
{ \
    struct { \
	at1 a1; \
	at2 a2; \
	at2 a3; \
    } PACKED _args = { (at1)a1, (at2)a2, (at3)a3 }; \
    _ASSERT(n < asm_tab_len); \
    asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK4(n, r, f, t1, at1, t2, at2, t3, at3, t4, at4) \
r f(t1 a1, t2 a2, t3 a3, t4 a4) \
{ \
    struct { \
	at1 a1; \
	at2 a2; \
	at3 a3; \
	at4 a4; \
    } PACKED _args = { (at1)a1, (at2)a2, (at3)a3, (at4)a4 }; \
    _ASSERT(n < asm_tab_len); \
    return asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_0_v(n, f) \
void f(void) \
{ \
    _ASSERT(n < asm_tab_len); \
    asm_call(asm_tab[n].seg, asm_tab[n].off, NULL, 0); \
}

#define _THUNK_P_0_vp(n, f) \
void FAR *f(void) \
{ \
    _ASSERT(n < asm_tab_len); \
    return (void FAR *)asm_call(asm_tab[n].seg, asm_tab[n].off, NULL, 0); \
}

#define _THUNK_P_0(n, r, f) \
r f(void) \
{ \
    _ASSERT(n < asm_tab_len); \
    return asm_call(asm_tab[n].seg, asm_tab[n].off, NULL, 0); \
}

#define _THUNK_P_1_v(n, f, t1, at1) \
void f(t1 a1) \
{ \
    struct { \
	at1 a1; \
    } PACKED _args = { (at1)a1 }; \
    _ASSERT(n < asm_tab_len); \
    asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_1(n, r, f, t1, at1) \
r f(t1 a1) \
{ \
    struct { \
	at1 a1; \
    } PACKED _args = { (at1)a1 }; \
    _ASSERT(n < asm_tab_len); \
    return asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_2(n, r, f, t1, at1, t2, at2) \
r f(t1 a1, t2 a2) \
{ \
    struct { \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { (at2)a2, (at1)a1 }; \
    _ASSERT(n < asm_tab_len); \
    return asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_3(n, r, f, t1, at1, t2, at2, t3, at3) \
r f(t1 a1, t2 a2, t3 a3) \
{ \
    struct { \
	at3 a3; \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { (at3)a3, (at2)a2, (at1)a1 }; \
    _ASSERT(n < asm_tab_len); \
    return asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_4_v(n, f, t1, at1, t2, at2, t3, at3, t4, at4) \
void f(t1 a1, t2 a2, t3 a3, t4 a4) \
{ \
    struct { \
	at4 a4; \
	at3 a3; \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { (at4)a4, (at3)a3, (at2)a2, (at1)a1 }; \
    _ASSERT(n < asm_tab_len); \
    asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_4(n, r, f, t1, at1, t2, at2, t3, at3, t4, at4) \
r f(t1 a1, t2 a2, t3 a3, t4 a4) \
{ \
    struct { \
	at4 a4; \
	at3 a3; \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { (at4)a4, (at3)a3, (at2)a2, (at1)a1 }; \
    _ASSERT(n < asm_tab_len); \
    return asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_5(n, r, f, t1, at1, t2, at2, t3, at3, t4, at4, t5, at5) \
r f(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5) \
{ \
    struct { \
	at5 a5; \
	at4 a4; \
	at3 a3; \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { (at5)a5, (at4)a4, (at3)a3, (at2)a2, (at1)a1 }; \
    _ASSERT(n < asm_tab_len); \
    return asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_6(n, r, f, t1, at1, t2, at2, t3, at3, t4, at4, t5, at5, t6, at6) \
r f(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6) \
{ \
    struct { \
	at6 a6; \
	at5 a5; \
	at4 a4; \
	at3 a3; \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { (at6)a6, (at5)a5, (at4)a4, (at3)a3, (at2)a2, (at1)a1 }; \
    _ASSERT(n < asm_tab_len); \
    return asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#include "thunk_asms.h"


static struct fdthunk_api *api_calls;

void FdSetApiCalls(struct fdthunk_api *calls)
{
    api_calls = calls;
}

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

#define _THUNK_API_1(r, n, t1, a1) \
r n(t1 a1) \
{ \
    return api_calls->n(a1); \
}

#define _THUNK_API_2(r, n, t1, a1, t2, a2) \
r n(t1 a1, t2 a2) \
{ \
    return api_calls->n(a1, a2); \
}

#define _THUNK_API_3v(n, t1, a1, t2, a2, t3, a3) \
void n(t1 a1, t2 a2, t3 a3) \
{ \
    api_calls->n(a1, a2, a3); \
}

#include "thunkapi.h"
