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

#undef _ARG
#undef _ARG_PTR
#undef _ARG_PTR_FAR

#define _ARG(n, t, ap) t
#define _ARG_PTR(n, t, ap) t *
#define _ARG_PTR_FAR(n, t, ap) t FAR *

#define _THUNK0_v(n, f) \
void f(void) \
{ \
    asm_call(asm_tab[n].seg, asm_tab[n].off, NULL, 0); \
}

#define _THUNK2_v(n, f, t1, t2) \
void f(t1 a1, t2 a2) \
{ \
    struct { \
	t1 a1; \
	t2 a2; \
    } _args = { a1, a2 }; \
    asm_call(asm_tab[n].seg, asm_tab[n].off, (UBYTE *)&_args, sizeof(_args)); \
}

#include "thunk_asms.h"

#undef _ARG
#undef _ARG_PTR
#undef _ARG_PTR_FAR
