/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2023  Stas Sergeev (stsp)
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

#include "portab.h"
#include "globals.h"
#include "proto.h"
#include "dispatch.hpp"
#include "thunks_c.h"

#define _ARG(n, t, ap) (*(t *)(ap + n))
#define _ARG_PTR(n, t, ap) // unimplemented, will create syntax error
#define _ARG_PTR_FAR(n, t, ap)  ({ \
    UDWORD __d = *(UDWORD *)(ap + n); \
    FP_FROM_D(t, __d); \
})
#define _ARG_R(t) t
#define _RET(r) r
#define _RET_PTR(r) // unused

UDWORD FdppThunkCall(int fn, UBYTE *sp, enum DispStat *r_stat, int *r_len)
{
    UDWORD ret;
    UBYTE rsz = 0;
    enum DispStat stat;

#define _SP sp
#define _DISP_CMN(f, c) { \
    fdlogprintf("dispatch " #f "\n"); \
    objtrace_enter(); \
    c; \
    objtrace_leave(); \
    fdlogprintf("dispatch " #f " done\n"); \
}
#define _DISPATCH(r, rv, rc, f, ...) _DISP_CMN(f, { \
    rv _r = fdpp_dispatch(&stat, f, ##__VA_ARGS__); \
    ret = rc(_r); \
    if (stat == DISP_OK) \
        rsz = (r); \
})
#define _DISPATCH_v(f, ...) _DISP_CMN(f, { \
    ret = fdpp_dispatch_v(&stat, f, ##__VA_ARGS__); \
})

    switch (fn) {
        #include <thunk_calls.h>

        default:
            fdprintf("unknown fn %i\n", fn);
            _fail();
            return 0;
    }

    *r_stat = stat;
    *r_len = rsz;
    return ret;
}
