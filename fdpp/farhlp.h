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

#ifndef FARHLP_H
#define FARHLP_H

struct f_m;

struct farhlp {
    struct f_m *far_map;
    int f_m_size;
    int f_m_len;
};

enum { FARHLP1, FARHLP2, FARHLP_MAX };
extern struct farhlp g_farhlp[FARHLP_MAX];

#ifdef __cplusplus
extern "C" {
#endif
void farhlp_init(struct farhlp *ctx);
void store_far(struct farhlp *ctx, const void *ptr, far_t fptr);
void store_far_replace(struct farhlp *ctx, const void *ptr, far_t fptr);
struct far_s lookup_far(struct farhlp *ctx, const void *ptr);
struct far_s lookup_far_ref(struct farhlp *ctx, const void *ptr);
struct far_s lookup_far_unref(struct farhlp *ctx, const void *ptr, int *rm);
int purge_far(struct farhlp *ctx);
#ifdef __cplusplus
}
#endif

#endif
