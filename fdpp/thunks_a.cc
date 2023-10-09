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
#include "thunks_a.h"

#define _E
#include "glob_tmpl.h"
#undef _E

#define __S(x) #x
#define _S(x) __S(x)

struct athunk asm_thunks[] = {
#define _A(v, w) { _S(v), __ASMREF(w), 0 }
#define SEMIC ,
#define __ASM(t, v) _A(_##v, __##v)
#define __ASM_FAR(t, v) _A(_##v, __##v)
#define __ASM_NEAR(t, v) { _S(_##v), __ASMREF(__##v), THUNKF_SHORT | THUNKF_DEEP }
#define __ASM_ARR(t, v, l) _A(_##v, __##v)
#define __ASM_ARRI(t, v) _A(_##v, __##v)
#define __ASM_ARRI_F(t, v) _A(_##v, __##v)
#define __ASM_FUNC(v) _A(_##v, __##v)
#include <glob_asm.h>
#undef __ASM
#undef __ASM_FAR
#undef __ASM_NEAR
#undef __ASM_ARR
#undef __ASM_ARRI
#undef __ASM_ARRI_F
#undef __ASM_FUNC
#undef SEMIC
};

const int num_athunks = _countof(asm_thunks);

struct athunk asm_cthunks[] = {
#define ASMCSYM(s, n) [n] = { _S(_##s), NULL, 0 },
#include "plt_asmc.h"
#define ASMPSYM(s, n) [n] = { _S(s), NULL, 0 },
#include "plt_asmp.h"
};

const int num_cthunks = _countof(asm_cthunks);
