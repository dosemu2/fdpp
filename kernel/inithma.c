/****************************************************************/
/*                                                              */
/*                          initHMA.c                           */
/*                            DOS-C                             */
/*                                                              */
/*          move kernel to HMA area                             */
/*                                                              */
/*                      Copyright (c) 2001                      */
/*                      tom ehlert                              */
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
#include "init-mod.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId =
    "$Id: inithma.c 956 2004-05-24 17:07:04Z bartoldeman $";
#endif

BSS(BYTE, DosLoadedInHMA, FALSE);  /* set to TRUE if loaded HIGH          */
BSS(BYTE, HMAclaimed, 0);          /* set to TRUE if claimed from HIMEM   */
BSS(UDWORD, HMAFree, 0x10);           /* first byte in HMA not yet used      */

#ifdef DEBUG
#ifdef __TURBOC__
#define int3() __int__(3);
#elif defined(__WATCOMC__)
void int3()
{
  __asm int 3;
}
#endif
#else
#define int3()
#endif

#ifdef DEBUG
#define HMAInitPrintf(x) DebugPrintf(x)
#else
#define HMAInitPrintf(x)
#endif

#if 0
#ifdef DEBUG
STATIC VOID hdump(BYTE FAR * p)
{
  int loop;
  HMAInitPrintf(("%P", GET_FP32(p)));

  for (loop = 0; loop < 16; loop++)
    HMAInitPrintf(("%02x ", (const char)p[loop]));

  DebugPrintf(("\n"));
}
#else
#define hdump(ptr)
#endif
#endif

#define KeyboardShiftState() (*(BYTE FAR *)(MK_FP(0x40,0x17)))

/*
    this tests, if the HMA area can be enabled.
    if so, it simply leaves it on
*/

STATIC int EnabledA20(void)
{
  return fmemcmp(MK_FP_N(0, 0), MK_FP(0xffff, 0x0010), 128);
}

STATIC int EnableHMA(VOID)
{
  _EnableA20();

  if (!EnabledA20())
  {
    _printf("HMA can't be enabled\n");
    return FALSE;
  }

  _DisableA20();

#ifdef DEBUG
  if (EnabledA20())
  {
    DebugPrintf(("HMA can't be disabled - no problem for us\n"));
  }
#endif

  _EnableA20();
  if (!EnabledA20())
  {
    DebugPrintf(("HMA can't be enabled second time\n"));
    return FALSE;
  }

  HMAInitPrintf(("HMA success - leaving enabled\n"));

  return TRUE;
}

/*
    move the kernel up to high memory
    this is very unportable

    if we thin we succeeded, we return TRUE, else FALSE
*/

#define HMAOFFSET  0x20
#define HMASEGMENT 0xffff

STATIC int ClaimHMA(void)
{
  void FAR *xms_addr;

  if (HMAclaimed)
    return TRUE;

  if ((xms_addr = DetectXMSDriver()) == NULL)
    return FALSE;

  XMSDriverAddress = xms_addr;

#ifdef DEBUG
  /* A) for debugging purpose, suppress this,
     if any shift key is pressed
   */
  if (KeyboardShiftState() & 0x0f)
  {
    DebugPrintf(("Keyboard state is %0x, NOT moving to HMA\n",
           KeyboardShiftState()));
    return FALSE;
  }
#endif

  /* B) check out, if we can have HMA */

  if (!EnableHMA())
  {
    _printf("Can't enable HMA area (the famous A20), NOT moving to HMA\n");

    return FALSE;
  }

  /*  allocate HMA through XMS driver */

  if (HMAclaimed == 0 &&
      (HMAclaimed =
       call_XMScall(xms_addr, 0x0100, 0xffff)) == 0)
  {
    _printf("Can't reserve HMA area ??\n");

    return FALSE;
  }

  return TRUE;
}

int MoveKernelToHMA(void)
{
  if (DosLoadedInHMA)
    return TRUE;
  if (!ClaimHMA())
    return FALSE;
  MoveKernel(0xffff);
  /* report the fact we are running high through int 21, ax=3306 */
  LoL->_version_flags |= 0x10;
  DosLoadedInHMA = TRUE;
  return TRUE;
}

/*
    this allocates some bytes from the HMA area
    only available if DOS=HIGH was successful
*/

UWORD HMAalloc(UWORD bytesToAllocate)
{
  UWORD HMAptr;

  if (!HMAclaimed)
    ClaimHMA();
  if (!HMAclaimed || !bytesToAllocate)
    return 0xffff;

  if (HMAFree > 0x10000 - bytesToAllocate)
    return 0xffff;

  HMAptr = HMAFree;

  /* align on 16 byte boundary */
  HMAFree = (HMAFree + bytesToAllocate + 0xf) & 0xfff0;

  /*_printf("HMA allocated %d byte at %x\n", bytesToAllocate, HMAptr); */

  fmemset(MK_FP(0xffff, HMAptr), 0, bytesToAllocate);

  return HMAptr;
}

UWORD HMAquery(UWORD *bytesAvail)
{
  if (!HMAclaimed)
    ClaimHMA();
  if (!HMAclaimed || HMAFree > 0xffff)
  {
    *bytesAvail = 0;
    return 0xffff;
  }

  *bytesAvail = 0x10000 - HMAFree;
  return HMAFree;
}

DATA(UWORD, CurrentKernelSegment, 0);

void MoveKernel(UWORD NewKernelSegment)
{
  UBYTE FAR *HMADest;
  UBYTE FAR *HMASource;
  UWORD len;
  UWORD offs = 0;
  struct RelocationTable FAR *rp;
  BOOL initial = 0;

  if (CurrentKernelSegment == 0) {
    CurrentKernelSegment = FP_SEG(_HMATextStart);
    initial = 1;
  }

  if (DosLoadedInHMA)
    return;

  ___assert(!(FP_OFF(_HMATextStart) & 0xf));
  HMASource =
      MK_FP(CurrentKernelSegment, FP_OFF(_HMATextStart));
  HMADest = MK_FP(NewKernelSegment, 0x0000);

  len = FP_OFF(_HMATextEnd) - FP_OFF(_HMATextStart);

  if (NewKernelSegment == 0xffff)
  {
    HMASource += HMAOFFSET;
    HMADest += HMAOFFSET;
    len -= HMAOFFSET;
    offs = HMAOFFSET;
    HMAFree = (FP_OFF(HMADest) + len + 0xf) & 0xfff0;
  }

  if (!initial)
    fmemcpy(HMADest, HMASource, len);
  /* else it's the very first relocation: handled by kernel.asm */

  /* first free byte after HMA_TEXT on 16 byte boundary */

  if (NewKernelSegment == 0xffff)
  {
    /* jmp FAR cpm_entry (copy from 0:c0) */
    pokeb(0xffff, 0x30 * 4 + 0x10, 0xea);
    pokel(0xffff, 0x30 * 4 + 0x11, (ULONG)cpm_entry);
  }

  HMAInitPrintf(("HMA moving %P up to %P for %04x bytes\n",
                 GET_FP32(HMASource), GET_FP32(HMADest), len));

  NewKernelSegment -= FP_OFF(_HMATextStart) >> 4;
  offs += FP_OFF(_HMATextStart);
  for (rp = _HMARelocationTableStart; rp < _HMARelocationTableEnd; rp++)
  {
    *rp->jmpSegment = NewKernelSegment;
  }
  RelocHook(CurrentKernelSegment, NewKernelSegment, offs, len);
  CurrentKernelSegment = NewKernelSegment;
}
