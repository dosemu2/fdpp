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

void FdSetAsmThunks(void *tab, FdAsmCall_t call)
{
    asm_tab = (struct far_s *)tab;
    asm_call = call;
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

#define __ARG(t) t
#define __ARG_PTR(t) t *
#define __ARG_PTR_FAR(t) t FAR *

#define PACKED __attribute__((packed))

#define _THUNK0_v(n, f) \
void f(void) \
{ \
    asm_call(asm_tab[n].seg, asm_tab[n].off, NULL, 0); \
}

#define _THUNK2_v(n, f, t1, at1, t2, at2) \
void f(t1 a1, t2 a2) \
{ \
    struct { \
	at1 a1; \
	at2 a2; \
    } PACKED _args = { a1, a2 }; \
    asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_0_v(n, f) \
void f(void) \
{ \
    asm_call(asm_tab[n].seg, asm_tab[n].off, NULL, 0); \
}

#define _THUNK_P_1(n, r, f, t1, at1) \
r f(t1 a1) \
{ \
    struct { \
	at1 a1; \
    } PACKED _args = { a1 }; \
    return asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_2(n, r, f, t1, at1, t2, at2) \
r f(t1 a1, t2 a2) \
{ \
    struct { \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { a2, a1 }; \
    return asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#define _THUNK_P_3(n, r, f, t1, at1, t2, at2, t3, at3) \
r f(t1 a1, t2 a2, t3 a3) \
{ \
    struct { \
	at3 a3; \
	at2 a2; \
	at1 a1; \
    } PACKED _args = { a3, a2, a1 }; \
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
    } PACKED _args = { a6, a5, a4, a3, a2, a1 }; \
    return asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#include "thunk_asms.h"
