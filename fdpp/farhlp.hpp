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

#ifndef FARHLP_HPP
#define FARHLP_HPP

#include <unordered_map>

struct f_m {
    const void *p;
    far_t f;
    int refcnt;
};
struct farhlp {
    std::unordered_map<const void *, struct f_m> map;
};

enum { FARHLP1, FARHLP2, FARHLP_MAX };
extern farhlp g_farhlp[FARHLP_MAX];

void farhlp_init(farhlp *ctx);
void farhlp_deinit(farhlp *ctx);
void store_far(farhlp *ctx, const void *ptr, far_t fptr);
void store_far_replace(farhlp *ctx, const void *ptr, far_t fptr);
struct far_s lookup_far(farhlp *ctx, const void *ptr);
struct far_s lookup_far_ref(farhlp *ctx, const void *ptr);
struct far_s lookup_far_unref(farhlp *ctx, const void *ptr, int *rm);
int purge_far(farhlp *ctx);

#endif
