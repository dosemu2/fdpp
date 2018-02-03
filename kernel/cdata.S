%include "segs.inc"
#include "../hdr/version.h"

segment _FIXED_DATA

    global _os_release
_os_release: db KERNEL_VERSION_STRING
