#ifndef THUNKS_H
#define THUNKS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t FdThunkCall(int fn, uint8_t *sp, uint8_t *r_len);

typedef uintptr_t (*FdAsmCall_t)(uint16_t seg, uint16_t off, uint8_t *sp, uint8_t len);
void FdSetAsmCalls(FdAsmCall_t call, void *tab, int len);
int FdSetAsmThunks(void **ptrs, int len);
void FdSetAbortHandler(void (*handler)(const char *, int));

#include "thunkapi.h"

void FdSetApiCalls(struct fdthunk_api *calls);

#ifdef __cplusplus
}
#endif

#endif
