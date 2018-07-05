#ifndef THUNKS_H
#define THUNKS_H

#include <stdint.h>
#include <stdarg.h>

#define FDPP_API_VER 5

#ifdef __cplusplus
extern "C" {
#endif

struct vm86_regs;
void FdppCall(struct vm86_regs *regs);

typedef void (*FdppAsmCall_t)(struct vm86_regs *regs, uint16_t seg,
        uint16_t off, uint8_t *sp, uint8_t len);

struct fdpp_api {
    uint8_t *(*mem_base)(void);
    void (*abort)(const char *, int);
    void (*print)(int prio, const char *format, va_list ap);
    void (*debug)(const char *msg);
    void (*panic)(const char *msg);
    void (*cpu_relax)(void);
    FdppAsmCall_t asm_call;
    FdppAsmCall_t asm_call_noret;
};
int FdppInit(struct fdpp_api *api, int ver, int *req_ver);

#ifdef __cplusplus
}
#endif

#endif
