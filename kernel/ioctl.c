/****************************************************************/
/*                                                              */
/*                          ioctl.c                             */
/*                                                              */
/*                    DOS-C ioctl system call                   */
/*                                                              */
/*                    Copyright (c) 1995,1998                   */
/*                      Pasquale J. Villani                     */
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
    "$Id$";
#endif

/*
 * WARNING:  this code is non-portable (8086 specific).
 */

/*  TE 10/29/01

	although device drivers have only 20 pushes available for them,
	MS NET plays by its own rules

	at least TE's network card driver DM9PCI (some 10$ NE2000 clone) does:
	with SP=8DC before calling down to execrh, and SP=8CC when 
	callf [interrupt], 	DM9PCI touches DOSDS:792, 
	14 bytes into error stack :-(((
	
	so some optimizations were made.		
	this uses the fact, that only CharReq device buffer is ever used.
	fortunately, this saves some code as well :-)

*/

COUNT DosDevIOctl(lregs * r)
{
  sft FAR *s;
  struct dpb FAR *dpbp;
  COUNT nMode;

  /* commonly used, shouldn't harm to do front up */

  CharReqHdr.r_length = sizeof(request);
  CharReqHdr.r_trans = MK_FP(r->DS, r->DX);
  CharReqHdr.r_status = 0;
  CharReqHdr.r_count = r->CX;

  /* Test that the handle is valid                                */
  switch (r->AL)
  {
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x06:
    case 0x07:
    case 0x0a:
    case 0x0c:
    case 0x10:

      /* Get the SFT block that contains the SFT              */
      if ((s = get_sft(r->BX)) == (sft FAR *) - 1)
        return DE_INVLDHNDL;
      
      switch (r->AL)
      {
        unsigned attr = s->sft_dev->dh_attr;
        unsigned flags = s->sft_flags;

        case 0x00:
          /* Get the flags from the SFT                           */
          if (flags & SFT_FDEVICE)
            r->AX = (attr & 0xff00) | (flags & 0xff);
          else
            r->AX = flags;
          /* Undocumented result, Ax = Dx seen using Pcwatch */
          r->DX = r->AX;
          break;

        case 0x01:
          /* sft_flags is a file, return an error because you     */
          /* can't set the status of a file.                      */
          if (!(flags & SFT_FDEVICE))
            return DE_INVLDFUNC;

          /* Set it to what we got in the DL register from the    */
          /* user.                                                */
          r->AL = s->sft_flags_lo = SFT_FDEVICE | r->DL;
          break;

        case 0x02:
          nMode = C_IOCTLIN;
          goto IoCharCommon;
          
        case 0x03:
          nMode = C_IOCTLOUT;
          goto IoCharCommon;
          
        case 0x06:
          if (flags & SFT_FDEVICE)
          {
            nMode = C_ISTAT;
            goto IoCharCommon;
          }
          r->AL = s->sft_posit >= s->sft_size ? 0 : 0xFF;
          break;
          
        case 0x07:
          if (flags & SFT_FDEVICE)
          {
            nMode = C_OSTAT;
            goto IoCharCommon;
          }
          r->AL = 0;
          break;

        case 0x0a:
          r->DX = flags;
          r->AX = 0;
          break;

        case 0x0c:
          nMode = C_GENIOCTL;
          goto IoCharCommon;
          
        case 0x10:
          nMode = C_IOCTLQRY;
        IoCharCommon:
          if ((flags & SFT_FDEVICE) &&
              (  ((r->AL == 0x02 || r->AL == 0x03) && (attr & ATTR_IOCTL))
              ||   r->AL == 0x06 || r->AL == 0x07
              || ((r->AL == 0x10) && (attr & ATTR_QRYIOCTL))
              || ((r->AL == 0x0c) && (attr & ATTR_GENIOCTL))))
          {
            CharReqHdr.r_unit = 0;
            CharReqHdr.r_command = nMode;
            execrh((request FAR *) & CharReqHdr, s->sft_dev);
            
            if (CharReqHdr.r_status & S_ERROR)
            {
              CritErrCode = (CharReqHdr.r_status & S_MASK) + 0x13;
              return DE_DEVICE;
            }
            
            if (r->AL == 0x06 || r->AL == 0x07)
            {
              r->AX = CharReqHdr.r_status & S_BUSY ? 0000 : 0x00ff;
            }
            else if (r->AL == 0x02 || r->AL == 0x03)
            {
              r->AX = CharReqHdr.r_count;
            }
            else if (r->AL == 0x0c || r->AL == 0x10)
            {
              r->AX = CharReqHdr.r_status;
            }
            break;
          }
          /* fall through */
        default:  
          return DE_INVLDFUNC;
      }
      break;

    case 0x04:
    case 0x05:
    case 0x08:
    case 0x09:
    case 0x0d:
    case 0x0e:
    case 0x0f:
    case 0x11:

/*
   This line previously returned the deviceheader at r->bl. But,
   DOS numbers its drives starting at 1, not 0. A=1, B=2, and so
   on. Changed this line so it is now zero-based.

   -SRM
 */
/* JPP - changed to use default drive if drive=0 */
/* JT Fixed it */

#define NDN_HACK
/* NDN feeds the actual ASCII drive letter to this function */
#ifdef NDN_HACK
      CharReqHdr.r_unit = ((r->BL & 0x1f) == 0 ? default_drive :
                           (r->BL & 0x1f) - 1);
#else
      CharReqHdr.r_unit = (r->BL == 0 ? default_drive : r->BL - 1);
#endif

      dpbp = get_dpb(CharReqHdr.r_unit);

      switch (r->AL)
      {
        case 0x04:
          nMode = C_IOCTLIN;
          goto IoBlockCommon;
        case 0x05:
          nMode = C_IOCTLOUT;
          goto IoBlockCommon;
        case 0x08:
          if (!dpbp)
          {
            return DE_INVLDDRV;
          }
          if (dpbp->dpb_device->dh_attr & ATTR_EXCALLS)
          {
            nMode = C_REMMEDIA;
            goto IoBlockCommon;
          }
          return DE_INVLDFUNC;
        case 0x09:
        {
          struct cds FAR *cdsp = get_cds(CharReqHdr.r_unit);
          r->AX = S_DONE | S_BUSY;
          if (cdsp != NULL && dpbp == NULL)
          {
            r->DX = ATTR_REMOTE;
          }
          else
          {
            if (!dpbp)
            {
              return DE_INVLDDRV;
            }
            r->DX = dpbp->dpb_device->dh_attr;
          }
          if (cdsp->cdsFlags & CDSSUBST)
          {
            r->DX |= ATTR_SUBST;
          }
          break;
        }
        case 0x0d:
          nMode = C_GENIOCTL;
          goto IoBlockCommon;
        case 0x11:
          nMode = C_IOCTLQRY;
        IoBlockCommon:
          if (!dpbp)
          {
            return DE_INVLDDRV;
          }
          if (((r->AL == 0x04) && !(dpbp->dpb_device->dh_attr & ATTR_IOCTL))
              || ((r->AL == 0x05) && !(dpbp->dpb_device->dh_attr & ATTR_IOCTL))
              || ((r->AL == 0x11)
                  && !(dpbp->dpb_device->dh_attr & ATTR_QRYIOCTL))
              || ((r->AL == 0x0d)
                  && !(dpbp->dpb_device->dh_attr & ATTR_GENIOCTL)))
          {
            return DE_INVLDFUNC;
          }
          if (r->AL == 0x0D && (r->CX & ~(0x486B-0x084A)) == 0x084A)
          {             /* 084A/484A, 084B/484B, 086A/486A, 086B/486B */
            r->AX = 0;  /* (lock/unlock logical/physical volume) */
            break;      /* simulate success for MS-DOS 7+ SCANDISK etc. --LG */
          }

          CharReqHdr.r_command = nMode;
          execrh((request FAR *) & CharReqHdr, dpbp->dpb_device);

          if (CharReqHdr.r_status & S_ERROR)
          {
            CritErrCode = (CharReqHdr.r_status & S_MASK) + 0x13;
            return DE_DEVICE;
          }
          if (r->AL == 0x08)
          {
            r->AX = (CharReqHdr.r_status & S_BUSY) ? 1 : 0;
          }

          else if (r->AL == 0x04 || r->AL == 0x05)
          {
            r->AX = CharReqHdr.r_count;
          }
          else if (r->AL == 0x0d || r->AL == 0x11)
          {
            r->AX = CharReqHdr.r_status;
          }
          break;

        case 0x0e:
          nMode = C_GETLDEV;
          goto IoLogCommon;
        case 0x0f:
          nMode = C_SETLDEV;
        IoLogCommon:
          if (!dpbp)
          {
            return DE_INVLDDRV;
          }
          if ((dpbp->dpb_device->dh_attr & ATTR_GENIOCTL))
          {
            
            CharReqHdr.r_command = nMode;
            execrh((request FAR *) & CharReqHdr, dpbp->dpb_device);
            
            if (CharReqHdr.r_status & S_ERROR)
            {
              CritErrCode = (CharReqHdr.r_status & S_MASK) + 0x13;
              return DE_ACCESS;
            }
            else
            {
              r->AL = CharReqHdr.r_unit;
              return SUCCESS;
            }
          } /* fall through */
        default:  
          return DE_INVLDFUNC;
      }
      break;
      
    case 0x0b:
      /* skip, it's a special case.                           */
      NetDelay = r->CX;
      if (!r->DX)
        NetRetry = r->DX;
      break;
      
    default:
      return DE_INVLDFUNC;
  }
  return SUCCESS;
}

