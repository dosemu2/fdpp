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

#define LJ_WRAP(c, c2, r) do { \
    try { \
        c; \
    } \
    catch (void *p) { \
        r = p; \
        c2; \
    } \
} while(0)

template<typename T, typename ...args, typename ...Args>
static inline T fdpp_dispatch(void **rt, T (*func)(args... fa), Args... a)
{
    T ret;
    static_assert(!std::is_void<T>::value, "something wrong");
    LJ_WRAP(ret = func(a...), ret = 0, *rt);
    return ret;
}

template<typename ...args, typename ...Args>
static inline void fdpp_dispatch_v(void **rt, void (*func)(args... fa), Args... a)
{
    LJ_WRAP(func(a...),, *rt);
}

static inline void fdpp_noret(void *p = NULL)
{
    throw(p);
}
