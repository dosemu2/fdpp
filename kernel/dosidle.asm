; File:
;                         DosIdle.asm
; Description:
;                   Dos Idle Interrupt Call
;
;                            DOS-C
;                   Copyright (c) 1995, 1999
;                      Pasquale J. Villani
;                      All Rights Reserved
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
;
        %include "segs.inc"

PSP_USERSP      equ     2eh
PSP_USERSS      equ     30h

segment _TEXT

                global  _DosIdle_int


                extern   _InDOS:wrt DGROUP
                extern   _cu_psp:wrt DGROUP
                extern   _MachineId:wrt DGROUP
                extern   critical_sp:wrt DGROUP
                extern   _lpUserStack:wrt DGROUP
                extern   _user_r:wrt DGROUP
                extern   _api_sp:wrt DGROUP      ; api stacks - for context
                extern   _api_ss:wrt DGROUP      ; switching
                extern   _usr_sp:wrt DGROUP      ; user stacks
                extern   _usr_ss:wrt DGROUP
                extern   _dosidle_flag:wrt DGROUP
;
;
_DosIdle_int:
                push    ds
                push    ax
                mov     ax,DGROUP
                mov     ds,ax
                pop     ax
                cmp     byte [_dosidle_flag],0
                jnz     DosId1
                call    Do_DosI
DosId1:
                pop     ds
                retn

Do_DosI:
                inc     byte [_dosidle_flag]
                push    ax
                push    es
                push    word [_MachineId]
                push    word [_user_r]
                push    word [_user_r+2]
                push    word [_lpUserStack]
                push    word [_lpUserStack+2]
                push    word [_api_sp]
                push    word [_api_ss]
                push    word [_usr_sp]
                push    word [_usr_ss]
                mov     es,word [_cu_psp]
                push    word [es:PSP_USERSS]
                push    word [es:PSP_USERSP]

                int     28h

                mov     es,word [_cu_psp]
                pop     word [es:PSP_USERSP]
                pop     word [es:PSP_USERSS]
                pop     word [_usr_ss]
                pop     word [_usr_sp]
                pop     word [_api_ss]
                pop     word [_api_sp]
                pop     word [_lpUserStack+2]
                pop     word [_lpUserStack]
                pop     word [_user_r+2]
                pop     word [_user_r]
                pop     word [_MachineId]
                pop     es
                pop     ax
                dec     byte [_dosidle_flag]
                ret

