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

/* needs the intermediate static store for functor because it may call
 * longjmp(), in which case the dtor of the functor won't be called.
 * So we need to leave the catch block first to get the dtor called,
 * and then call the functor via its static copy. */
#define fdpp_dispatch(c) ({ \
    static std::function<void(void)> post; \
    bool do_post = false; \
    int _r; \
    try { \
        _r = c; \
    } \
    catch (std::function<void(void)> p) { \
        do_post = true; \
        post = p; \
        _r = 0; \
    } \
    if (do_post) \
        post(); \
    _r; \
})

#define fdpp_dispatch_v(c) do { \
    static std::function<void(void)> post; \
    bool do_post = false; \
    try { \
        c; \
    } \
    catch (std::function<void(void)> p) { \
        do_post = true; \
        post = p; \
    } \
    if (do_post) \
        post(); \
} while(0)

#define fdpp_noret(c) \
{ \
    throw(std::function<void(void)>([=] () { \
        c; \
    })); \
}
