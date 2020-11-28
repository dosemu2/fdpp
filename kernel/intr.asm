; File:
;                         intr.asm
; Description:
;       Assembly implementation of calling an interrupt
;
;                    Copyright (c) 2000
;                       Steffen Kaiser
;                       All Rights Reserved
;
; This file is part of FreeDOS.
;
; FreeDOS is free software; you can redistribute it and/or
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
; write to the Free Software Foundation, Inc.,
; 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
;

		%include "segs.inc"

%macro INTR 2
        push bp                      ; Standard C entry
        mov  bp,sp
        push si
        push di
        push ds
        pushf

        mov ax, [bp+%1]               ; interrupt number
        mov [cs:.intr_1-1], al
        jmp short .intr_2           ; flush the instruction cache
.intr_2:
%if %2 == 1
        lds bx, [bp+4]               ; regpack structure FAR
%else
        mov bx, [bp+4]               ; regpack structure
%endif
        mov ax, [bx]
        mov cx, [bx+4]
        mov dx, [bx+6]
        mov si, [bx+8]
        mov di, [bx+10]
;        mov bp, [bx+12]
        push word [bx+14]            ; ds
        mov es, [bx+16]
        push word [bx+22]            ; flags
        popf
        mov bx, [bx+2]
        pop ds
        int 0
.intr_1:

        pushf
        push ds
        push bx
        mov bx, sp
%if %2 == 1
        lds bx, [bp+4]               ; regpack structure FAR
%else
        mov bx, [bp+4]               ; address of REGPACK
        mov ds, [bp-6]               ; take saved ds
%endif
        mov [bx], ax
        pop word [bx+2]              ; bx
        mov [bx+4], cx
        mov [bx+6], dx
        mov [bx+8], si
        mov [bx+10], di
;        mov [bx+12], bp
        pop word [bx+14]             ; ds
        mov [bx+16], es
        pop word [bx+22]             ; flags

        popf
        pop ds
        pop di
        pop si
        pop bp
%endmacro

segment HMA_TEXT
;
;       void ASMPASCAL call_intr(WORD nr, struct REGPACK FAR *rp)
;
    global CALL_INTR
CALL_INTR:
        INTR 8,1
        ret 6

;
;       void ASMPASCAL call_intr_func(void FAR *ptr, struct REGPACK FAR *rp)
;
        global CALL_INTR_FUNC
CALL_INTR_FUNC:
        push bp                      ; Standard C entry
        mov  bp,sp
        push ds
        push es
        pushf

        lds bx, [bp+4]               ; regpack structure
        mov ax, [bx]
        mov cx, [bx+4]
        mov dx, [bx+6]
        mov si, [bx+8]
        mov di, [bx+10]
;       mov bp, [bx+12]
        push word [bx+14]            ; ds
        mov es, [bx+16]
        push word [bx+22]            ; flags
        popf
        mov bx, [bx+2]
        pop ds
        pushf
        call far [bp+8]

        pushf
        push ds
        push bx
        mov bx, sp
        mov ds, [bp-2]          ; peek saved ds
        mov bx, [bp+4]          ; address of REGPACK
        mov ds, [bp+6]
        mov [bx], ax
        pop word [bx+2]         ; bx
        mov [bx+4], cx
        mov [bx+6], dx
        mov [bx+8], si
        mov [bx+10], di
;       mov [bx+12], bp
        pop word [bx+14]        ; ds
        mov [bx+16], es
        pop word [bx+22]        ; flags

        popf
        pop es
        pop ds
        pop bp
        ret 8

;WORD share_criterr(WORD flags, WORD err, struct dhdr FAR *lpDevice, UWORD ax)
                global  _share_criterr
_share_criterr:
                pushf
                push    cs
                push    criterr_ret
                push    es
                push    ds
                push    bp
                push    di
                push    si
                push    dx
                push    cx
                push    bx
                mov     bx, sp
                push    word [ss:bx + 24 + 8]   ; ax
                mov     si,  [ss:bx + 24 + 4]   ; off lpDevice
                mov     bp,  [ss:bx + 24 + 6]   ; seg lpDevice
                mov     di,  [ss:bx + 24 + 2]   ; err
                mov     ax,  [ss:bx + 24 + 0]   ; flags
                int     24h
                add     sp, 24
criterr_ret:
                ret


segment INIT_TEXT
;
;       void init_call_intr(nr, rp)
;       REG int nr
;       REG struct REGPACK *rp
;
		global	INIT_CALL_INTR
INIT_CALL_INTR:
		INTR 6,0
		ret 4

;
; int init_call_XMScall( (WORD FAR * driverAddress)(), WORD AX, WORD DX)
;
; this calls HIMEM.SYS
;
                global INIT_CALL_XMSCALL
INIT_CALL_XMSCALL:
            pop  bx         ; ret address
            pop  dx
            pop  ax
            pop  cx         ; driver address
            pop  es

            push cs         ; ret address
            push bx
            push es         ; driver address ("jmp es:cx")
            push cx
            retf

; void FAR *DetectXMSDriver(VOID)
global DETECTXMSDRIVER
DETECTXMSDRIVER:
        mov ax, 4300h
        int 2fh                 ; XMS installation check

        cmp al, 80h
        je detected
        xor ax, ax
        xor dx, dx
        ret

detected:
        push es
        push bx
        mov ax, 4310h           ; XMS get driver address
        int 2fh

        mov ax, bx
        mov dx, es
        pop bx
        pop es
        ret

global KEYCHECK
KEYCHECK:
        mov ah, 1
        int 16h
        ret

;; int exists(const char *pathname);
    global INIT_EXISTS
INIT_EXISTS:
        pop bx         ; ret address
        pop si         ; pathname
        push bx        ; ret address
        mov ax, 6c00h
        mov bx, 0
        mov cx, 0
        mov dx, 0
        int 21h
        cmp ax, 50h
        je file_exists
        xor ax, ax
exists_ret:
        ret
file_exists:
        mov ax, 1
        jmp exists_ret

;; int open(const char *pathname, int flags);
    global INIT_DOSOPEN
INIT_DOSOPEN:
        ;; init calling DOS through ints:
        pop bx         ; ret address
        pop ax         ; flags
        pop dx         ; pathname
        push bx        ; ret address
        mov ah, 3dh
        ;; AX will have the file handle

common_int21:
        int 21h
        jnc common_no_error
        mov ax, -1
common_no_error:
        ret

;; int close(int fd);
    global CLOSE
CLOSE:
        pop ax         ; ret address
        pop bx         ; fd
        push ax        ; ret address
        mov ah, 3eh
        jmp short common_int21

;; UCOUNT init_DosRead(int fd, void *buf, UCOUNT count);
    global INIT_DOSREAD
INIT_DOSREAD:
        pop ax         ; ret address
        pop cx         ; count
        pop dx         ; buf
        pop bx         ; fd
        push ax        ; ret address
        mov ah, 3fh
        jmp short common_int21

;; int dup2(int oldfd, int newfd);
    global DUP2
DUP2:
        pop ax         ; ret address
        pop cx         ; newfd
        pop bx         ; oldfd
        push ax        ; ret address
        mov ah, 46h
        jmp short common_int21

;
; ULONG ASMPASCAL lseek(int fd, long position);
;
    global LSEEK
LSEEK:
        pop ax         ; ret address
        pop dx         ; position low
        pop cx         ; position high
        pop bx         ; fd
        push ax        ; ret address
        mov ax,4200h   ; origin: start of file
        int 21h
        jnc     seek_ret        ; CF=1?
        sbb     ax,ax           ;  then dx:ax = -1, else unchanged
        sbb     dx,dx
seek_ret:
        ret

;; VOID init_PSPSet(seg psp_seg)
    global INIT_PSPSET
INIT_PSPSET:
        pop ax         ; ret address
        pop bx         ; psp_seg
        push ax        ; ret_address
	mov ah, 50h
        int 21h
        ret

;; COUNT init_DosExec(COUNT mode, exec_blk * ep, BYTE * lp)
    global INIT_DOSEXEC
INIT_DOSEXEC:
        pop es                  ; ret address
        pop dx                  ; filename
        pop bx                  ; exec block
        pop ax                  ; mode
        push es                 ; ret address
        mov ah, 4bh
        push ds
        pop es                  ; es = ds
        int 21h
        jc short exec_no_error
        xor ax, ax
exec_no_error:
        ret

;; int init_setdrive(int drive)
   global INIT_SETDRIVE
INIT_SETDRIVE:
	mov ah, 0x0e
common_dl_int21:
        pop bx                  ; ret address
        pop dx                  ; drive/char
        push bx
        int 21h
        ret

;; int init_switchar(int char)
   global INIT_SWITCHAR
INIT_SWITCHAR:
	mov ax, 0x3701
	jmp short common_dl_int21

;
; seg ASMPASCAL allocmem(UWORD size);
;
    global ALLOCMEM
ALLOCMEM:
        pop ax           ; ret address
        pop bx           ; size
        push ax          ; ret address
        mov ah, 48h
        int 21h
        sbb bx, bx       ; carry=1 -> ax=-1
        or  ax, bx       ; segment
        ret

;; void set_DTA(void far *dta)
    global SET_DTA
SET_DTA:
        pop ax           ; ret address
        pop dx           ; off(dta)
        pop bx           ; seg(dta)
        push ax          ; ret address
        mov ah, 1ah
        push ds
        mov ds, bx
        int 21h
        pop ds
        ret
