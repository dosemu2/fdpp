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
    far_t f;
    int refcnt;
};
struct farhlp {
    std::unordered_map<const void *, struct f_m> map;
};

struct fh1 {
    const void *ptr;
    far_t f;
};
extern fh1 g_farhlp1;
extern farhlp g_farhlp2;

void farhlp_init(farhlp *ctx);
void store_far(farhlp *ctx, const void *ptr, far_t fptr);
void store_far_replace(farhlp *ctx, const void *ptr, far_t fptr);
struct far_s lookup_far(farhlp *ctx, const void *ptr);
struct far_s lookup_far_ref(farhlp *ctx, const void *ptr);
struct far_s lookup_far_unref(farhlp *ctx, const void *ptr, int *rm);

#endif
