#include <string.h>
#include <stdarg.h>
#include "portab.h"
#include "dyndata.h"
#include "smalloc.h"
#include "farhlp.h"
#include "dosobj.h"

static smpool pool;
static __DOSFAR(uint8_t) base;
static struct farhlp hlp;
static int initialized;

static void err_printf(int prio, const char *fmt, ...) PRINTF(2);
static void err_printf(int prio, const char *fmt, ...)
{
    va_list vl;

    va_start(vl, fmt);
    fdvprintf(fmt, vl);
    va_end(vl);
}

void dosobj_init(int size)
{
    void FAR *fa = DynAlloc("dosobj", 1, size);
    void *ptr = resolve_segoff(GET_FAR(fa));

    sminit(&pool, ptr, size);
    smregister_error_notifier(&pool, err_printf);
    base = fa;
    farhlp_init(&hlp);
    initialized = 1;
}

__DOSFAR(uint8_t) mk_dosobj(const void *data, UWORD len)
{
    void *ptr;
    uint16_t offs;

    _assert(initialized);
    ptr = smalloc(&pool, len);
    if (!ptr) {
        fdprintf("dosobj: OOM! len=%i\n", len);
        _fail();
    }
    offs = (uintptr_t)ptr - (uintptr_t)smget_base_addr(&pool);
    return base + offs;
}

void pr_dosobj(__DOSFAR(uint8_t) fa, const void *data, UWORD len)
{
    void *ptr = resolve_segoff(GET_FAR(fa));

    memcpy(ptr, data, len);
}

void cp_dosobj(void *data, __DOSFAR(uint8_t) fa, UWORD len)
{
    void *ptr = resolve_segoff(GET_FAR(fa));

    memcpy(data, ptr, len);
}

void rm_dosobj(__DOSFAR(uint8_t) fa)
{
    void *ptr = resolve_segoff(GET_FAR(fa));

    smfree(&pool, ptr);
}

uint16_t dosobj_seg(void)
{
    return FP_SEG(base);
}

__DOSFAR(uint8_t) mk_dosobj_st(const void *data, UWORD len)
{
    far_t f;
    __DOSFAR(uint8_t) ret;

    _assert(initialized);
    f = lookup_far_ref(&hlp, data);
    if (f.seg || f.off)
        return __DOSFAR(uint8_t)(f);
    ret = mk_dosobj(data, len);
    store_far(&hlp, data, GET_FAR(ret));
    return ret;
}

void rm_dosobj_st(const void *data)
{
    void *ptr;
    int rm;
    far_t f = lookup_far_unref(&hlp, data, &rm);

    _assert(f.seg || f.off);
    if (rm) {
        ptr = resolve_segoff(f);
        smfree(&pool, ptr);
    }
}
