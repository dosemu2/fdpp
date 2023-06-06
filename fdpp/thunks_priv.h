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

#ifndef THUNKS_PRIV_H
#define THUNKS_PRIV_H

#define FDPP_KERNEL_VERSION          4

#ifndef __ASSEMBLER__

void *resolve_segoff(struct far_s fa);
void *resolve_segoff_fd(struct far_s fa);
void thunk_call_void(struct far_s fa);
void thunk_call_void_noret(struct far_s fa);
struct far_s lookup_far_st(const void *ptr);
void fdprintf(const char *format, ...) PRINTF(1);
void fdlogprintf(const char *format, ...) PRINTF(1);
void fdloudprintf(const char *format, ...) PRINTF(1);
void fdvprintf(const char *format, va_list vl);
void fddebug(const BYTE * s, ...);
int is_dos_space(const void *ptr);

#endif

#endif
