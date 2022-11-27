#ifndef FD_EXPORTS_H
#define FD_EXPORTS_H

#include <stdint.h>

struct far_s {
    uint16_t off;
    uint16_t seg;
};
typedef struct far_s far_t;

void do_abort(const char *file, int line);
#define _fail() do_abort(__FILE__, __LINE__)
#define ___assert(c) if (!(c)) _fail()
#define PRINTF(n) __attribute__((format(printf, n, n + 1)))
void fpanic(const char * s, ...) PRINTF(1);
void fdebug(const char * s, ...) PRINTF(1);
void cpu_relax(void);
void fdexit(int rc);
void _fd_mark_mem(far_t ptr, uint16_t size, int type);
#define fd_mark_mem(p, s, t) _fd_mark_mem(GET_FAR(p), s, t)
void _fd_prot_mem(far_t ptr, uint16_t size, int type);
#define fd_prot_mem(p, s, t) _fd_prot_mem(GET_FAR(p), s, t)
void _fd_mark_mem_np(far_t ptr, uint16_t size, int type);
#define fd_mark_mem_np(p, s, t) _fd_mark_mem_np(GET_FAR(p), s, t)

#endif
