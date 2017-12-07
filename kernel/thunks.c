#include "../hdr/portab.h"
#include "globals.h"
#include "proto.h"
#include "thunks.h"

#define _ARG(n, t, ap) (*(t *)(ap + n))
#define _ARG_PTR(n, t, ap) // unimplemented, will create syntax error
#define _ARG_PTR_FAR(n, t, ap)  ((t FAR *)(uintptr_t)*(UDWORD *)(ap + n))

UDWORD FdThunkCall(int fn, UBYTE *sp, UBYTE *r_len)
{
    UDWORD ret = 0;
#define _RET ret
#define _RSZ (*r_len)
#define _SP sp

    switch (fn) {
        #include "thunk_calls.h"
    }

    return ret;
}
