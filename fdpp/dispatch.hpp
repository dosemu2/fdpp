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

#include <functional>

enum DispStat { DISP_OK, DISP_NORET };

template<typename T, typename ...A>
static inline int _dispatch(enum DispStat *rs,
        std::function<T(A...)> func, A... a)
{
    int ret;
    try {
        ret = func(a...);
        *rs = DISP_OK;
    }
    catch (int p) {
        ret = p;
        *rs = DISP_NORET;
    }
    return ret;
}

template<typename T, typename ...A>
static inline int fdpp_dispatch(enum DispStat *rs, T (*func)(A... fa), A... a)
{
    return _dispatch(rs, std::function<T(A...)>(func), a...);
}

template<typename ...A>
static inline int fdpp_dispatch_v(enum DispStat *rs, void (*func)(A... fa), A... a)
{
    return _dispatch(
        rs,
        std::function<int(A...)>([func] (A... _a) {
            func(_a...);
            return 0;
        }),
        a...
    );
}

static inline void fdpp_noret(int stat)
{
    throw(stat);
}
