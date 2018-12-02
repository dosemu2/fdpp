/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2018  Stas Sergeev (stsp)
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

#include "thunks_priv.h"
%include "segs.inc"
%include "plt_seg.inc"

%define _SEG(s) _SEG_%+s

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
    dw _%1, _SEG(_%1)
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
    ; cx = number of argument words
    jcxz %%11
    mov bx, cx
    dec bx
    add bx, bx
%%1:
    mov di, sp
    push word [ss : bx + di + 8]
    loop %%1
%%11:
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

segment INIT_TEXT

__asm_callsymtab_start:
%include "plt_asmc.inc"
%include "plt_asmp.inc"
__asm_callsymtab_end:

#define __ASM(t, n) asmsym n
#define __ASM_FAR(t, n) asmsym n
#define __ASM_NEAR(t, n) asmsym n
#define __ASM_ARR(t, n, l) asmsym n
#define __ASM_ARRI(t, n) asmsym n
#define __ASM_ARRI_F(t, n) asmsym n
#define __ASM_FUNC(n) asmsym n
#define SEMIC
__asm_symtab_start:
#include <glob_asm.h>
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
    mov ah, FDPP_KERNEL_VERSION
    mov bh, DOS_HELPER_PLUGIN_ID_FDPP
    mov bl, DOS_SUBHELPER_DL_SET_SYMTAB
    push word IGROUP
    push word init_near_wrp
    push word TGROUP
    push word near_wrp
    push word 2    ; amount of near wrappers
    push word __asm_callsymtab_end - __asm_callsymtab_start
    push word IGROUP
    push word __asm_callsymtab_start
    push word __asm_symtab_end - __asm_symtab_start
    push word IGROUP
    push word __asm_symtab_start
    push cs
    push word IGROUP
    push word IGROUP
    push word __InitTextEnd
    push word IGROUP
    push word __InitTextStart
    int DOS_HELPER_INT
    mov sp, bp
    pop bp
    ret
