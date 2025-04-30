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

enum { HEAP_FAR, HEAP_NEAR, HEAP_LOW, HEAP_MAX };
struct HeapS {
    struct DynS Dynp_;
    #define Dynp Dynp_.Buffer
    UDWORD Allocated;
    UDWORD MaxSize;
    void (*AllocHook)(struct HeapS *, unsigned);
};
STATIC struct HeapS *HeapMap[HEAP_MAX];
enum { H_DYN, H_OTHER, H_HMA, HEAPS };
STATIC struct HeapS Heap[HEAPS];

static void BasicAllocHook(struct HeapS *h, unsigned size)
{
  void FAR *now = (void FAR *)&h->Dynp[h->Allocated];
  fmemset(now, 0, size);
}

static void HmaAllocHook(struct HeapS *h, unsigned size)
{
    UDWORD off = FP_OFF(h->Dynp) + h->Allocated;
    ___assert(HMAFree == (LONG)((off + 0xf) & 0x1fff0));
    HMAFree = (off + size + 0xf) & 0x1fff0;
}

void DynInit(void)
{
  void FAR *dyn = MK_FP(FP_SEG(LoL), FP_OFF(Dyn));
  void FAR *hma = MK_FP(0xffff, HMAFree);
  int i;

  for (i = 0; i < HEAPS; i++) {
    Heap[i].Dynp = NULL;
    Heap[i].Allocated = 0;
    Heap[i].MaxSize = 0;
    Heap[i].AllocHook = BasicAllocHook;
  }
  Heap[H_DYN].Dynp = dyn;
  HeapMap[HEAP_NEAR] = &Heap[H_DYN];
  Heap[H_HMA].Dynp = hma;
  Heap[H_HMA].MaxSize = 0x10000 - HMAFree;
  Heap[H_HMA].AllocHook = HmaAllocHook;

#define KERNEL_HIGH() (bprm.Flags & FDPP_FL_KERNEL_HIGH)
#define HEAP_HIGH() (bprm.Flags & FDPP_FL_HEAP_HIGH)
#define HEAP_HMA() (bprm.Flags & FDPP_FL_HEAP_HMA)

  if (!KERNEL_HIGH() && !HEAP_HIGH()) {
    /* legacy layout, Dyn only */
    Heap[H_DYN].MaxSize = 0x10000;
    for (i = 0; i < HEAP_MAX; i++)
      HeapMap[i] = &Heap[H_DYN];
  } else if (KERNEL_HIGH()) {
    ___assert(bprm.HeapSeg < DOS_PSP ||
        bprm.HeapSeg >= DOS_PSP + (sizeof(psp) >> 4));
    Heap[H_DYN].MaxSize = bprm.HeapSize + (InitEnd - dyn);
    Heap[H_OTHER].Dynp = MK_FP(bprm.HeapSeg, 0);
    Heap[H_OTHER].MaxSize = 0x10000;
    HeapMap[HEAP_FAR] = &Heap[HEAP_HIGH() ? (HEAP_HMA() ? H_HMA : H_DYN) :
        H_OTHER];
    HeapMap[HEAP_LOW] = &Heap[H_OTHER];
  } else {
    /* kernel low, heap in UMA or HMA */
    ___assert(bprm.HeapSeg && bprm.HeapSize);
    Heap[H_DYN].MaxSize = 0x10000;
    Heap[H_OTHER].Dynp = MK_FP(bprm.HeapSeg, 0);
    Heap[H_OTHER].MaxSize = bprm.HeapSize;
    HeapMap[HEAP_FAR] = &Heap[HEAP_HMA() ? H_HMA : H_OTHER];
    HeapMap[HEAP_LOW] = &Heap[H_DYN];
  }
}

static far_t DoAlloc(const char *what, unsigned num, unsigned size, int heap)
{
  void FAR *now;
  unsigned total = num * size;
  struct HeapS *h = HeapMap[heap];

#ifndef DEBUG
  UNREFERENCED_PARAMETER(what);
#endif

  if ((ULONG) total + h->Allocated > h->MaxSize)
  {
    char buf[256];
    _snprintf(buf, sizeof(buf), "Dyn %u", (ULONG) total + h->Allocated);
    panic(buf);
  }

#if 0
  /* can't do prints here as the subsystems are not yet inited */
  DebugPrintf(("DYNDATA:allocating %s - %u * %u bytes, total %u, %u..%u\n",
               what, num, size, total, Allocated,
               Allocated + total));
#endif

  now = (void FAR *)&h->Dynp[h->Allocated];

  h->AllocHook(h, total);
  h->Allocated += total;

  return GET_FAR(now);
}

far_t DynAlloc(const char *what, unsigned num, unsigned size)
{
  return DoAlloc(what, num, size, HEAP_FAR);
}

far_t DynAllocNear(const char *what, unsigned num, unsigned size)
{
  return DoAlloc(what, num, size, HEAP_NEAR);
}

far_t DynAllocLow(const char *what, unsigned num, unsigned size)
{
  return DoAlloc(what, num, size, HEAP_LOW);
}

void FAR *DynLast(void)
{
  struct HeapS *h = HeapMap[HEAP_LOW];
  DebugPrintf(("dynamic data end at %P\n",
               GET_FP32(h->Dynp + h->Allocated)));

  return h->Dynp + h->Allocated;
}

seg DynLastSeg(void)
{
  struct HeapS *h = &Heap[H_DYN];
  seg ret = FP_SEG(h->Dynp);

  ___assert(!(FP_OFF(h->Dynp) & 0xf));
  return ret + ((FP_OFF(h->Dynp) + ((h->Allocated + 0xf) & 0x1fff0)) >> 4);
}

UDWORD DynRemained(void)
{
  struct HeapS *h = &Heap[H_DYN];

  ___assert(h->MaxSize >= h->Allocated);
  return h->MaxSize - h->Allocated;
}
