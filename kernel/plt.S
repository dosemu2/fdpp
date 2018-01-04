#define DOS_HELPER_DL		    0x60
#define DOS_HELPER_DL_VERSION	    1
#define DOS_SUBHELPER_DL_CCALL	    0

#define DOS_HELPER_INT		    0xE6

%include "segs.inc"

%macro asmcfunc 3
segment %3
    global _%1
    _%1:
    mov al, DOS_HELPER_DL
    mov ah, DOS_HELPER_DL_VERSION
    mov bl, DOS_SUBHELPER_DL_CCALL
    mov cx, %2
    int DOS_HELPER_INT
%endmacro

%include "plt.inc"

#include "../hdr/version.h"

    global _os_release
_os_release: db KERNEL_VERSION_STRING
