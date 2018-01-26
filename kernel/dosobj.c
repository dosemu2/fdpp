#include <assert.h>
#include <string.h>
#include "portab.h"
#include "dyndata.h"
#include "smalloc.h"
#include "dosobj.h"

static smpool pool;
static uint8_t FAR *base;

void dosobj_init(void)
{
    const int size = 512;
    void FAR *fa = DynAlloc("dosobj", 1, size);
    void *ptr = resolve_segoff(GET_FAR(fa));

    sminit(&pool, ptr, size);
    base = fa;
}

void FAR *mk_dosobj(const void *data, UWORD len)
{
    void *ptr = smalloc(&pool, len);
    uint16_t offs;

    assert(ptr);
    offs = (uintptr_t)ptr - (uintptr_t)smget_base_addr(&pool);
    return base + offs;
}

void pr_dosobj(void FAR *fa, const void *data, UWORD len)
{
    void *ptr = resolve_segoff(GET_FAR(fa));

    memcpy(ptr, data, len);
}

void cp_dosobj(void *data, const void FAR *fa, UWORD len)
{
    void *ptr = resolve_segoff(GET_FAR(fa));

    memcpy(data, ptr, len);
}

void rm_dosobj(void FAR *fa)
{
    void *ptr = resolve_segoff(GET_FAR(fa));

    smfree(&pool, ptr);
}
