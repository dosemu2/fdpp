#define DOS_HELPER_DL                0x60
#define DOS_HELPER_DL_VERSION        1
#define DOS_SUBHELPER_DL_SET_SYMTAB  0
#define DOS_SUBHELPER_DL_CCALL       1

#define DOS_HELPER_INT               0xE6

%include "segs.inc"

%macro asmcfunc 3
segment %3
    global _%1
    _%1:
    mov al, DOS_HELPER_DL
    mov bl, DOS_SUBHELPER_DL_CCALL
    mov cx, %2
    int DOS_HELPER_INT
%endmacro

%include "plt.inc"

%macro asmcsym 2
_%1_num dw %2
_%1_ptr:
    extern _%1
    dw _%1, SEG _%1
%endmacro

%macro asmpsym 2
_%1_num dw %2
_%1_ptr:
    extern %1
    dw %1, SEG %1
%endmacro

%macro asmsym 1
_%1_ptr:
    extern _%1
    dw _%1, SEG _%1
%endmacro

segment IC

__asm_callsymtab_start:
%include "plt_asmc.inc"
%include "plt_asmp.inc"
__asm_callsymtab_end:

#define __ASM(t, n) asmsym n
#define __ASM_FAR(t, n) asmsym n
#define __ASM_ARR(t, n, l) asmsym n
#define __ASM_ARRI(t, n) asmsym n
#define __ASM_FUNC(n) asmsym n
#define SEMIC
__asm_symtab_start:
#include "glob_asm.h"
__asm_symtab_end:

segment INIT_TEXT

    global plt_init
plt_init:
    mov al, DOS_HELPER_DL
    mov ah, DOS_HELPER_DL_VERSION
    mov bl, DOS_SUBHELPER_DL_SET_SYMTAB
    mov cx, __asm_callsymtab_end - __asm_callsymtab_start
    mov si, __asm_callsymtab_start
    mov dx, __asm_symtab_end - __asm_symtab_start
    mov di, __asm_symtab_start
    int DOS_HELPER_INT
    ret
