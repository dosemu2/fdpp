#ifndef THUNKS_H
#define THUNKS_H

#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "thunkapi.h"

uint32_t FdppThunkCall(int fn, uint8_t *sp, uint8_t *r_len);

typedef uintptr_t (*FdppAsmCall_t)(uint16_t seg, uint16_t off, uint8_t *sp, uint8_t len);
struct asm_dsc_s;
void FdppSetAsmCalls(struct asm_dsc_s *tab, int size);
struct far_s;
int FdppSetAsmThunks(struct far_s *ptrs, int size);
struct fdpp_api {
    uint8_t *(*mem_base)(void);
    void (*abort_handler)(const char *, int);
    void (*print_handler)(const char *format, va_list ap);
    FdppAsmCall_t asm_call;
    struct fdthunk_api thunks;
};
void FdppInit(struct fdpp_api *api);

#ifdef __cplusplus
}
#endif

#endif
