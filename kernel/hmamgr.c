/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2019  Stas Sergeev (stsp)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "portab.h"
#include "globals.h"

typedef struct HMCB {
    uint16_t signature;   // "MS"
    uint16_t owner;       // 0000=free, 0001=DOS, FF33=IO.SYS, FFFF=MSDOS.SYS
    uint16_t size;        // bytes not including this header
    uint16_t next;        // offset of next memory block in segment FFFFh, or 0000h if last
    uint8_t reserved[8];  // unused (explicitly set to 0 for MS-DOS 7.10)
} __attribute__((packed)) hmcb;

/* "MS" */
#define HMCB_SIG 0x534d

static hmcb init_mcb = { .signature = HMCB_SIG, };

#define FIRST_MCB 0x10

#define nxtMCBsize(mcb,size) MK_FP(FP_SEG(mcb), FP_OFF(mcb) + (size) + 16)
#define nxtMCB(mcb) MK_FP(FP_SEG(mcb), mcb->next)

#define mcbFree(mcb) ((mcb)->owner == 0)
#define mcbValid(mcb) ( ((mcb)->signature == HMCB_SIG) )
#define mcbFreeable(mcb) mcbValid(mcb)
#define mcbLast(mcb) ((mcb)->next == 0)

#define off2far(off) (hmcb FAR *)MK_FP(0xffff, (off))
#define mcb2off(mcb) FP_OFF(mcb)
#define MCBDESTRY2(p, q) (fdebug("MCB corruption, good:%P bad:%P\n", \
	GET_FAR(p), GET_FAR(q)),_fail(),DE_MCBDESTRY)
#define MCBDESTRY(p) (fdebug("MCB corruption, bad:%P\n", \
	GET_FAR(p)),_fail(),DE_MCBDESTRY)

/*
 * Join any following unused MCBs to MCB 'p'.
 *  Return:
 *  SUCCESS: on success
 *  else: error number <<currently DE_MCBDESTRY only>>
 */
STATIC COUNT joinMCBs(UWORD off)
{
  hmcb FAR *p = off2far(off);
  hmcb FAR *q;

  /* loop as long as the current MCB is not the last one in the chain
     and the next MCB is unused */
  while (!mcbLast(p))
  {
    q = nxtMCB(p);
    if (!mcbFree(q))
      break;
    if (!mcbValid(q))
      return MCBDESTRY2(p, q);
    /* join both MCBs */
    fd_prot_mem(p, sizeof(*p), FD_MEM_NORMAL);
    p->next = q->next;       /* possibly the next MCB is the last one */
    p->size += q->size + 16; /* one for q's MCB itself */
    fd_prot_mem(p, sizeof(*p), FD_MEM_READONLY);
    fd_prot_mem(q, sizeof(*q), FD_MEM_NORMAL);
#if 0				/* this spoils QB4's habit to double-free: */
    q->m_type = 'K';            /* Invalidate the magic number (whole MCB) */
#else
    q->size = 0xffff;		/* mark the now unlinked MCB as "fake" */
#endif
#if 0
    fd_mark_mem(q, sizeof(*q), FD_MEM_UNINITIALIZED);
#else
    /* uninitialized above produces too many false-positives */
    fd_mark_mem(q, sizeof(*q), FD_MEM_NORMAL);
#endif
  }

  return SUCCESS;
}

/* Allocate a new memory area. *para is assigned to the segment of the
   MCB rather then the segment of the data portion */
/* If mode == LARGEST, asize MUST be != NULL and will always recieve the
   largest available block, which is allocated.
   If mode != LARGEST, asize maybe NULL, but if not, it is assigned to the
   size of the largest available block only on failure.
   size is the minimum size of the block to search for,
   even if mode == LARGEST.
 */
COUNT DosHMAAlloc(UWORD size, UWORD *off)
{
  REG hmcb FAR *p;
  REG hmcb FAR *q = NULL;
  hmcb FAR *foundSeg;
  hmcb FAR *biggestSeg;

  if (!HMAclaimed)
    return DE_NOMEM;

  /* Initialize                                           */
  p = off2far(FIRST_MCB);

  biggestSeg = foundSeg = NULL;

  /* Search through memory blocks                         */
  FOREVER
  {
    /* check for corruption                         */
    if (!mcbValid(p))
      return MCBDESTRY2(q, p);

    if (mcbFree(p))
    {                           /* unused block, check if it applies to the rule */
      if (joinMCBs(mcb2off(p)) != SUCCESS)       /* join following unused blocks */
        return MCBDESTRY2(q, p);    /* error */

      if (!biggestSeg || biggestSeg->size < p->size)
        biggestSeg = p;

      if (p->size >= size)
      {                         /* if the block is too small, ignore */
        foundSeg = p;
        goto stopIt;        /* OK, rest of chain can be ignored */
      }
    }

    if (mcbLast(p))
      break;                    /* end of chain reached */

    q = p;
    p = nxtMCB(p);              /* advance to next MCB */
  }

  if (!foundSeg || !foundSeg->size)
  {                             /* no block to fullfill the request */
    return DE_NOMEM;
  }

stopIt:                        /* reached from FIRST_FIT on match */

  if (size != foundSeg->size)
  {
    /* Split the found buffer because it is larger than requested */
    /* foundSeg := pointer to allocated block
       p := pointer to MCB that will form the rest of the block
     */
    p = nxtMCBsize(foundSeg, size);

    fd_prot_mem(p, sizeof(*p), FD_MEM_NORMAL);
    /* initialize stuff because p > foundSeg  */
    *p = init_mcb;
    p->next = foundSeg->next;
    p->size = foundSeg->size - size - 16;
    fd_mark_mem(p, sizeof(*p), FD_MEM_READONLY);
    fd_prot_mem(foundSeg, sizeof(*foundSeg), FD_MEM_NORMAL);
    foundSeg->next = mcb2off(p);

    /* Already initialized:
       p->size, ->m_type, foundSeg->m_type
     */
    p->owner = 0;        /* unused */
    fd_prot_mem(p, sizeof(*p), FD_MEM_READONLY);

    foundSeg->size = size;
    fd_prot_mem(foundSeg, sizeof(*foundSeg), FD_MEM_READONLY);
  }

  /* Already initialized:
     foundSeg->size, ->m_type
   */
  fd_prot_mem(foundSeg, sizeof(*foundSeg), FD_MEM_NORMAL);
  foundSeg->owner = cu_psp;     /* the new block is for current process */
  fd_prot_mem(foundSeg, sizeof(*foundSeg), FD_MEM_READONLY);

  *off = mcb2off(foundSeg) + 16;
  return SUCCESS;
}

/*
 * Deallocate a memory block. para is the segment of the MCB itself
 * This function can be called with para == 0, which eases other parts
 * of the kernel.
 */
COUNT DosHMAFree(UWORD off)
{
  REG hmcb FAR *p;

  if (!HMAclaimed)
    return DE_NOMEM;

  if (!off)                    /* let esp. the kernel call this fct with para==0 */
    return DE_INVLDMCB;

  /* Initialize                                           */
  p = off2far(off - 16);

  /* check for corruption                         */
  if (!mcbFreeable(p))	/* does not have to be valid, freeable is enough */
    return DE_INVLDMCB;

  if (mcbFree(p))
    return SUCCESS;

  fd_prot_mem(p, sizeof(*p), FD_MEM_NORMAL);
  /* Mark the mcb as free so that we can later    */
  /* merge with other surrounding free MCBs       */
  p->owner = 0;
  fd_prot_mem(p, sizeof(*p), FD_MEM_READONLY);

  /* try to join p with the free MCBs following it if possible */
  if (joinMCBs(mcb2off(p)) != SUCCESS)
    return MCBDESTRY(p);

  return SUCCESS;
}

/*
 * Resize an allocated memory block.
 * para is the segment of the data portion of the block rather than
 * the segment of the MCB itself.
 *
 * Resize on an unallocated block is allowed and makes it allocated.
 *
 * If the block shall grow, it is resized to the maximal size less than
 * or equal to size. This is the way MS DOS is reported to work.
 */
COUNT DosHMAChange(UWORD off, UWORD size)
{
  REG hmcb FAR *p;
  REG hmcb FAR *q;

  if (!HMAclaimed)
    return DE_NOMEM;

  /* checked with PC-DOS: realloc to 0 doesn't mean free. :(
   * If you change this, also see handling in return_user(). */
  if (size == 0)
    return DosHMAFree(off - 16);
  /* Initialize                                                   */
  p = off2far(off - 16);       /* pointer to MCB */

  /* check for corruption                                         */
  if (!mcbValid(p))
    return DE_INVLDMCB;

  /* check if to grow the block                                   */
  if (size > p->size)
  {
    /* first try to make the MCB larger by joining with any following
       unused blocks */
    if (joinMCBs(mcb2off(p)) != SUCCESS)
      return MCBDESTRY(p);

    if (size > p->size)
    {                           /* block is still too small */
      return DE_NOMEM;
    }
  }

  fd_prot_mem(p, sizeof(*p), FD_MEM_NORMAL);
  /*       shrink it down                                         */
  if (size < p->size)
  {
    /* make q a pointer to the new next block               */
    q = nxtMCBsize(p, size);
    /* reduce the size of p and add difference to q         */
    fd_prot_mem(q, sizeof(*q), FD_MEM_NORMAL);
    *q = init_mcb;
    q->size = p->size - size - 16;
    q->next = p->next;
    fd_mark_mem(q, sizeof(*q), FD_MEM_READONLY);

    p->size = size;
    p->next = mcb2off(q);

    /* Mark the mcb as free so that we can later    */
    /* merge with other surrounding free MCBs       */
    q->owner = 0;
    fd_prot_mem(q, sizeof(*q), FD_MEM_READONLY);

    /* try to join q with the free MCBs following it if possible */
    if (joinMCBs(mcb2off(q)) != SUCCESS)
      return MCBDESTRY2(p, q);
  }

  /* MS network client NET.EXE: DosMemChange sets the PSP              *
   *               not tested, if always, or only on success         TE*
   * only on success seems more logical to me - Bart                                                                                                                   */
  p->owner = cu_psp;
  fd_prot_mem(p, sizeof(*p), FD_MEM_READONLY);

  return SUCCESS;
}

/*
 * Check the MCB chain for allocation corruption
 */
COUNT DosHMACheck(void)
{
  REG hmcb FAR *p;
  REG hmcb FAR *pprev = NULL;

  if (!HMAclaimed)
    return SUCCESS;

  /* Initialize                                           */
  p = off2far(FIRST_MCB);

  /* Search through memory blocks                         */
  while (!mcbLast(p)) /* not all MCBs touched */
  {
    /* check for corruption                         */
    if (!mcbValid(p))
    {
      put_string("dos mem corrupt, FIRST_MCB=");
      put_unsigned(FIRST_MCB, 16, 4);
      hexd("\nprev ", pprev, 16);
      hexd("notMZ", p, 16);
      return MCBDESTRY2(pprev, p);
    }

    /* not corrupted - but not end, bump the pointer */
    pprev = p;
    p = nxtMCB(p);
  }

  return SUCCESS;
}

COUNT DosHMAQuery(UWORD *off, UWORD *avail)
{
  hmcb FAR *p;
  hmcb FAR *q = NULL;

  if (!HMAclaimed)
    return DE_NOMEM;

  for (p = off2far(FIRST_MCB); !mcbLast(p); q = p, p = nxtMCB(p))
  {
    if (!mcbValid(p))           /* check for corruption */
      return MCBDESTRY2(q, p);
  }

  *off = mcb2off(p);
  *avail = p->size;
  return SUCCESS;
}

COUNT FreeProcessHMA(UWORD ps)
{
  hmcb FAR *p;
  hmcb FAR *q = NULL;

  if (!HMAclaimed)
    return SUCCESS;

  /* Search through all memory blocks                         */
  for (p = off2far(FIRST_MCB);; q = p, p = nxtMCB(p))
  {
    if (!mcbValid(p))           /* check for corruption */
      return MCBDESTRY2(q, p);

    if (p->owner == ps)
      DosHMAFree(mcb2off(p));

    if (mcbLast(p))
      break;
  }

  return SUCCESS;
}

void HMAInitFirstMcb(UWORD off)
{
  hmcb FAR *q = off2far(off);

  fd_prot_mem(q, sizeof(*q), FD_MEM_NORMAL);
  *q = init_mcb;
  q->size = 0x10000 - off - 0x10;
  fd_prot_mem(q, sizeof(*q), FD_MEM_READONLY);

  if (off > 0x20)
  {
    hmcb FAR *p = off2far(FIRST_MCB);

    fd_prot_mem(p, sizeof(*p), FD_MEM_NORMAL);
    *p = init_mcb;
    p->owner = 0xffff;
    p->size = off - 0x20;
    p->next = mcb2off(q);
    fd_prot_mem(p, sizeof(*p), FD_MEM_READONLY);
  }
}
