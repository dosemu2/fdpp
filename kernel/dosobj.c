#include <assert.h>
#include <string.h>
#include "portab.h"
#include "dyndata.h"
#include "smalloc.h"
#include "thunks_priv.h"
#include "dosobj.h"

static smpool pool;
static void FAR *base;

void dosobj_init(void)
{
    const int size = 512;
    void FAR *fa = DynAlloc("dosobj", 1, size);
    void *ptr = resolve_segoff(fa);

    sminit(&pool, ptr, size);
    base = fa;
}

void FAR *mk_dosobj(const void *data, UWORD len)
{
    void *ptr = smalloc(&pool, len);
    uint16_t offs = (uintptr_t)ptr - (uintptr_t)smget_base_addr(&pool);

    assert(ptr);
    memcpy(ptr, data, len);
    return base + offs;
}

void rm_dosobj(void *data, const void FAR *fa, UWORD len)
{
    void *ptr = resolve_segoff(fa);

    memcpy(data, ptr, len);
    smfree(&pool, ptr);
}
