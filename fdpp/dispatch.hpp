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

#include <csetjmp>
#include <type_traits>

#define fdpp_dispatch(c) ({ \
    int _r; \
    try { \
        _r = c; \
    } catch (std::jmp_buf env) { \
        if (posth) \
            posth(); \
        posth = NULL; \
        std::longjmp(env, 1); \
    } \
    _r; \
})

#define fdpp_dispatch_v(c) do { \
    try { \
        c; \
    } catch (std::jmp_buf env) { \
        if (posth) \
            posth(); \
        posth = NULL; \
        std::longjmp(env, 1); \
    } \
} while(0)

static void (*posth)(void);

static inline void fdpp_ljmp(std::jmp_buf env, void (*post)(void))
{
    posth = post;
    throw(env);
}
