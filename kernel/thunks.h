#ifndef THUNKS_H
#define THUNKS_H

UDWORD FdThunkCall(int fn, UBYTE *sp, UBYTE *r_len);

typedef UDWORD (*FdAsmCall_t)(UWORD seg, UWORD off, UBYTE *sp, UBYTE len);
void FdSetAsmThunks(void *tab, FdAsmCall_t call);

#endif
