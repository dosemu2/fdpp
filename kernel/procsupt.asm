;
; File:
;                         procsupt.asm
; Description:
;     Assembly support routines for process handling, etc.
;
;                     Copyright (c) 1995,1998
;                       Pasquale J. Villani
;                       All Rights Reserved
;
; This file is part of DOS-C.
;
; DOS-C is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version
; 2, or (at your option) any later version.
;
; DOS-C is distributed in the hope that it will be useful, but
; WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
; the GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public
; License along with DOS-C; see the file COPYING.  If not,
; write to the Free Software Foundation, 675 Mass Ave,
; Cambridge, MA 02139, USA.
;
; $Id: procsupt.asm 1591 2011-05-06 01:46:55Z bartoldeman $
;


		%include "segs.inc"

                extern  _user_r

                extern  _break_flg     ; break detected flag
                extern  _int21_handler

                %include "stacks.inc"

segment HMA_TEXT

                extern   _DGROUP_

;
;       Special call for switching processes
;
;       void exec_user(exec_regs far *erp)
;
                global  _exec_user
_exec_user:
                pop     ax		      ; return address (unused)

                pop     bp		      ; erp (user ss:sp)
                pop     si
                cli
                mov     ss,si
                mov     sp,bp
;    /* start allocating REGs (as in MS-DOS - some demos expect them so --LG) */
;    /* see http://www.beroset.com/asm/showregs.asm */
                pop     bx
                mov     cx,0ffh
                mov     si,[bp + 6] ; IP
                pop     di
                pop     ax
                mov     ds,ax
                mov     es,ax
                mov     dx,ax
                mov     ax,bx
                mov     bp,091eh ;this is more or less random but some programs
                                 ;expect 0x9 in the high byte of BP!! */
                iret

                global  _ret_user
_ret_user:
                pop     ax      ; return address (unused)

                pop     bp      ; irp (user ss:sp)
                pop     si
                cli
                mov     ss,si
                mov     sp,bp
                POP$ALL
                iret

segment _LOWTEXT


;; Called whenever the BIOS detects a ^Break state
                global  _got_cbreak
_got_cbreak:
	push ds
	push ax
	mov ax, 40h
	mov ds, ax
	or byte [71h], 80h	;; set the ^Break flag
	pop ax
	pop ds
	iret

segment	HMA_TEXT

                global  _spawn_int23
_spawn_int23:
		mov ds, [cs:_DGROUP_]		;; Make sure DS is OK
		mov bp, [_user_r]
		cli
		mov ss, [_user_r+2]
		RestoreSP
		sti
		; get all the user registers back
		Restore386Registers
		POP$ALL
		push bp
		mov bp, sp
		clc
		int 23h
		mov sp, bp
		pop bp
		jnc ??int23_respawn
		;; The user returned via RETF 0, Carry is set
		;; --> terminate program
		;; This is done by set the _break_flg and modify the
		;; AH value, which is passed to the _respawn_ call
		;; into 0, which is "Terminate program".
		push ds                 ;; we need DGROUP
		mov ds, [cs:_DGROUP_]
		inc byte [_break_flg]
		pop ds
		xor ax, ax		;; clear ah --> perform DOS-00 --> terminate
??int23_respawn:
		jmp DGROUP:_int21_handler

;
; interrupt enable and disable routines
;
;                public  _enable
;_enable         proc near
;                sti
;                ret
;_enable         endp
;
;                public  _disable
;_disable        proc near
;                cli
;                ret
;_disable        endp

        extern _p_0_tos,_P_0,_P_0_bad,_P_0_exit

; prepare to call process 0 (the shell) from P_0() in C

    global reloc_call_call_p_0
reloc_call_call_p_0:
        pop ax          ; return address (32-bit, unused)
        pop ax
        pop ax          ; fetch parameter 0 (32-bit) from the old stack
        pop dx
        mov ds,[cs:_DGROUP_]
        cli
        mov ss,[cs:_DGROUP_]
        mov sp,_p_0_tos ; load the dedicated process 0 stack
        sti
        push dx         ; pass parameter 0 onto the new stack
        push ax
        call _P_0
        add sp, 4

.L_2:
        mov al, [_p0_exec_mode]

        mov bx, [_p0_execblk_p]
        mov es, [_p0_execblk_p + 2]

        mov dx, [_p0_cmdline_p]
        mov ds, [_p0_cmdline_p + 2] ; ds assigned last

        mov ah, 4bh
        int 21h
        mov ds,[cs:_DGROUP_]
        jc .L_1
        mov ah, 4dh    ; get exit code
        int 21h
        push ax
        call _P_0_exit
        pop ax
        jmp .L_2

.L_1:
        call _P_0_bad
        jmp .L_2

segment _DATA
    global _p0_cmdline_p
_p0_cmdline_p dd 0
    global _p0_execblk_p
_p0_execblk_p dd 0
    global _p0_exec_mode
_p0_exec_mode db 0
