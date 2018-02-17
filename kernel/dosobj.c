#include <string.h>
#include "portab.h"
#include "dyndata.h"
#include "smalloc.h"
#include "farhlp.h"
#include "dosobj.h"

static smpool pool;
static __DOSFAR(uint8_t) base;
static struct farhlp hlp;
static int initialized;

void dosobj_init(void)
{
    const int size = 512;
    void FAR *fa = DynAlloc("dosobj", 1, size);
    void *ptr = resolve_segoff(GET_FAR(fa));

    sminit(&pool, ptr, size);
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
    _assert(ptr);
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
