%macro asmcfunc 2
    global _%1
    _%1:
    nop
%endmacro

%include "plt.inc"

#include "../hdr/version.h"

    global _os_release
_os_release: db KERNEL_VERSION_STRING
