/****************************************************************/
/*                                                              */
/*                          inthndlr.c                          */
/*                                                              */
/*    Interrupt Handler and Function dispatcher for Kernel      */
/*                                                              */
/*                      Copyright (c) 1995                      */
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
/*                                                              */
/****************************************************************/
#define MAIN

#include "portab.h"
#include "globals.h"
#include "nls.h"

#ifdef VERSION_STRINGS
BYTE *RcsId =
    "$Id$";
#endif

#ifdef TSC
STATIC VOID StartTrace(VOID);
static bTraceNext = FALSE;
#endif

#if 0                           /* Very suspicious, passing structure by value??
                                   Deactivated -- 2000/06/16 ska */
/* Special entry for far call into the kernel                           */
#pragma argsused
VOID FAR int21_entry(iregs UserRegs)
{
  int21_handler(UserRegs);
}
#endif

/* Structures needed for int 25 / int 26 */
struct HugeSectorBlock {
  ULONG blkno;
  WORD nblks;
  BYTE FAR *buf;
};

/* Normal entry.  This minimizes user stack usage by avoiding local     */
/* variables needed for the rest of the handler.                        */
/* this here works on the users stack !! and only very few functions 
   are allowed                                                          */
VOID ASMCFUNC int21_syscall(iregs FAR * irp)
{
  Int21AX = irp->AX;

  switch (irp->AH)
  {
      /* DosVars - get/set dos variables                              */
    case 0x33:
      switch (irp->AL)
      {
          /* Get Ctrl-C flag                                      */
        case 0x00:
          irp->DL = break_ena;
          break;

          /* Set Ctrl-C flag                                      */
        case 0x01:
          break_ena = irp->DL != 0;
          break;

        case 0x02:             /* andrew schulman: get/set extended control break  */
          {
            UBYTE tmp = break_ena;
            irp->DL = break_ena;
            break_ena = tmp != 0;
          }

          /* Get Boot Drive                                       */
        case 0x05:
          irp->DL = BootDrive;
          break;

          /* Get DOS-C version                                    */
        case 0x06:
          irp->BL = os_major;
          irp->BH = os_minor;
          irp->DL = rev_number;
          irp->DH = version_flags;      /* bit3:runs in ROM,bit 4: runs in HMA */
          break;

        case 0x03:             /* DOS 7 does not set AL */
        case 0x07:             /* neither here */

        default:               /* set AL=0xFF as error, NOT carry */
          irp->AL = 0xff;
          break;

          /* Toggle DOS-C rdwrblock trace dump                    */
        case 0xfd:
#ifdef DEBUG
          bDumpRdWrParms = !bDumpRdWrParms;
#endif
          break;

          /* Toggle DOS-C syscall trace dump                      */
        case 0xfe:
#ifdef DEBUG
          bDumpRegs = !bDumpRegs;
#endif
          break;

          /* Get DOS-C release string pointer                     */
        case 0xff:
          irp->DX = FP_SEG(os_release);
          irp->AX = FP_OFF(os_release);
          break;
      }
      break;

      /* Set PSP                                                      */
    case 0x50:
      cu_psp = irp->BX;
      break;

      /* Get PSP                                                      */
    case 0x51:
      irp->BX = cu_psp;
      break;

      /* UNDOCUMENTED: return current psp                             */
    case 0x62:
      irp->BX = cu_psp;
      break;

      /* Normal DOS function - DO NOT ARRIVE HERE          */
    default:
      break;
  }
}

#ifdef WITHFAT32
      /* DOS 7.0+ FAT32 extended functions */
int int21_fat32(lregs *r)
{
  COUNT rc;
  
  switch (r->AL)
  {
    /* Get extended drive parameter block */
    case 0x02:
    {
      struct dpb FAR *dpb;
      struct xdpbdata FAR *xddp;
    
      if (r->CX < sizeof(struct xdpbdata))
        return DE_INVLDBUF;

      dpb = GetDriveDPB(r->DL, &rc);
      if (rc != SUCCESS)
        return rc;

      /* hazard: no error checking! */
      flush_buffers(dpb->dpb_unit);
      dpb->dpb_flags = M_CHANGED;       /* force reread of drive BPB/DPB */
    
      if (media_check(dpb) < 0)
        return DE_INVLDDRV;
    
      xddp = MK_FP(r->ES, r->DI);
      
      fmemcpy(&xddp->xdd_dpb, dpb, sizeof(struct dpb));
      xddp->xdd_dpbsize = sizeof(struct dpb);
      break;
    }
    /* Get extended free drive space */
    case 0x03:
    {
      struct xfreespace FAR *xfsp = MK_FP(r->ES, r->DI);
    
      if (r->CX < sizeof(struct xfreespace))
        return DE_INVLDBUF;

      rc = DosGetExtFree(MK_FP(r->DS, r->DX), xfsp);
      if (rc != SUCCESS)
        return rc;
      xfsp->xfs_datasize = sizeof(struct xfreespace);
      xfsp->xfs_version.actual = 0;
      break;
    }
    /* Set DPB to use for formatting */
    case 0x04:
    {
      struct xdpbforformat FAR *xdffp = MK_FP(r->ES, r->DI);
      struct dpb FAR *dpb;
      if (r->CX < sizeof(struct xdpbforformat))
      {
        return DE_INVLDBUF;
      }
      dpb = GetDriveDPB(r->DL, &rc);
      if (rc != SUCCESS)
        return rc;
      
      xdffp->xdff_datasize = sizeof(struct xdpbforformat);
      xdffp->xdff_version.actual = 0;
    
      switch ((UWORD) xdffp->xdff_function)
      {
        case 0x00:
        {
          DWORD nfreeclst = xdffp->xdff_f.setdpbcounts.nfreeclst;
          DWORD cluster = xdffp->xdff_f.setdpbcounts.cluster;
          if (ISFAT32(dpb))
          {
            if ((dpb->dpb_xfsinfosec == 0xffff
                 && (nfreeclst != 0 || cluster != 0))
                || nfreeclst == 1 || nfreeclst > dpb->dpb_xsize
                || cluster == 1 || cluster > dpb->dpb_xsize)
            {
              return DE_INVLDPARM;
            }
            dpb->dpb_xnfreeclst = nfreeclst;
            dpb->dpb_xcluster = cluster;
            write_fsinfo(dpb);
          }
          else
          {
            if (nfreeclst == 1 || nfreeclst > dpb->dpb_size ||
                cluster == 1 || cluster > dpb->dpb_size)
            {
              return DE_INVLDPARM;
            }
            dpb->dpb_nfreeclst = nfreeclst;
            dpb->dpb_cluster = cluster;
          }
          break;
        }
        case 0x01:
        {
          ddt *pddt = getddt(r->DL);
          fmemcpy(&pddt->ddt_bpb, xdffp->xdff_f.rebuilddpb.bpbp,
                  sizeof(bpb));
        }
        case 0x02:
        {
        rebuild_dpb:
            /* hazard: no error checking! */
          flush_buffers(dpb->dpb_unit);
          dpb->dpb_flags = M_CHANGED;
          
          if (media_check(dpb) < 0)
            return DE_INVLDDRV;
          break;
        }
        case 0x03:
        {
          struct buffer FAR *bp;
          bpb FAR *bpbp;
          DWORD newmirroring =
            xdffp->xdff_f.setmirroring.newmirroring;
      
          if (newmirroring != -1
              && (ISFAT32(dpb)
                  && (newmirroring & ~(0xf | 0x80))))
          {
            return DE_INVLDPARM;
          }
          xdffp->xdff_f.setmirroring.oldmirroring =
            (ISFAT32(dpb) ? dpb->dpb_xflags : 0);
          if (newmirroring != -1 && ISFAT32(dpb))
          {
            bp = getblock(1, dpb->dpb_unit);
            bp->b_flag &= ~(BFR_DATA | BFR_DIR | BFR_FAT);
            bp->b_flag |= BFR_VALID | BFR_DIRTY;
            bpbp = (bpb FAR *) & bp->b_buffer[BT_BPB];
            bpbp->bpb_xflags = newmirroring;
          }
          goto rebuild_dpb;
        }
        case 0x04:
        {
          struct buffer FAR *bp;
          bpb FAR *bpbp;
          DWORD rootclst = xdffp->xdff_f.setroot.newrootclst;
          if (!ISFAT32(dpb)
              || (rootclst != -1
                  && (rootclst == 1
                      || rootclst > dpb->dpb_xsize)))
          {
            return DE_INVLDPARM;
          }
          xdffp->xdff_f.setroot.oldrootclst = dpb->dpb_xrootclst;
          if (rootclst != -1)
          {
            bp = getblock(1, dpb->dpb_unit);
            bp->b_flag &= ~(BFR_DATA | BFR_DIR | BFR_FAT);
            bp->b_flag |= BFR_VALID | BFR_DIRTY;
            bpbp = (bpb FAR *) & bp->b_buffer[BT_BPB];
            bpbp->bpb_xrootclst = rootclst;
          }
          goto rebuild_dpb;
        }
      }
    
      break;
    }
    /* Extended absolute disk read/write */
    /* TODO(vlp) consider using of the 13-14th bits of SI */
    case 0x05:
    {
      struct HugeSectorBlock FAR *SectorBlock =
        (struct HugeSectorBlock FAR *)MK_FP(r->DS, r->BX);
      UBYTE mode;
      
      if (r->CX != 0xffff || ((r->SI & 1) == 0 && r->SI != 0)
          || (r->SI & ~0x6001))
      {
        return DE_INVLDPARM;
      }
    
      if (r->DL - 1 >= lastdrive || r->DL == 0)
        return -0x207;
    
      if (r->SI == 0)
        mode = DSKREADINT25;
      else
        mode = DSKWRITEINT26;
    
      r->AX =
        dskxfer(r->DL - 1, SectorBlock->blkno, SectorBlock->buf,
                SectorBlock->nblks, mode);
    
      if (mode == DSKWRITEINT26)
        if (r->AX <= 0)
          setinvld(r->DL - 1);
      
      if (r->AX > 0)
        return -0x20c;
      break;
    }
  default:
    return DE_INVLDFUNC;
  }
  return SUCCESS;
}
#endif

VOID ASMCFUNC int21_service(iregs FAR * r)
{
  COUNT rc = 0;
  lregs lr; /* 8 local registers (ax, bx, cx, dx, si, di, ds, es) */

#define FP_DS_DX (MK_FP(lr.DS, lr.DX))
#define FP_ES_DI (MK_FP(lr.ES, lr.DI))
                                                   

#define CLEAR_CARRY_FLAG()  r->FLAGS &= ~FLG_CARRY
#define SET_CARRY_FLAG()    r->FLAGS |= FLG_CARRY

  ((psp FAR *) MK_FP(cu_psp, 0))->ps_stack = (BYTE FAR *) r;

	lr.AX = r->AX;
	lr.BX = r->BX;
	lr.CX = r->CX;
	lr.DX = r->DX;
	lr.SI = r->SI;
	lr.DI = r->DI;
	lr.DS = r->DS;
	lr.ES = r->ES;

dispatch:

#ifdef DEBUG
  if (bDumpRegs)
  {
    fmemcpy(&error_regs, user_r, sizeof(iregs));
    printf("System call (21h): %02x\n", user_r->AX);
    dump_regs = TRUE;
    dump();
  }
#endif

  if ((lr.AH >= 0x38 && lr.AH <= 0x4F) || (lr.AH >= 0x56 && lr.AH <= 0x5c))
    CLEAR_CARRY_FLAG();
  /* Clear carry by default for these functions */


  /* Check for Ctrl-Break */
  switch (lr.AH)
  {
    default:
      if (!break_ena)
        break;
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x08:
    case 0x09:
    case 0x0a:
    case 0x0b:
      if (control_break())
        handle_break();
  }

  /* The dispatch handler                                         */
  switch (lr.AH)
  {
      /* int 21h common error handler                                 */
    case 0x64:
    	goto error_invalid;

      /* case 0x00:   --> Simulate a DOS-4C-00 */

      /* Read Keyboard with Echo                      */
    case 0x01:
      lr.AL = _sti(TRUE);
      sto(lr.AL);
      break;

      /* Display Character                                            */
    case 0x02:
      sto(lr.DL);
      break;

      /* Auxiliary Input                                                      */
    case 0x03:
      BinaryRead(STDAUX, &lr.AL, &UnusedRetVal);
      break;

      /* Auxiliary Output                                                     */
    case 0x04:
      DosWrite(STDAUX, 1, (BYTE FAR *) & lr.DL, & UnusedRetVal);
      break;
      /* Print Character                                                      */
    case 0x05:
      DosWrite(STDPRN, 1, (BYTE FAR *) & lr.DL, & UnusedRetVal);
      break;

      /* Direct Console I/O                                            */
    case 0x06:
      if (lr.DL != 0xff)
        sto(lr.DL);
      else if (StdinBusy())
      {
        lr.AL = 0x00;
        r->FLAGS |= FLG_ZERO;
      }
      else
      {
        r->FLAGS &= ~FLG_ZERO;
        lr.AL = _sti(FALSE);
      }
      break;

      /* Direct Console Input                                         */
    case 0x07:
      lr.AL = _sti(FALSE);
      break;

      /* Read Keyboard Without Echo                                   */
    case 0x08:
      lr.AL = _sti(TRUE);
      break;

      /* Display String                                               */
    case 0x09:
      {         
        unsigned count;
        
        for (count = 0; ; count++)
        	if (((UBYTE FAR *)FP_DS_DX)[count] == '$')
        		break;

        DosWrite(STDOUT, count, FP_DS_DX,
                 & UnusedRetVal);
      }
      lr.AL = '$';
      break;

      /* Buffered Keyboard Input                                      */
    case 0x0a:
      sti_0a((keyboard FAR *) FP_DS_DX);
      break;

      /* Check Stdin Status                                           */
    case 0x0b:
      if (StdinBusy())
        lr.AL = 0x00;
      else
        lr.AL = 0xFF;
      break;

      /* Flush Buffer, Read Keayboard                                 */
    case 0x0c:
      KbdFlush();
      switch (lr.AL)
      {
        case 0x01:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x0a:
          lr.AH = lr.AL;
          goto dispatch;

        default:
          lr.AL = 0x00;
          break;
      }
      break;

      /* Reset Drive                                                  */
    case 0x0d:
      flush();
      break;

      /* Set Default Drive                                            */
    case 0x0e:
      lr.AL = DosSelectDrv(lr.DL);
      break;

    case 0x0f:
      lr.AL = FcbOpen(FP_DS_DX, O_FCB | O_LEGACY | O_OPEN | O_RDWR);
      break;

    case 0x10:
      lr.AL = FcbClose(FP_DS_DX);
      break;

    case 0x11:
      lr.AL = FcbFindFirstNext(FP_DS_DX, TRUE);
      break;

    case 0x12:
      lr.AL = FcbFindFirstNext(FP_DS_DX, FALSE);
      break;

    case 0x13:
      lr.AL = FcbDelete(FP_DS_DX);
      break;

    case 0x14:
      /* FCB read */
      lr.AL = FcbReadWrite(FP_DS_DX, 0, XFR_READ);
      break;

    case 0x15:
      /* FCB write */
      lr.AL = FcbReadWrite(FP_DS_DX, 0, XFR_WRITE);
      break;

    case 0x16:
      lr.AL = FcbOpen(FP_DS_DX, O_FCB | O_LEGACY | O_CREAT | O_TRUNC | O_RDWR);
      break;

    case 0x17:
      lr.AL = FcbRename(FP_DS_DX);
      break;

    default:
#ifdef DEBUG
      printf("Unsupported INT21 AH = 0x%x, AL = 0x%x.\n", lr.AH, lr.AL);
#endif
      /* Fall through. */

      /* CP/M compatibility functions                                 */
    case 0x18:
    case 0x1d:
    case 0x1e:
    case 0x20:
#ifndef TSC
    case 0x61:
#endif
    case 0x6b:
      lr.AL = 0;
      break;

      /* Get Default Drive                                            */
    case 0x19:
      lr.AL = default_drive;
      break;

      /* Set DTA                                                      */
    case 0x1a:
      {
        psp FAR *p = MK_FP(cu_psp, 0);

        p->ps_dta = FP_DS_DX;
        dos_setdta(p->ps_dta);
      }
      break;

      /* Get Default Drive Data                                       */
    case 0x1b:
      lr.DL = 0;
      /* fall through */

      /* Get Drive Data                                               */
    case 0x1c:
      {
        BYTE FAR *p;

        p = FatGetDrvData(lr.DL, &lr.AX, &lr.CX, &lr.DX);
        lr.DS = FP_SEG(p);
        lr.BX = FP_OFF(p);
        lr.CX = lr.CX;
        lr.DX = lr.DX;
        lr.AX = lr.AX;
      }
      break;

      /* Get default DPB                                              */
      /* case 0x1f: see case 0x32 */

      /* Random read using FCB */
    case 0x21:
      lr.AL = FcbRandomIO(FP_DS_DX, XFR_READ);
      break;

      /* Random write using FCB */
    case 0x22:
      lr.AL = FcbRandomIO(FP_DS_DX, XFR_WRITE);
      break;

      /* Get file size in records using FCB */
    case 0x23:
      lr.AL = FcbGetFileSize(FP_DS_DX);
      break;

      /* Set random record field in FCB */
    case 0x24:
      FcbSetRandom(FP_DS_DX);
      break;

      /* Set Interrupt Vector                                         */
    case 0x25:
      setvec(lr.AL, FP_DS_DX);
      break;

      /* Dos Create New Psp                                           */
    case 0x26:
      {
        psp FAR *p = MK_FP(cu_psp, 0);

        new_psp((psp FAR *) MK_FP(lr.DX, 0), p->ps_size);
      }
      break;

      /* Read random record(s) using FCB */
    case 0x27:
      lr.AL = FcbRandomBlockIO(FP_DS_DX, lr.CX, XFR_READ);
      break;

      /* Write random record(s) using FCB */
    case 0x28:
      lr.AL = FcbRandomBlockIO(FP_DS_DX, lr.CX, XFR_WRITE);
      break;

      /* Parse File Name                                              */
    case 0x29:
      {
        UWORD offset = FcbParseFname(&rc, MK_FP(lr.DS, lr.SI), FP_ES_DI);
        lr.AL = rc;
        lr.SI += offset;
      }
      break;

      /* Get Date                                                     */
    case 0x2a:
      DosGetDate(&lr.AL,        /* WeekDay              */
                 &lr.DH,        /* Month                */
                 &lr.DL,        /* MonthDay             */
                 &lr.CX);       /* Year                 */
      break;

      /* Set Date                                                     */
    case 0x2b:
      rc = DosSetDate(lr.DH,    /* Month                */
                      lr.DL,    /* MonthDay             */
                      lr.CX);   /* Year                 */
      lr.AL = (rc != SUCCESS ? 0xff : 0);
      break;

      /* Get Time                                                     */
    case 0x2c:
      DosGetTime(&lr.CH,        /* Hour                 */
                 &lr.CL,        /* Minutes              */
                 &lr.DH,        /* Seconds              */
                 &lr.DL);       /* Hundredths           */
      break;

      /* Set Date                                                     */
    case 0x2d:
      rc = DosSetTime(lr.CH,    /* Hour                 */
                      lr.CL,    /* Minutes              */
                      lr.DH,    /* Seconds              */
                      lr.DL);   /* Hundredths           */
      lr.AL = (rc != SUCCESS ? 0xff : 0);
      break;

      /* Set verify flag                                              */
    case 0x2e:
      verify_ena = (lr.AL ? TRUE : FALSE);
      break;

      /* Get DTA                                                      */
    case 0x2f:
      lr.ES = FP_SEG(dta);
      lr.BX = FP_OFF(dta);
      break;

      /* Get DOS Version                                              */
    case 0x30:
      lr.AL = os_major;
      lr.AH = os_minor;
      lr.BH = OEM_ID;
      lr.CH = REVISION_MAJOR;   /* JPP */
      lr.CL = REVISION_MINOR;
      lr.BL = REVISION_SEQ;

      if (ReturnAnyDosVersionExpected)
      {
        /* TE for testing purpose only and NOT 
           to be documented:
           return programs, who ask for version == XX.YY
           exactly this XX.YY. 
           this makes most MS programs more happy.
         */
        UBYTE FAR *retp = MK_FP(r->cs, r->ip);

        if (retp[0] == 0x3d &&  /* cmp ax, xxyy */
            (retp[3] == 0x75 || retp[3] == 0x74))       /* je/jne error    */
        {
          lr.AL = retp[1];
          lr.AH = retp[2];
        }
        else if (retp[0] == 0x86 &&     /* xchg al,ah   */
                 retp[1] == 0xc4 && retp[2] == 0x3d &&  /* cmp ax, xxyy */
                 (retp[5] == 0x75 || retp[5] == 0x74))  /* je/jne error    */
        {
          lr.AL = retp[4];
          lr.AH = retp[3];
        }

      }

      break;

      /* Keep Program (Terminate and stay resident) */
    case 0x31:
      DosMemChange(cu_psp, lr.DX < 6 ? 6 : lr.DX, 0);
      return_mode = 3;
      return_code = lr.AL;
      tsr = TRUE;
      return_user();
      break;

      /* Get default BPB */
    case 0x1f:
      /* Get DPB                                                      */
    case 0x32:
      /* r->DL is NOT changed by MS 6.22 */
      /* INT21/32 is documented to reread the DPB */
      {
        struct dpb FAR *dpb;
        UCOUNT drv = lr.DL;

        if (drv == 0 || lr.AH == 0x1f)
          drv = default_drive;
        else
          drv--;

        if (drv >= lastdrive)
        {
          lr.AL = 0xFF;
          CritErrCode = 0x0f;
          break;
        }

        dpb = CDSp[drv].cdsDpb;
        if (dpb == 0 || CDSp[drv].cdsFlags & CDSNETWDRV)
        {
          lr.AL = 0xFF;
          CritErrCode = 0x0f;
          break;
        }
        /* hazard: no error checking! */        
        flush_buffers(dpb->dpb_unit);
        dpb->dpb_flags = M_CHANGED;     /* force flush and reread of drive BPB/DPB */

#ifdef WITHFAT32
        if (media_check(dpb) < 0 || ISFAT32(dpb))
#else
        if (media_check(dpb) < 0)
#endif
        {
          lr.AL = 0xff;
          CritErrCode = 0x0f;
          break;
        }
        lr.DS = FP_SEG(dpb);
        lr.BX = FP_OFF(dpb);
        lr.AL = 0;
      }

      break;
/*
    case 0x33:  
    see int21_syscall
*/
      /* Get InDOS flag                                               */
    case 0x34:
      lr.ES = FP_SEG(&InDOS);
      lr.BX = FP_OFF(&InDOS);
      break;

      /* Get Interrupt Vector                                         */
  case 0x35:
     {
        intvec p = getvec((COUNT) lr.AL);
        lr.ES = FP_SEG(p);
        lr.BX = FP_OFF(p);
      }
      break;

      /* Dos Get Disk Free Space                                      */
    case 0x36:
      DosGetFree(lr.DL, &lr.AX, &lr.BX, &lr.CX, &lr.DX);
      break;

      /* Undocumented Get/Set Switchar                                */
    case 0x37:
      switch (lr.AL)
      {
          /* Get switch character */
        case 0x00:
          lr.DL = switchar;
          lr.AL = 0x00;
          break;

          /* Set switch character */
        case 0x01:
          switchar = lr.DL;
          lr.AL = 0x00;
          break;

        default:
          goto error_invalid;
      }
      break;

      /* Get/Set Country Info                                         */
    case 0x38:
      {
        UWORD cntry = lr.AL;

        if (cntry == 0)
          cntry = (UWORD) - 1;
        else if (cntry == 0xff)
          cntry = lr.BX;

        if (0xffff == lr.DX)
        {
          /* Set Country Code */
          if ((rc = DosSetCountry(cntry)) < 0)
            goto error_invalid;
        }
        else
        {
          /* Get Country Information */
          if ((rc = DosGetCountryInformation(cntry, FP_DS_DX)) < 0)
            goto error_invalid;
          /* HACK FIXME */
          if (cntry == (UWORD) - 1)
            cntry = 1;
          /* END OF HACK */
          lr.AX = lr.BX = cntry;
        }
      }
      break;

      /* Dos Create Directory                                         */
    case 0x39:
      rc = DosMkdir((BYTE FAR *) FP_DS_DX);
      if (rc != SUCCESS)
        goto error_exit;
      break;

      /* Dos Remove Directory                                         */
    case 0x3a:
      rc = DosRmdir((BYTE FAR *) FP_DS_DX);
      if (rc != SUCCESS)
        goto error_exit;
      break;

      /* Dos Change Directory                                         */
    case 0x3b:
      if ((rc = DosChangeDir((BYTE FAR *) FP_DS_DX)) < 0)
        goto error_exit;
      break;

      /* Dos Create File                                              */
    case 0x3c:
      {
        long lrc = DosOpen(FP_DS_DX, O_LEGACY | O_RDWR | O_CREAT | O_TRUNC, lr.CL);
        
        if (lrc < 0)
        {
          rc = (COUNT)lrc;
          goto error_exit;
        }
        lr.AX = (UWORD)lrc;
        break;
      }

      /* Dos Open                                                     */
    case 0x3d:
      {
        long lrc = DosOpen(FP_DS_DX, O_LEGACY | O_OPEN | lr.AL, 0);

        if (lrc < 0)
        {
          rc = (COUNT)lrc;
          goto error_exit;
        }
        lr.AX = (UWORD)lrc;
        break;
      }

      /* Dos Close                                                    */
    case 0x3e:
      if ((rc = DosClose(lr.BX)) < 0)
        goto error_exit;
      break;

      /* Dos Read                                                     */
    case 0x3f:
      lr.AX = DosRead(lr.BX, lr.CX, FP_DS_DX, & rc);
      if (rc != SUCCESS)
        goto error_exit;
      break;

      /* Dos Write                                                    */
    case 0x40:
      lr.AX = DosWrite(lr.BX, lr.CX, FP_DS_DX, & rc);
      if (rc != SUCCESS)
        goto error_exit;
      break;

      /* Dos Delete File                                              */
    case 0x41:
      rc = DosDelete((BYTE FAR *) FP_DS_DX, D_ALL);
      if (rc < 0)
        goto error_exit;
      break;

      /* Dos Seek                                                     */
    case 0x42:
      {
        ULONG lrc;
        if (lr.AL > 2)
          goto error_invalid;
        lrc = DosSeek(lr.BX, (LONG)((((ULONG) (lr.CX)) << 16) | lr.DX), lr.AL);
        if (lrc == (ULONG)-1)
        {
          rc = -DE_INVLDHNDL;
          goto error_exit;
        }
        else
        {
          lr.DX = (lrc >> 16);
          lr.AX = (UWORD) lrc;
        }
      }
      break;

      /* Get/Set File Attributes                                      */
    case 0x43:
      switch (lr.AL)
      {
        case 0x00:
          rc = DosGetFattr((BYTE FAR *) FP_DS_DX);
          if (rc >= SUCCESS)
            lr.CX = rc;
          break;

        case 0x01:
          rc = DosSetFattr((BYTE FAR *) FP_DS_DX, lr.CX);
          break;

        default:
          goto error_invalid;
      }
      if (rc < SUCCESS)
        goto error_exit;
      break;

      /* Device I/O Control                                           */
    case 0x44:
      rc = DosDevIOctl(&lr);      /* can set critical error code! */

      if (rc != SUCCESS)
      {
        lr.AX = -rc;
        if (rc != DE_DEVICE && rc != DE_ACCESS)
          CritErrCode = -rc;
        goto error_carry;
      }
      break;

      /* Duplicate File Handle                                        */
    case 0x45:
      { 
        long lrc = DosDup(lr.BX);
        if (lrc < SUCCESS)
        {
          rc = (COUNT)lrc;
          goto error_exit;
        }
        lr.AX = (UWORD)lrc;
      }
      break;

      /* Force Duplicate File Handle                                  */
    case 0x46:
      rc = DosForceDup(lr.BX, lr.CX);
      if (rc < SUCCESS)
        goto error_exit;
      break;

      /* Get Current Directory                                        */
    case 0x47:
      if ((rc = DosGetCuDir(lr.DL, MK_FP(lr.DS, lr.SI))) < 0)
        goto error_exit;
      else
        lr.AX = 0x0100;         /*jpp: from interrupt list */
      break;

      /* Allocate memory */
    case 0x48:
      if ((rc =
           DosMemAlloc(lr.BX, mem_access_mode, &(lr.AX), &(lr.BX))) < 0)
      {
        DosMemLargest(&(lr.BX));
        goto error_exit;
      }
      else
        ++(lr.AX);              /* DosMemAlloc() returns seg of MCB rather than data */
      break;

      /* Free memory */
    case 0x49:
      if ((rc = DosMemFree((lr.ES) - 1)) < 0)
        goto error_exit;
      break;

      /* Set memory block size */
    case 0x4a:
      if ((rc = DosMemChange(lr.ES, lr.BX, &lr.BX)) < 0)
      {
#if 0
        if (cu_psp == lr.ES)
        {

          psp FAR *p;
          
          p = MK_FP(cu_psp, 0);
          p->ps_size = lr.BX + cu_psp;
        }
#endif
        goto error_exit;
      }
      break;

      /* Load and Execute Program */
    case 0x4b:
      break_flg = FALSE;

      if ((rc = DosExec(lr.AL, MK_FP(lr.ES, lr.BX), FP_DS_DX)) != SUCCESS)
        goto error_exit;
      break;

      /* Terminate Program                                            */
    case 0x00:
      lr.AX = 0x4c00;

      /* End Program                                                  */
    case 0x4c:
      if (cu_psp == RootPsp
          || ((psp FAR *) (MK_FP(cu_psp, 0)))->ps_parent == cu_psp)
        break;
      tsr = FALSE;
      if (ErrorMode)
      {
        ErrorMode = FALSE;
        return_mode = 2;
      }
      else if (break_flg)
      {
        break_flg = FALSE;
        return_mode = 1;
      }
      else
      {
        return_mode = 0;
      }
      return_code = lr.AL;
      if (DosMemCheck() != SUCCESS)
        panic("MCB chain corrupted");
#ifdef TSC
      StartTrace();
#endif
      return_user();
      break;

      /* Get Child-program Return Value                               */
    case 0x4d:
      lr.AL = return_code;
      lr.AH = return_mode;
      break;

      /* Dos Find First                                               */
    case 0x4e:
      /* dta for this call is set on entry.  This     */
      /* needs to be changed for new versions.        */
      if ((rc = DosFindFirst((UCOUNT) lr.CX, (BYTE FAR *) FP_DS_DX)) < 0)
        goto error_exit;
      lr.AX = 0;
      break;

      /* Dos Find Next                                                */
    case 0x4f:
      /* dta for this call is set on entry.  This     */
      /* needs to be changed for new versions.        */
      if ((rc = DosFindNext()) < 0)
        goto error_exit;
      lr.AX = -SUCCESS;
      break;
/*
    case 0x50:  
    case 0x51:
    see int21_syscall
*/
      /* ************UNDOCUMENTED************************************* */
      /* Get List of Lists                                            */
    case 0x52:
      lr.ES = FP_SEG(&DPBp);
      lr.BX = FP_OFF(&DPBp);
      break;

    case 0x53:
      /*  DOS 2+ internal - TRANSLATE BIOS PARAMETER BLOCK TO DRIVE PARAM BLOCK */
      bpb_to_dpb((bpb FAR *) MK_FP(lr.DS, lr.SI),
                 (struct dpb FAR *)MK_FP(lr.ES, r->BP)
#ifdef WITHFAT32
                 , (lr.CX == 0x4558 && lr.DX == 0x4152)
#endif
          );
      break;

      /* Get verify state                                             */
    case 0x54:
      lr.AL = (verify_ena ? TRUE : FALSE);
      break;

      /* ************UNDOCUMENTED************************************* */
      /* Dos Create New Psp & set p_size                              */
    case 0x55:
      new_psp((psp FAR *) MK_FP(lr.DX, 0), lr.SI);
      cu_psp = lr.DX;
      break;

      /* Dos Rename                                                   */
    case 0x56:
      rc = DosRename((BYTE FAR *) FP_DS_DX,
                     (BYTE FAR *) FP_ES_DI);
      if (rc < SUCCESS)
        goto error_exit;
      break;

      /* Get/Set File Date and Time                                   */
    case 0x57:
      switch (lr.AL)
      {
        case 0x00:
          rc = DosGetFtime((COUNT) lr.BX,       /* Handle               */
                           &lr.DX,        /* FileDate             */
                           &lr.CX);       /* FileTime             */
          if (rc < SUCCESS)
            goto error_exit;
          break;

        case 0x01:
          rc = DosSetFtime((COUNT) lr.BX,       /* Handle               */
                           (date) lr.DX,        /* FileDate             */
                           (time) lr.CX);       /* FileTime             */
          if (rc < SUCCESS)
            goto error_exit;
          break;

        default:
          goto error_invalid;
      }
      break;

      /* Get/Set Allocation Strategy                                  */
    case 0x58:
      switch (lr.AL)
      {
        case 0x00:
          lr.AL = mem_access_mode;
          lr.AH = 0;
          break;

        case 0x01:
          {
            switch (lr.BL)
            {
              case LAST_FIT:
              case LAST_FIT_U:
              case LAST_FIT_UO:
              case BEST_FIT:
              case BEST_FIT_U:
              case BEST_FIT_UO:
              case FIRST_FIT:
              case FIRST_FIT_U:
              case FIRST_FIT_UO:
                mem_access_mode = lr.BL;
                break;

              default:
                goto error_invalid;
            }
          }
          break;

        case 0x02:
          lr.AL = uppermem_link;
          break;

        case 0x03:
	  if (uppermem_root != 0xffff)	  /* always error if not exists */
          {
            DosUmbLink(lr.BL);
            break;
          }
          /* else fall through */

        default:
          goto error_invalid;
#ifdef DEBUG
        case 0xff:
          show_chain();
          break;
#endif
      }
      break;

      /* Get Extended Error */
    case 0x59:
      lr.AX = CritErrCode;
      lr.ES = FP_SEG(CritErrDev);
      lr.DI = FP_OFF(CritErrDev);
      lr.CH = CritErrLocus;
      lr.BH = CritErrClass;
      lr.BL = CritErrAction;
      break;

      /* Create Temporary File */
    case 0x5a:
      if ((rc = DosMkTmp(FP_DS_DX, lr.CX)) < 0)
        goto error_exit;
      lr.AX = rc;
      break;

      /* Create New File */
    case 0x5b:
      {
        long lrc = DosOpen(FP_DS_DX, O_LEGACY | O_RDWR | O_CREAT, lr.CX);
        if (lrc < 0)
        {
          rc = (COUNT)lrc;
          goto error_exit;
        }
        lr.AX = (UWORD)lrc;
        break;
      }

/* /// Added for SHARE.  - Ron Cemer */
      /* Lock/unlock file access */
    case 0x5c:
      if ((rc = DosLockUnlock
           (lr.BX,
            (((unsigned long)lr.CX) << 16) | (((unsigned long)lr.DX) ),
            (((unsigned long)lr.SI) << 16) | (((unsigned long)lr.DI) ),
            ((lr.AX & 0xff) != 0))) != 0)
        goto error_exit;
      break;
/* /// End of additions for SHARE.  - Ron Cemer */

      /* UNDOCUMENTED: server, share.exe and sda function             */
    case 0x5d:
      switch (lr.AL)
      {
          /* Remote Server Call */
        case 0x00:
          fmemcpy(&lr, FP_DS_DX, sizeof(lr));
          goto dispatch;

        case 0x06:
          lr.DS = FP_SEG(internal_data);
          lr.SI = FP_OFF(internal_data);
          lr.CX = swap_indos - internal_data;
          lr.DX = swap_always - internal_data;
          CLEAR_CARRY_FLAG();
          break;

        case 0x07:
        case 0x08:
        case 0x09:
          rc = remote_printredir(lr.DX, Int21AX);
          if (rc != SUCCESS)
            goto error_exit;
          CLEAR_CARRY_FLAG();
          break;
        default:
          goto error_invalid;
      }
      break;

    case 0x5e:
      CLEAR_CARRY_FLAG();
      switch (lr.AL)
      {
        case 0x00:
          lr.CX = get_machine_name(FP_DS_DX);
          break;

        case 0x01:
          set_machine_name(FP_DS_DX, lr.CX);
          break;

        default:
          rc = remote_printset(lr.BX, lr.CX, lr.DX, (FP_ES_DI),
                               lr.SI, (MK_FP(lr.DS, Int21AX)));
          if (rc != SUCCESS)
            goto error_exit;
          lr.AX = SUCCESS;
          break;
      }
      break;

    case 0x5f:
      CLEAR_CARRY_FLAG();
      switch (lr.AL)
      {
        case 0x07:
          if (lr.DL < lastdrive)
          {
            CDSp[lr.DL].cdsFlags |= 0x100;
          }
          break;

        case 0x08:
          if (lr.DL < lastdrive)
          {
            CDSp[lr.DL].cdsFlags &= ~0x100;
          }
          break;

        default:
/*              
            void int_2f_111e_call(iregs FAR *r);
            int_2f_111e_call(r);          
          break;*/

          rc = remote_doredirect(lr.BX, lr.CX, lr.DX,
                                 (FP_ES_DI), lr.SI,
                                 (MK_FP(lr.DS, Int21AX)));
          /* the remote function manipulates *r directly !,
             so we should not copy lr to r here            */
          if (rc != SUCCESS)
          {
            CritErrCode = -rc;      /* Maybe set */
            SET_CARRY_FLAG();
          }
          r->AX = -rc;
          goto real_exit;
      }
      break;

    case 0x60:                 /* TRUENAME */
      CLEAR_CARRY_FLAG();
      if ((rc = DosTruename(MK_FP(lr.DS, lr.SI), adjust_far(FP_ES_DI))) < SUCCESS)
        goto error_exit;
      break;

#ifdef TSC
      /* UNDOCUMENTED: no-op                                          */
      /*                                                              */
      /* DOS-C: tsc support                                           */
    case 0x61:
#ifdef DEBUG
      switch (lr.AL)
      {
        case 0x01:
          bTraceNext = TRUE;
          break;

        case 0x02:
          bDumpRegs = FALSE;
          break;
      }
#endif
      lr.AL = 0x00;
      break;
#endif

      /* UNDOCUMENTED: return current psp                             
         case 0x62: is in int21_syscall
         lr.BX = cu_psp;
         break;
       */

      /* UNDOCUMENTED: Double byte and korean tables                  */
    case 0x63:
      {
        lr.DS = FP_SEG(&nlsDBCSHardcoded);
        lr.SI = FP_OFF(&nlsDBCSHardcoded);
#if 0
        /* not really supported, but will pass.                 */
        lr.AL = 0x00;           /*jpp: according to interrupt list */
        /*Bart: fails for PQDI and WATCOM utilities: 
           use the above again */
#endif
        break;
      }
/*
    case 0x64:
      see above (invalid)
*/

      /* Extended country info                                        */
    case 0x65:
      switch (lr.AL)
      {
        case 0x20:             /* upcase single character */
          lr.DL = DosUpChar(lr.DL);
          break;
        case 0x21:             /* upcase memory area */
          DosUpMem(FP_DS_DX, lr.CX);
          break;
        case 0x22:             /* upcase ASCIZ */
          DosUpString(FP_DS_DX);
          break;
        case 0xA0:             /* upcase single character of filenames */
          lr.DL = DosUpFChar(lr.DL);
          break;
        case 0xA1:             /* upcase memory area of filenames */
          DosUpFMem(FP_DS_DX, lr.CX);
          break;
        case 0xA2:             /* upcase ASCIZ of filenames */
          DosUpFString(FP_DS_DX);
          break;
        case 0x23:             /* check Yes/No response */
          lr.AX = DosYesNo(lr.DL);
          break;
        default:
          if ((rc = DosGetData(lr.AL, lr.BX, lr.DX, lr.CX,
                               FP_ES_DI)) < 0)
          {
#ifdef NLS_DEBUG
            printf("DosGetData() := %d\n", rc);
#endif
            goto error_exit;
          }
#ifdef NLS_DEBUG
          printf("DosGetData() returned successfully\n", rc);
#endif

          break;
      }
      CLEAR_CARRY_FLAG();
      break;

      /* Code Page functions */
    case 0x66:
        CLEAR_CARRY_FLAG();
        switch (lr.AL)
        {
          case 1:
            rc = DosGetCodepage(&lr.BX, &lr.DX);
            break;
          case 2:
            rc = DosSetCodepage(lr.BX, lr.DX);
            break;

          default:
            goto error_invalid;
        }
        if (rc != SUCCESS)
          goto error_exit;
        break;

      /* Set Max file handle count */
    case 0x67:
      CLEAR_CARRY_FLAG();
      if ((rc = SetJFTSize(lr.BX)) != SUCCESS)
        goto error_exit;
      break;

      /* Flush file buffer -- COMMIT FILE.  */
    case 0x68:
    case 0x6a:
      CLEAR_CARRY_FLAG();
      if ((rc = DosCommit(lr.BX)) < 0)
        goto error_exit;
      break;

      /* Get/Set Serial Number */
    case 0x69:
      CLEAR_CARRY_FLAG();
      rc = (lr.BL == 0 ? default_drive : lr.BL - 1);
      if (rc < lastdrive)
      {
        UWORD saveCX = lr.CX;
        if (CDSp[rc].cdsFlags & CDSNETWDRV)
        {
          goto error_invalid;
        }
        switch (lr.AL)
        {
          case 0x00:
            lr.AL = 0x0d;
            lr.CX = 0x0866;
            rc = DosDevIOctl(&lr);
            break;

          case 0x01:
            lr.AL = 0x0d;
            lr.CX = 0x0846;
            rc = DosDevIOctl(&lr);
            break;
        }
        lr.CX = saveCX;
        if (rc != SUCCESS)
          goto error_exit;
        break;
      }
      else
        lr.AL = 0xFF;
      break;
/*
    case 0x6a: see case 0x68
    case 0x6b: dummy func: return AL=0
*/
      /* Extended Open-Creat, not fully functional. (bits 4,5,6 of BH) */
    case 0x6c:
      {
        long lrc;
        CLEAR_CARRY_FLAG();
      
        if (lr.AL != 0 || lr.DH != 0 ||
            (lr.DL & 0x0f) > 0x2 || (lr.DL & 0xf0) > 0x10)
          goto error_invalid;
        lrc = DosOpen(MK_FP(lr.DS, lr.SI),
                      (lr.BX & 0x7000) | ((lr.DL & 3) << 8) |
                      ((lr.DL & 0x10) << 6), lr.CL);
        if (lrc < 0)
        {
          rc = (COUNT)lrc;
          goto error_exit;
        }
        else
        {
          lr.AX = (UWORD)lrc;
          /* action */
          lr.CX = lrc >> 16;
        }
        break;
      }

      /* case 0x6d and above not implemented : see default; return AL=0 */

#ifdef WITHFAT32
      /* DOS 7.0+ FAT32 extended functions */
    case 0x73:
      CLEAR_CARRY_FLAG();
      rc = int21_fat32(&lr);
      if (rc != SUCCESS)
        goto error_exit;
      break;
#endif
      
#ifdef WITHLFNAPI
      /* FreeDOS LFN helper API functions */
    case 0x74:
      {
        switch (lr.AL)
        {
          /* Allocate LFN inode */
          case 0x01:
            rc = lfn_allocate_inode();
            break;
          /* Free LFN inode */
          case 0x02:
            rc = lfn_free_inode(lr.BX);
            break;
          /* Setup LFN inode */
          case 0x03:
            rc = lfn_setup_inode(lr.BX, ((ULONG)lr.CX << 16) | lr.DX, ((ULONG)lr.SI << 16) | lr.DI);
            break;
          /* Create LFN entries */
          case 0x04:
            rc = lfn_create_entries(lr.BX, (lfn_inode_ptr)FP_DS_DX);
            break;
          /* Read next LFN */
          case 0x05:
            rc = lfn_dir_read(lr.BX, (lfn_inode_ptr)FP_DS_DX);
            break;
          /* Write SFN pointed by LFN inode */
          case 0x06:
            rc = lfn_dir_write(lr.BX);
            break;
          default:
            goto error_invalid;
        }
        lr.AX = rc;
        if (rc < 0) goto error_exit;
        else CLEAR_CARRY_FLAG();
        break;
      }
#endif
  }
  goto exit_dispatch;
  
error_invalid:
  rc = DE_INVLDFUNC;
error_exit:
  lr.AX = -rc;
  CritErrCode = lr.AX;      /* Maybe set */
error_carry:
  SET_CARRY_FLAG();
exit_dispatch:
  r->AX = lr.AX;
  r->BX = lr.BX;
  r->CX = lr.CX;
  r->DX = lr.DX;
  r->SI = lr.SI;
  r->DI = lr.DI;
  r->DS = lr.DS;
  r->ES = lr.ES;
real_exit:  

#ifdef DEBUG
  if (bDumpRegs)
  {
    fmemcpy(&error_regs, user_r, sizeof(iregs));
    dump_regs = TRUE;
    dump();
  }
#endif
  return;
}

#if 0
        /* No kernel INT-23 handler required no longer -- 1999/04/15 ska */
/* ctrl-Break handler */
#pragma argsused
VOID INRPT FAR int23_handler(int es, int ds, int di, int si, int bp,
                             int sp, int bx, int dx, int cx, int ax,
                             int ip, int cs, int flags)
{
  tsr = FALSE;
  return_mode = 1;
  return_code = -1;
  mod_sto(CTL_C);
  DosMemCheck();
#ifdef TSC
  StartTrace();
#endif
  return_user();
}
#endif

struct int25regs {
  UWORD es, ds;
  UWORD di, si, bp, sp;
  UWORD bx, dx, cx, ax;
  UWORD flags, ip, cs;
};

/* 
    this function is called from an assembler wrapper function 
*/
VOID ASMCFUNC int2526_handler(WORD mode, struct int25regs FAR * r)
{
  ULONG blkno;
  UWORD nblks;
  BYTE FAR *buf;
  UBYTE drv;

  if (mode == 0x26)
    mode = DSKWRITEINT26;
  else
    mode = DSKREADINT25;

  drv = r->ax;

  if (drv >= lastdrive)
  {
    r->ax = 0x201;
    r->flags |= FLG_CARRY;
    return;
  }

#ifdef WITHFAT32
  if (!(CDSp[drv].cdsFlags & CDSNETWDRV) &&
      ISFAT32(CDSp[drv].cdsDpb))
  {
    r->ax = 0x207;
    r->flags |= FLG_CARRY;
    return;
  }
#endif

  nblks = r->cx;
  blkno = r->dx;

  buf = MK_FP(r->ds, r->bx);

  if (nblks == 0xFFFF)
  {
    /*struct HugeSectorBlock FAR *lb = MK_FP(r->ds, r->bx); */
    blkno = ((struct HugeSectorBlock FAR *)buf)->blkno;
    nblks = ((struct HugeSectorBlock FAR *)buf)->nblks;
    buf = ((struct HugeSectorBlock FAR *)buf)->buf;
  }

  InDOS++;

  r->ax = dskxfer(drv, blkno, buf, nblks, mode);

  if (mode == DSKWRITE)
    if (r->ax <= 0)
      setinvld(drv);

  if (r->ax > 0)
  {
    r->flags |= FLG_CARRY;
    --InDOS;
    return;
  }

  r->ax = 0;
  r->flags &= ~FLG_CARRY;
  --InDOS;

}

/*
VOID int25_handler(struct int25regs FAR * r) { int2526_handler(DSKREAD,r); }
VOID int26_handler(struct int25regs FAR * r) { int2526_handler(DSKWRITE,r); }
*/

#ifdef TSC
STATIC VOID StartTrace(VOID)
{
  if (bTraceNext)
  {
#ifdef DEBUG
    bDumpRegs = TRUE;
#endif
    bTraceNext = FALSE;
  }
#ifdef DEBUG
  else
    bDumpRegs = FALSE;
#endif
}
#endif

/* 
    this function is called from an assembler wrapper function 
    and serves the internal dos calls - int2f/12xx
*/
struct int2f12regs {
  UWORD es, ds;
  UWORD di, si, bp, bx, dx, cx, ax;
  UWORD ip, cs, flags;
  UWORD callerARG1;             /* used if called from INT2F/12 */
};

VOID ASMCFUNC int2F_12_handler(volatile struct int2f12regs r)
{
  UWORD function = r.ax & 0xff;

  if (function > 0x31)
    return;

  switch (function)
  {
    case 0x00:                 /* installation check */
      r.ax |= 0x00ff;
      break;

    case 0x03:                 /* get DOS data segment */
      r.ds = FP_SEG(&nul_dev);
      break;

    case 0x06:                 /* invoke critical error */

      /* code, drive number, error, device header */
      r.ax &= 0xff00;
      r.ax |= CriticalError(r.callerARG1 >> 8,
                            (r.callerARG1 & (EFLG_CHAR << 8)) ? 0 : r.
                            callerARG1 & 0xff, r.di, MK_FP(r.bp, r.si));
      break;

    case 0x08:                 /* decrease SFT reference count */
      {
        sft FAR *p = MK_FP(r.es, r.di);

        r.ax = p->sft_count;

        if (--p->sft_count == 0)
          --p->sft_count;
      }
      break;

    case 0x0c:                 /* perform "device open" for device, set owner for FCB */

      if (lpCurSft->sft_flags & SFT_FDEVICE)
      {
        request rq;

        rq.r_unit = 0;
        rq.r_status = 0;
        rq.r_command = C_OPEN;
        rq.r_length = sizeof(request);
        execrh((request FAR *) & rq, lpCurSft->sft_dev);
      }

      /* just do it always, not just for FCBs */
      lpCurSft->sft_psp = cu_psp;
      break;

    case 0x0d:                 /* get dos date/time */

      r.ax = dos_getdate();
      r.dx = dos_gettime();
      break;

    case 0x12:                 /* get length of asciiz string */

      r.cx = fstrlen(MK_FP(r.es, r.di)) + 1;

      break;

    case 0x16:                 /* get address of system file table entry - used by NET.EXE
                                   BX system file table entry number ( such as returned from 2F/1220)
                                   returns
                                   ES:DI pointer to SFT entry */
      {
        sft FAR *p = get_sft(r.bx);

        r.es = FP_SEG(p);
        r.di = FP_OFF(p);
        break;
      }

    case 0x17:                 /* get current directory structure for drive - used by NET.EXE
                                   STACK: drive (0=A:,1=B,...)
                                   ; returns
                                   ;   CF set if error
                                   ;   DS:SI pointer to CDS for drive
                                   ; 
                                   ; called like
                                   ;   push 2 (c-drive)
                                   ;   mov ax,1217
                                   ;   int 2f
                                   ;
                                   ; probable use: get sizeof(CDSentry)
                                 */
      {
        UWORD drv = r.callerARG1 & 0xff;

        if (drv >= lastdrive)
          r.flags |= FLG_CARRY;
        else
        {
          r.ds = FP_SEG(CDSp);
          r.si = FP_OFF(&CDSp[drv]);
          r.flags &= ~FLG_CARRY;
        }
        break;
      }

    case 0x18:                 /* get caller's registers */

      r.ds = FP_SEG(user_r);
      r.si = FP_OFF(user_r);
      break;

    case 0x1b:                 /* #days in February - valid until 2099. */

      r.ax = (r.ax & 0xff00) | (r.cx & 3 ? 28 : 29);
      break;

    case 0x21:                 /* truename */

      DosTruename(MK_FP(r.ds, r.si), MK_FP(r.es, r.di));

      break;

    case 0x23:                 /* check if character device */
      {
        struct dhdr FAR *dhp;

        dhp = IsDevice((BYTE FAR *) DirEntBuffer.dir_name);

        if (dhp)
        {
          r.bx = (r.bx & 0xff) | (dhp->dh_attr << 8);
          r.flags &= ~FLG_CARRY;
        }
        else
        {
          r.flags |= FLG_CARRY;
        }

      }

      break;

    case 0x25:                 /* get length of asciiz string */

      r.cx = fstrlen(MK_FP(r.ds, r.si)) + 1;
      break;

    case 0x2a:                 /* Set FastOpen but does nothing. */

      r.flags &= ~FLG_CARRY;
      break;

    case 0x2c:                 /* added by James Tabor For Zip Drives
                                   Return Null Device Pointer          */
      /* by UDOS+RBIL: get header of SECOND device driver in device chain, 
         omitting the NUL device TE */
      r.bx = FP_SEG(nul_dev.dh_next);
      r.ax = FP_OFF(nul_dev.dh_next);

      break;

    case 0x2e:                 /* GET or SET error table addresse - ignored
                                   called by MS debug with  DS != DOSDS, printf
                                   doesn't work!! */
      break;

    default:
      printf("unimplemented internal dos function INT2F/12%02x\n",
             function);
      r.flags |= FLG_CARRY;
      break;

  }

}

/*
 * 2000/09/04  Brian Reifsnyder
 * Modified interrupts 0x25 & 0x26 to return more accurate error codes.
 */

/*
 * Log: inthndlr.c,v - see "cvs log inthndlr.c" for newer entries.
 *
 * Revision 1.10  2000/10/29 23:51:56  jimtabor
 * Adding Share Support by Ron Cemer
 *
 * Revision 1.9  2000/08/06 05:50:17  jimtabor
 * Add new files and update cvs with patches and changes
 *
 * Revision 1.8  2000/06/21 18:16:46  jimtabor
 * Add UMB code, patch, and code fixes
 *
 * Revision 1.7  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.6  2000/05/17 19:15:12  jimtabor
 * Cleanup, add and fix source.
 *
 * Revision 1.5  2000/05/11 06:14:45  jimtabor
 * Removed #if statement
 *
 * Revision 1.4  2000/05/11 04:26:26  jimtabor
 * Added code for DOS FN 69 & 6C
 *
 * Revision 1.3  2000/05/09 00:30:11  jimtabor
 * Clean up and Release
 *
 * Revision 1.2  2000/05/08 04:30:00  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.24  2000/04/29 05:13:16  jtabor
 *  Added new functions and clean up code
 *
 * Revision 1.22  2000/03/17 22:59:04  kernel
 * Steffen Kaiser's NLS changes
 *
 * Revision 1.21  2000/03/17 05:00:11  kernel
 * Fixed Func 0x32
 *
 * Revision 1.20  2000/03/16 03:28:49  kernel
 * *** empty log message ***
 *
 * Revision 1.19  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.18  1999/09/23 04:40:47  jprice
 * *** empty log message ***
 *
 * Revision 1.13  1999/09/14 01:18:36  jprice
 * ror4: fix int25 & 26 are not cached.
 *
 * Revision 1.12  1999/09/13 22:16:47  jprice
 * Fix 210B function
 *
 * Revision 1.11  1999/08/25 03:18:08  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.10  1999/08/10 18:07:57  jprice
 * ror4 2011-04 patch
 *
 * Revision 1.9  1999/08/10 18:03:43  jprice
 * ror4 2011-03 patch
 *
 * Revision 1.8  1999/05/03 06:25:45  jprice
 * Patches from ror4 and many changed of signed to unsigned variables.
 *
 * Revision 1.7  1999/04/23 04:24:39  jprice
 * Memory manager changes made by ska
 *
 * Revision 1.6  1999/04/16 12:21:22  jprice
 * Steffen c-break handler changes
 *
 * Revision 1.5  1999/04/11 04:33:39  jprice
 * ror4 patches
 *
 * Revision 1.3  1999/04/04 22:57:47  jprice
 * no message
 *
 * Revision 1.2  1999/04/04 18:51:43  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:41:04  jprice
 * New version without IPL.SYS
 *
 * Revision 1.9  1999/03/23 23:38:49  jprice
 * Now sets carry when we don't support a function
 *
 * Revision 1.8  1999/03/02 07:02:55  jprice
 * Added some comments.  Fixed some minor bugs.
 *
 * Revision 1.7  1999/03/01 05:45:08  jprice
 * Added some DEBUG ifdef's so that it will compile without DEBUG defined.
 *
 * Revision 1.6  1999/02/08 05:55:57  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.5  1999/02/04 03:11:07  jprice
 * Formating
 *
 * Revision 1.4  1999/02/01 01:48:41  jprice
 * Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
 *
 * Revision 1.3  1999/01/30 08:28:11  jprice
 * Clean up; Fixed bug with set attribute function.
 *
 * Revision 1.2  1999/01/22 04:13:26  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:00  jprice
 * Imported sources
 *
 *
 *    Rev 1.14   06 Dec 1998  8:47:38   patv
 * Expanded due to improved int 21h handler code.
 *
 *    Rev 1.13   07 Feb 1998 20:38:46   patv
 * Modified stack fram to match DOS standard
 *
 *    Rev 1.12   22 Jan 1998  4:09:26   patv
 * Fixed pointer problems affecting SDA
 *
 *    Rev 1.11   06 Jan 1998 20:13:18   patv
 * Broke apart int21_system from int21_handler.
 *
 *    Rev 1.10   04 Jan 1998 23:15:22   patv
 * Changed Log for strip utility
 *
 *    Rev 1.9   04 Jan 1998 17:26:16   patv
 * Corrected subdirectory bug
 *
 *    Rev 1.8   03 Jan 1998  8:36:48   patv
 * Converted data area to SDA format
 *
 *    Rev 1.7   01 Aug 1997  2:00:10   patv
 * COMPATIBILITY: Added return '$' in AL for function int 21h fn 09h
 *
 *    Rev 1.6   06 Feb 1997 19:05:54   patv
 * Added hooks for tsc command
 *
 *    Rev 1.5   22 Jan 1997 13:18:32   patv
 * pre-0.92 Svante Frey bug fixes.
 *
 *    Rev 1.4   16 Jan 1997 12:46:46   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.3   29 May 1996 21:03:40   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.2   19 Feb 1996  3:21:48   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.1   01 Sep 1995 17:54:20   patv
 * First GPL release.
 *
 *    Rev 1.0   02 Jul 1995  8:33:34   patv
 * Initial revision.
 */
