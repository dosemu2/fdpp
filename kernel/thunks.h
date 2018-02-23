#ifndef THUNKS_H
#define THUNKS_H

#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "thunkapi.h"

uint32_t FdppThunkCall(int fn, uint8_t *sp, uint8_t *r_len);

typedef void (*FdppAsmCall_t)(uint16_t seg, uint16_t off, uint8_t *sp, uint8_t len);
void FdppSetSymTab(void *tab);

enum FdppReg { REG_es, REG_cs, REG_ss, REG_ds, REG_fs, REG_gs,
  REG_eax, REG_ebx, REG_ecx, REG_edx, REG_esi, REG_edi,
  REG_ebp, REG_esp, REG_eip, REG_eflags };

struct fdpp_api {
    uint8_t *(*mem_base)(void);
    void (*abort_handler)(const char *, int);
    void (*print_handler)(const char *format, va_list ap);
    void (*cpu_relax)(void);
    FdppAsmCall_t asm_call;
    uint32_t (*getreg)(enum FdppReg reg);
    void (*setreg)(enum FdppReg reg, uint32_t value);
    struct fdthunk_api thunks;
};
void FdppInit(struct fdpp_api *api);

#ifdef __cplusplus
}
#endif

#endif
