#ifndef THUNKS_H
#define THUNKS_H

#ifdef __cplusplus
extern "C" {
#endif

UDWORD FdThunkCall(int fn, UBYTE *sp, UBYTE *r_len);

typedef UDWORD (*FdAsmCall_t)(UWORD seg, UWORD off, UBYTE *sp, UBYTE len);
void FdSetAsmCalls(FdAsmCall_t call, void *tab, int len);
int FdSetAsmThunks(void **ptrs, int len);
void FdSetAbortHandler(void (*handler)(const char *, int));

#include "thunkapi.h"

void FdSetApiCalls(struct fdthunk_api *calls);

#ifdef __cplusplus
}
#endif

#endif
