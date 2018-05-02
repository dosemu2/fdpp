#define DOS_HELPER_PLUGIN            0x60
#define DOS_HELPER_PLUGIN_ID_FDPP    0
#define DOS_HELPER_PLUGIN_VERSION    1
#define DOS_SUBHELPER_DL_SET_SYMTAB  0
#define DOS_SUBHELPER_DL_CCALL       1

#define DOS_HELPER_INT               0xE6

%include "segs.inc"

%macro asmcfunc 4
segment %3
    global _%1
    _%1:
    mov al, DOS_HELPER_PLUGIN
    mov bh, DOS_HELPER_PLUGIN_ID_FDPP
    mov bl, DOS_SUBHELPER_DL_CCALL
    mov cx, %2
    mov dx, sp
    add dx, %4
    mov si, cs
    int DOS_HELPER_INT
%endmacro

%macro asmcfunc_f 3
    asmcfunc %1, %2, %3, 4
    retf
%endmacro

%macro asmcfunc_n 3
    asmcfunc %1, %2, %3, 2
    ret
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

%macro nearwrp 0
    push bp
    mov bp, sp
    sub sp, 2
    mov bx, cx
    dec bx
    add bx, bx
%%1:
    mov di, sp
    push word [ss : bx + di + 8]
    loop %%1
    mov [bp - 2], sp
    call ax
    mov bx, sp
    sub bx, [bp - 2]
    mov sp, bp
    pop bp
    ; see if C or pascal stack
    or bx, bx
    jz %%2
    ; remove pascal arguments
    cmp bx, 2
    je %%3
    mov di, sp
    ; move return addr upwards
    pop word [ss : di + bx]
    pop word [ss : di + bx + 2]
    ; remove whatever remained
    sub bx, 4
    add sp, bx
%%2:
    retf
%%3:
    retf 2
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
#define __ASM_ARRI_F(t, n) asmsym n
#define __ASM_FUNC(n) asmsym n
#define SEMIC
__asm_symtab_start:
#include "glob_asm.h"
__asm_symtab_end:

segment HMA_TEXT

near_wrp:
    nearwrp

segment INIT_TEXT

init_near_wrp:
    nearwrp

    global plt_init
plt_init:
    push bp
    mov bp, sp
    mov al, DOS_HELPER_PLUGIN
    mov ah, DOS_HELPER_PLUGIN_VERSION
    mov bh, DOS_HELPER_PLUGIN_ID_FDPP
    mov bl, DOS_SUBHELPER_DL_SET_SYMTAB
    push word SEG init_near_wrp
    push word init_near_wrp
    push word SEG near_wrp
    push word near_wrp
    push word 2    ; amount of near wrappers
    push word __asm_callsymtab_end - __asm_callsymtab_start
    push word __asm_callsymtab_start
    push word __asm_symtab_end - __asm_symtab_start
    push word __asm_symtab_start
    int DOS_HELPER_INT
    mov sp, bp
    pop bp
    ret
