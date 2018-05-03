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
    int refcnt;
};

struct farhlp g_farhlp[FARHLP_MAX];

static __attribute__((constructor)) void init(void)
{
    int i;

    for (i = 0; i < FARHLP_MAX; i++)
        farhlp_init(&g_farhlp[i]);
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
        struct f_m *fm = &ctx->far_map[i];
        if (!fm->refcnt)
            continue;
        if (fm->p == ptr)
            return i;
    }
    return -1;
}

static int find_unused(struct farhlp *ctx)
{
    int i;

    for (i = 0; i < ctx->f_m_len; i++) {
        if (!ctx->far_map[i].refcnt)
            return i;
    }
    return -1;
}

static void _store_far(struct farhlp *ctx, const void *ptr, far_t fptr,
    int rplc)
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
        if (!rplc) {
            far_t *f = &ctx->far_map[idx].f;
            _assert(f->seg == fptr.seg && f->off == fptr.off);
            /* already exists, do nothing */
            return;
        }
    } else {
        idx = find_unused(ctx);
    }
    if (idx == -1)
        idx = ctx->f_m_len++;
    fm = &ctx->far_map[idx];
    fm->p = ptr;
    fm->f = fptr;
    fm->refcnt = 1;
}

void store_far(struct farhlp *ctx, const void *ptr, far_t fptr)
{
    _store_far(ctx, ptr, fptr, 0);
}

void store_far_replace(struct farhlp *ctx, const void *ptr, far_t fptr)
{
    _store_far(ctx, ptr, fptr, 1);
}

far_t lookup_far(struct farhlp *ctx, const void *ptr)
{
    int idx = do_lookup(ctx, ptr);

    if (idx == -1)
        return (far_t){0, 0};
    return ctx->far_map[idx].f;
}

far_t lookup_far_ref(struct farhlp *ctx, const void *ptr)
{
    struct f_m *fm;
    int idx = do_lookup(ctx, ptr);

    if (idx == -1)
        return (far_t){0, 0};
    fm = &ctx->far_map[idx];
    fm->refcnt++;
    return fm->f;
}

far_t lookup_far_unref(struct farhlp *ctx, const void *ptr, int *rm)
{
    struct f_m *fm;
    far_t ret;
    int idx = do_lookup(ctx, ptr);

    if (idx == -1)
        return (far_t){0, 0};
    fm = &ctx->far_map[idx];
    ret = fm->f;
    fm->refcnt--;
    *rm = 0;
    if (!fm->refcnt) {
        *rm = 1;
        while (ctx->f_m_len && !ctx->far_map[ctx->f_m_len - 1].refcnt)
            ctx->f_m_len--;
    }
    return ret;
}
