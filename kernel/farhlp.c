#include <stdlib.h>
#include "portab.h"
#include "farhlp.h"

/* hackish helper to store/lookup far pointers - using static
 * object (map) is an ugly hack in an OOP world.
 * Need this to work around some C++ deficiencies, see comments
 * in farptr.hpp */

struct f_m {
    const void *p;
    far_t f;
};

struct farhlp g_farhlp;

static __attribute__((constructor)) void init(void)
{
    farhlp_init(&g_farhlp);
}

#define INIT_SIZE 128

void farhlp_init(struct farhlp *ctx)
{
    ctx->f_m_size = INIT_SIZE;
    ctx->f_m_len = 0;
    ctx->far_map = (struct f_m *)malloc(ctx->f_m_size * sizeof(struct f_m));
    _assert(ctx->far_map);
}

static int do_lookup(struct farhlp *ctx, const void *ptr)
{
    int i;

    for (i = 0; i < ctx->f_m_len; i++) {
        if (ctx->far_map[i].p == ptr)
            return i;
    }
    return -1;
}

void store_far(struct farhlp *ctx, const void *ptr, far_t fptr)
{
    int idx;
    struct f_m *fm;

    _assert(ctx->f_m_size && ctx->f_m_len <= ctx->f_m_size);
    if (ctx->f_m_len == ctx->f_m_size) {
        ctx->f_m_size *= 2;
        ctx->far_map = (struct f_m *)realloc(ctx->far_map,
                ctx->f_m_size * sizeof(struct f_m));
        _assert(ctx->far_map);
    }
    idx = do_lookup(ctx, ptr);
    if (idx != -1) {
        far_t *f = &ctx->far_map[idx].f;
        _assert(f->seg == fptr.seg && f->off == fptr.off);
        /* already exists, do nothing */
        return;
    }
    fm = &ctx->far_map[ctx->f_m_len++];
    fm->p = ptr;
    fm->f = fptr;
}

far_t lookup_far(struct farhlp *ctx, const void *ptr)
{
    int idx = do_lookup(ctx, ptr);
    if (idx == -1)
        return (far_t){0, 0};
    return ctx->far_map[idx].f;
}
