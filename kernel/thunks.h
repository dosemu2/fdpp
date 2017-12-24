#ifndef THUNKS_H
#define THUNKS_H

UDWORD FdThunkCall(int fn, UBYTE *sp, UBYTE *r_len);

typedef UDWORD (*FdAsmCall_t)(UWORD seg, UWORD off, UBYTE *sp, UBYTE len);
void FdSetAsmCalls(FdAsmCall_t call, void *tab, int len);
int FdSetAsmThunks(void **ptrs, int len);

struct thunk_api {
#define _THUNK_API_v(n) void (* n)(void)
#define _THUNK_API_2(r, n, t1, a1, t2, a2) r (* n)(t1 a1, t2 a2)
#define _THUNK_API_3v(n, t1, a1, t2, a2, t3, a3) void (* n)(t1 a1, t2 a2, t3 a3)
#define SEMIC ;
#include "thunkapi.h"
#undef SEMIC
#undef _THUNK_API_v
#undef _THUNK_API_2
#undef _THUNK_API_3v
};

void FdSetApiCalls(struct thunk_api *calls);

#endif
