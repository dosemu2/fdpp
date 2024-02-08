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
#include "farptr.hpp"
#include "thunks_p.h"

#define __ARG(t) t
#define __ARG_PTR(t) t *
#define __ARG_PTR_FAR(t) __FAR(t)
#define __ARG_A(t) t
#define __ARG_PTR_A(t) NEAR_PTR_DO(t, !!(_flags & _TFLG_NORET))
#define __ARG_PTR_FAR_A(t) __DOSFAR2(t, !!(_flags & _TFLG_NORET))
#define __RET(t, v) v
#define __RET_PTR(t, v) // unimplemented, will create syntax error
#define __RET_PTR_FAR(t, v) FP_FROM_D(t, v)
#define __CALL(n, s, l, f) do_asm_call(n, s, l, f)
#define __CSTK(l) clean_stk(l)

#define __CNV_PTR_FAR(t, d, f, l, t0) t d = (f)
#define __CNV_PTR(t, d, f, l, t0) \
    _MK_FAR_SZ(__##d, f, l); \
    t d = __MK_NEAR2(__##d, t)
#define __CNV_PTR_CCHAR(t, d, f, l, t0) \
    _MK_FAR_STR(__##d, f); \
    t d = __MK_NEAR(__##d)
#define __CNV_PTR_ARR(t, d, f, l, t0) \
    _MK_FAR_SZ(__##d, f, l); \
    t d = __MK_NEAR(__##d)
#define __CNV_PTR_VOID(t, d, f, l, t0) \
    _MK_FAR_SZ(__##d, f, l); \
    t d = __MK_NEAR(__##d)
#define __CNV_SIMPLE(t, d, f, l, t0) t d = (f)

#define _CNV(c, t, at, l, n) c(at, _a##n, a##n, l, t)
#define _L_REF(nl) a##nl
#define _L_IMM(n, l) (sizeof(*_L_REF(n)) * (l))
#define _L_SZ(n) sizeof(*_L_REF(n))

#include <thunk_asms.h>
