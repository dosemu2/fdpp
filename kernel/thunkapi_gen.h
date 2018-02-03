#ifndef THUNKAPI_H
#define THUNKAPI_H

struct fdthunk_api {
#define _THUNK_API_v(n) void (* n)(void)
#define _THUNK_API_0(r, n) r (* n)(void)
#define _THUNK_API_1(r, n, t1, a1) r (* n)(t1 a1)
#define _THUNK_API_1v(n, t1, a1) void (* n)(t1 a1)
#define _THUNK_API_2v(n, t1, a1, t2, a2) void (* n)(t1 a1, t2 a2)
#define _THUNK_API_2(r, n, t1, a1, t2, a2) r (* n)(t1 a1, t2 a2)
#define _THUNK_API_3v(n, t1, a1, t2, a2, t3, a3) void (* n)(t1 a1, t2 a2, t3 a3)
#include "thunkapi_tmpl.h"
#undef _THUNK_API_v
#undef _THUNK_API_0
#undef _THUNK_API_1
#undef _THUNK_API_1v
#undef _THUNK_API_2v
#undef _THUNK_API_2
#undef _THUNK_API_3v
};

#endif
