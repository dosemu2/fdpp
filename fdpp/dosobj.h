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

#ifndef DOSOBJ_H
#define DOSOBJ_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
void dosobj_init(far_t fa, int size);
far_t mk_dosobj(const void *data, UWORD len);
void pr_dosobj(far_t fa, const void *data, UWORD len);
void cp_dosobj(void *data, far_t fa, UWORD len);
void rm_dosobj(far_t fa);
void dosobj_dump(void);

uint16_t dosobj_seg(void);
#ifdef __cplusplus
}
#endif

#endif
