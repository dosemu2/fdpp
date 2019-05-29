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

enum DispStat { DISP_OK, DISP_NORET };

#define LJ_WRAP(c, rv, rs) do { \
    try { \
        c; \
        rs = DISP_OK; \
    } \
    catch (int p) { \
        rv = p; \
        rs = DISP_NORET; \
    } \
} while(0)

template<typename T, typename ...A>
static inline int fdpp_dispatch(enum DispStat *rs, T (*func)(A... fa), A... a)
{
    int ret;
    LJ_WRAP(ret = func(a...), ret, *rs);
    return ret;
}

template<typename ...A>
static inline int fdpp_dispatch_v(enum DispStat *rs, void (*func)(A... fa), A... a)
{
    int ret = 0;
    LJ_WRAP(func(a...), ret, *rs);
    return ret;
}

static inline void fdpp_noret(int stat)
{
    throw(stat);
}
