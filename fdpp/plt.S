/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2018-2023  Stas Sergeev (stsp)
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
#include "bprm.h"
%include "segs.inc"

extern _BootParamSeg

segment _BSS
fdpp_plt dd 0

%macro call_plt 0
    push ds
    mov di, DGROUP
    mov ds, di
    call far [fdpp_plt]
    pop ds
%endmacro

%macro asmcfunc 4
segment %3
    global _%1
    _%1:
    mov al, 0 ; reserved
    mov bx, 0 ; reserved
    mov cx, %2
    mov dx, sp
    add dx, %4
    mov si, cs
    call_plt
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

segment HMA_TEXT

near_wrp:
    nearwrp

segment INIT_TEXT

init_near_wrp:
    nearwrp

    global plt_init
plt_init:
    ; copy plt entry
    mov ax, FDPP_BS_SEG
    mov es, ax
    mov ax, [es:FDPP_BS_OFF+FDPP_PLT_OFFSET]
    mov bx, [es:FDPP_BS_OFF+FDPP_PLT_OFFSET+2]
    mov [fdpp_plt], ax
    mov [fdpp_plt + 2], bx
    ret
