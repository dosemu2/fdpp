#ifndef THUNKS_H
#define THUNKS_H

#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t FdppThunkCall(int fn, uint8_t *sp, uint8_t *r_len);

typedef uintptr_t (*FdppAsmCall_t)(uint16_t seg, uint16_t off, uint8_t *sp, uint8_t len);
struct asm_dsc_s;
void FdppSetAsmCalls(FdppAsmCall_t call, struct asm_dsc_s *tab, int size);
struct far_s;
int FdppSetAsmThunks(struct far_s *ptrs, int size);
struct fdpp_api {
    uint8_t *mem_base;
    void (*abort_handler)(const char *, int);
    void (*print_handler)(const char *format, va_list ap);
};
void FdppInit(struct fdpp_api *api);

#include "thunkapi.h"

void FdSetApiCalls(struct fdthunk_api *calls);

#ifdef __cplusplus
}
#endif

#endif
