/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2018  Stas Sergeev (stsp)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include "portab.h"
#include "thunks_priv.h"
#include "farhlp.hpp"

/* hackish helper to store/lookup far pointers - using static
 * object (map) is an ugly hack in an OOP world.
 * Need this to work around some C++ deficiencies, see comments
 * in farptr.hpp */

farhlp g_farhlp[FARHLP_MAX];

void farhlp_init(farhlp *ctx)
{
}

void farhlp_deinit(farhlp *ctx)
{
    ctx->map.clear();
}

void store_far(farhlp *ctx, const void *ptr, far_t fptr)
{
    struct f_m *fm;
    const std::pair<decltype(ctx->map)::iterator, bool> &p =
            ctx->map.insert(std::make_pair(ptr,
            decltype(ctx->map)::mapped_type()));
    if (!p.second) {
        far_t *f = &p.first->second.f;
        _assert(f->seg == fptr.seg && f->off == fptr.off);
        /* already exists, do nothing */
        return;
    }
    fm = &p.first->second;
    fm->f = fptr;
    fm->refcnt = 1;
}

void store_far_replace(farhlp *ctx, const void *ptr, far_t fptr)
{
    struct f_m *fm = &ctx->map[ptr];
    fm->f = fptr;
    fm->refcnt = 1;
}

far_t lookup_far(farhlp *ctx, const void *ptr)
{
    decltype(ctx->map)::iterator it = ctx->map.find(ptr);

    if (it == ctx->map.end())
        return (far_t){0, 0};
    return it->second.f;
}

far_t lookup_far_ref(farhlp *ctx, const void *ptr)
{
    struct f_m *fm;
    decltype(ctx->map)::iterator it = ctx->map.find(ptr);

    if (it == ctx->map.end())
        return (far_t){0, 0};
    fm = &it->second;
    fm->refcnt++;
    return fm->f;
}

far_t lookup_far_unref(farhlp *ctx, const void *ptr, int *rm)
{
    struct f_m *fm;
    far_t ret;
    int r;
    decltype(ctx->map)::iterator it = ctx->map.find(ptr);

    if (it == ctx->map.end())
        return (far_t){0, 0};
    fm = &it->second;
    ret = fm->f;
    fm->refcnt--;
    r = 0;
    if (!fm->refcnt) {
        r = 1;
        ctx->map.erase(it);
    }
    if (rm)
        *rm = r;
    return ret;
}

int purge_far(farhlp *ctx)
{
    int old = ctx->map.size();
    ctx->map.clear();
    return old;
}
