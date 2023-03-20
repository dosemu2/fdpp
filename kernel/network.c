/****************************************************************/
/*                                                              */
/*                          network.c                           */
/*                            DOS-C                             */
/*                                                              */
/*                 Networking Support functions                 */
/*                                                              */
/*                   Copyright (c) 1995, 1999                   */
/*                         James Tabor                          */
/*                      All Rights Reserved                     */
/*                                                              */
/* This file is part of DOS-C.                                  */
/*                                                              */
/* DOS-C is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU General Public License  */
/* as published by the Free Software Foundation; either version */
/* 2, or (at your option) any later version.                    */
/*                                                              */
/* DOS-C is distributed in the hope that it will be useful, but */
/* WITHOUT ANY WARRANTY; without even the implied warranty of   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See    */
/* the GNU General Public License for more details.             */
/*                                                              */
/* You should have received a copy of the GNU General Public    */
/* License along with DOS-C; see the file COPYING.  If not,     */
/* write to the Free Software Foundation, 675 Mass Ave,         */
/* Cambridge, MA 02139, USA.                                    */
/****************************************************************/

#include "portab.h"
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId =
    "$Id: network.c 895 2004-04-21 17:40:12Z bartoldeman $";
#endif

/* see RBIL D-2152 and D-215D06 before attempting
   to change these two functions!
 */
UWORD get_machine_name(char FAR * netname)
{
  fmemcpy(netname, &net_name, 16);
  return (NetBios);
}

VOID set_machine_name(const char FAR * netname, UWORD name_num)
{
  NetBios = name_num;
  fmemcpy(&net_name, netname, 15);
  net_set_count++;
}

int network_redirector_fp(unsigned cmd, void FAR *s)
{
  return (WORD)network_redirector_mx(cmd, s, 0);
}

int network_redirector(unsigned cmd)
{
  return network_redirector_fp(cmd, NULL);
}

/* Int 2F network redirector functions
 *
 * added by stsp for fdpp project
 */

UDWORD remote_lseek(void FAR *sft, DWORD pos)
{
    iregs regs = {};
    regs.es = FP_SEG(sft);
    regs.di = FP_OFF(sft);
    regs.d.x = pos & 0xffff;
    regs.c.x = pos >> 16;
    regs.a.x = 0x1121;
    call_intr(0x2f, MK_FAR_SCP(regs));
    if (regs.flags & FLG_CARRY)
        return -regs.a.x;
    return ((regs.d.x << 16) | regs.a.x);
}

UWORD remote_getfattr(void)
{
    iregs regs = {};
    regs.a.x = 0x110f;
    call_intr(0x2f, MK_FAR_SCP(regs));
    if (regs.flags & FLG_CARRY)
        return -regs.a.x;
    return regs.a.x;
}

BYTE remote_lock_unlock(void FAR *sft, BYTE unlock,
    struct remote_lock_unlock FAR *arg)
{
    iregs regs = {};
    regs.es = FP_SEG(sft);
    regs.di = FP_OFF(sft);
    regs.b.b.l = unlock;
    regs.c.x = 1;
    regs.ds = FP_SEG(arg);
    regs.d.x = FP_OFF(arg);
    regs.a.x = 0x110a;
    call_intr(0x2f, MK_FAR_SCP(regs));
    if (regs.flags & FLG_CARRY)
        return -regs.a.x;
    return SUCCESS;
}

BYTE remote_qualify_filename(char FAR *dst, const char FAR *src)
{
    iregs regs = {};
    regs.es = FP_SEG(dst);
    regs.di = FP_OFF(dst);
    regs.ds = FP_SEG(src);
    regs.si = FP_OFF(src);
    regs.a.x = 0x1123;
    call_intr(0x2f, MK_FAR_SCP(regs));
    if (regs.flags & FLG_CARRY)
        return -regs.a.x;
    return SUCCESS;
}

BYTE remote_getfree(void FAR *cds, void *dst)
{
    UWORD *udst = (UWORD *)dst;
    iregs regs = {};
    regs.es = FP_SEG(cds);
    regs.di = FP_OFF(cds);
    regs.a.x = 0x110c;
    call_intr(0x2f, MK_FAR_SCP(regs));
    if (regs.flags & FLG_CARRY)
        return -regs.a.x;
    udst[0] = regs.a.x;
    udst[1] = regs.b.x;
    udst[2] = regs.c.x;
    udst[3] = regs.d.x;
    return SUCCESS;
}
