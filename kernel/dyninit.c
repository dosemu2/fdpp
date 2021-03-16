/*
    DYNINIT.C

    this serves requests from the INIT modules to
    allocate dynamic data.

kernel layout:
 00000H 000FFH 00100H PSP                PSP
 00100H 004E1H 003E2H _TEXT              CODE
 004E2H 007A7H 002C6H _IO_TEXT           CODE
 007A8H 008E5H 0013EH _IO_FIXED_DATA     CODE
 008F0H 0139FH 00AB0H _FIXED_DATA        DATA
 013A0H 019F3H 00654H _DATA              DATA
 019F4H 0240DH 00A1AH _BSS               BSS

additionally:
                      DYN_DATA           DYN


 02610H 0F40EH 0CDFFH HMA_TEXT           HMA

 FCBs, f_nodes, buffers,...
 drivers


 0F410H 122DFH 02ED0H INIT_TEXT          INIT
 122E0H 12AA5H 007C6H ID                 ID
 12AA6H 12CBFH 0021AH IB                 IB

 purpose is to move the HMA_TEXT = resident kernel
 around, so that below it - after BSS, there is data
 addressable near by the kernel, to hold some arrays
 like f_nodes

 making f_nodes near saves ~2.150 code in HMA

*/
#include "portab.h"
#include "globals.h"
#include "init-mod.h"
#include "dyn.h"
#include "dyndata.h"
#include "debug.h"

#ifndef __TURBOC__
#include "init-dat.h"
#else
extern struct DynS FAR ASM Dyn;
#endif
STATIC struct DynS FAR *Dynp;
STATIC BSS(unsigned, Allocated, 0);

void DynInit(void FAR *ptr)
{
  Dynp = ptr;
}

far_t DynAlloc(const char *what, unsigned num, unsigned size)
{
  void FAR *now;
  unsigned total = num * size;

#ifndef DEBUG
  UNREFERENCED_PARAMETER(what);
#endif

  if ((ULONG) total + Allocated > 0xffff)
  {
    char buf[256];
    _snprintf(buf, sizeof(buf), "Dyn %u", (ULONG) total + Allocated);
    panic(buf);
  }

#if 0
  /* can't do prints here as the subsystems are not yet inited */
  DebugPrintf(("DYNDATA:allocating %s - %u * %u bytes, total %u, %u..%u\n",
               what, num, size, total, Allocated,
               Allocated + total));
#endif

  now = (void FAR *)&Dynp->Buffer[Allocated];
  fmemset(now, 0, total);

  Allocated += total;

  return GET_FAR(now);
}

void DynFree(void *ptr)
{
  Allocated = (char *)ptr - (char *)Dynp->Buffer;
}

far_t DynLast()
{
  DebugPrintf(("dynamic data end at %P\n",
               GET_FP32(Dynp->Buffer + Allocated)));

  return GET_FAR(Dynp->Buffer + Allocated);
}
