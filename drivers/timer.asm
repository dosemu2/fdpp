;
; File:
;                          timer.asm
; Description:
;             Set a single timer and check when expired
;
;                       Copyright (c) 1995
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
; $Logfile:   C:/dos-c/src/drivers/timer.asv  $
;
; $Header$
;
; $Log$
; Revision 1.5  2001/04/15 03:21:50  bartoldeman
; See history.txt for the list of fixes.
;
; Revision 1.4  2001/03/21 02:56:25  bartoldeman
; See history.txt for changes. Bug fixes and HMA support are the main ones.
;
; Revision 1.3  2000/05/25 20:56:19  jimtabor
; Fixed project history
;
; Revision 1.2  2000/05/11 03:56:20  jimtabor
; Clean up and Release
;
; Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
; The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
; MS-DOS.  Distributed under the GNU GPL.
;
; Revision 1.3  1999/08/10 17:21:08  jprice
; ror4 2011-01 patch
;
; Revision 1.2  1999/03/29 17:08:31  jprice
; ror4 changes
;
; Revision 1.1.1.1  1999/03/29 15:40:34  jprice
; New version without IPL.SYS
;
; Revision 1.2  1999/01/22 04:16:39  jprice
; Formating
;
; Revision 1.1.1.1  1999/01/20 05:51:00  jprice
; Imported sources
;
;
;   Rev 1.2   29 Aug 1996 13:07:12   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 18:50:42   patv
;Initial GPL release.
;
;   Rev 1.0   02 Jul 1995  8:01:04   patv
;Initial revision.
;

                %include "..\kernel\segs.inc"

segment	HMA_TEXT

;
;       void tmark()
;
	global	_tmark
_tmark:
        xor     ah,ah
        int     01aH                    ; get current time in ticks
        xor     ah,ah
        mov     word [LastTime],dx    ; and store it
        mov     word [LastTime+2],cx
        ret


;
;       int tdelay(Ticks)
;
	global	_tdelay
_tdelay:
        push    bp
        mov     bp,sp
        xor     ah,ah
        int     01aH                    ; get current time in ticks
        xor     ah,ah
        mov     word bx,dx          ; and save it to a local variable
                                    ; "Ticks" (cx:bx)
;
; Do a c equivalent of:
;
;               return Now >= (LastTime + Ticks);
;
        mov     ax,word [LastTime+2]
        mov     dx,word [LastTime]
        add     dx,word [bp+4]
        adc     ax,word [bp+6]
        cmp     ax,cx            
        mov     ax,0                    ; mov does not affect flags
        ja      short tdel_1
        jne     short tdel_2
        cmp     dx,bx
        ja      short tdel_1
tdel_2:
        inc     ax                      ; True return
tdel_1:
        pop     bp                      ; False return
        ret


;
;       void twait(Ticks)
;
	global	_twait
_twait:
        push    bp
        mov     bp,sp
        call    _tmark                 ; mark a start
;
;       c equivalent
;               do
;                       GetNowTime(&Now);
;               while((LastTime + Ticks) < Now);
twait_1:
        xor     ah,ah
        int     01aH
        xor     ah,ah                           ; do GetNowTime
        mov     bx,dx                           ; and save it to "Now" (cx:bx)
;
;       do comparison
;
        mov     ax,word [LastTime+2]
        mov     dx,word [LastTime]
        add     dx,word [bp+4]
        adc     ax,word [bp+6]
        cmp     ax,cx
        jb      short twait_1
        jne     short twait_2
        cmp     dx,bx
        jb      short twait_1
twait_2:
        pop     bp
        ret

segment	_BSS
LastTime:	resd	1
