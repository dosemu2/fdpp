%include "segs.inc"
#include "../hdr/version.h"

segment IC

    global _os_release
_os_release: db KERNEL_VERSION_STRING
