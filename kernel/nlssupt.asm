; File:
;                         nls.asm
; Description:
;     Assembly support routines for nls functions.
;
;                    Copyright (c) 1995, 1998
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
; $Logfile:   C:/dos-c/src/kernel/nlssupt.asv  $
;
; $Id$
;
; $Log$
; Revision 1.6  2001/03/21 02:56:26  bartoldeman
; See history.txt for changes. Bug fixes and HMA support are the main ones.
;
;
; Revision 1.5  2001/03/08 21:15:00  bartoldeman
; Fixed typo in dosUpChar (Tom Ehlert)
;        
; Revision 1.4  2000/08/06 05:50:17  jimtabor
; Add new files and update cvs with patches and changes
;
; Revision 1.3  2000/05/25 20:56:21  jimtabor
; Fixed project history
;
; Revision 1.2  2000/05/08 04:30:00  jimtabor
; Update CVS to 2020
;
; Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
; The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
; MS-DOS.  Distributed under the GNU GPL.
;
; Revision 1.3  2000/03/17 22:59:04  kernel
; Steffen Kaiser's NLS changes
;
; Revision 1.2  1999/08/10 17:57:13  jprice
; ror4 2011-02 patch
;
; Revision 1.1.1.1  1999/03/29 15:41:25  jprice
; New version without IPL.SYS
;
; Revision 1.4  1999/02/08 05:55:57  jprice
; Added Pat's 1937 kernel patches
;
; Revision 1.3  1999/02/01 01:48:41  jprice
; Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
;
; Revision 1.2  1999/01/22 04:13:26  jprice
; Formating
;
; Revision 1.1.1.1  1999/01/20 05:51:01  jprice
; Imported sources
;
;     Rev 1.3   06 Dec 1998  8:46:56   patv
;  Bug fixes.
;
;     Rev 1.2   16 Jan 1997 12:46:44   patv
;  pre-Release 0.92 feature additions
;
;     Rev 1.1   29 May 1996 21:03:38   patv
;  bug fixes for v0.91a
;
;     Rev 1.0   19 Feb 1996  3:24:04   patv
;  Added NLS, int2f and config.sys processing
; $EndLog$
;


		%include "segs.inc"

segment	HMA_TEXT
                global  _reloc_call_CharMapSrvc
                extern  _DosUpChar:wrt HGROUP
;
; CharMapSrvc:
;       User callable character mapping service.
;       Part of Function 38h
;
_reloc_call_CharMapSrvc:
                push    ds
                push    es
                push    bp
                push    si
                push    di
                push    dx
                push    cx
                push    bx

                push    ax          ; arg of _upChar
                push ax
                mov  ax,DGROUP   
                mov     ds, ax
                pop ax

                call    _DosUpChar
                ;add     sp, byte 2	// next POP retrieves orig AX

                pop bx
                mov ah, bh		; keep hibyte untouched

                pop     bx
                pop     cx
                pop     dx
                pop     di
                pop     si
                pop     bp
                pop     es
                pop     ds
                retf                            ; Return far
