/****************************************************************/
/*                                                              */
/*                          fattab.c                            */
/*                                                              */
/*                 FAT File System Table Functions              */
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
/****************************************************************/

#include "portab.h"
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.5  2001/04/21 22:32:53  bartoldeman
 * Init DS=Init CS, fixed stack overflow problems and misc bugs.
 *
 * Revision 1.4  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
 *
 * Revision 1.3  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.2  2000/05/08 04:30:00  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.6  1999/08/10 18:03:42  jprice
 * ror4 2011-03 patch
 *
 * Revision 1.5  1999/05/03 06:25:45  jprice
 * Patches from ror4 and many changed of signed to unsigned variables.
 *
 * Revision 1.4  1999/04/16 00:53:33  jprice
 * Optimized FAT handling
 *
 * Revision 1.3  1999/04/11 04:33:38  jprice
 * ror4 patches
 *
 * Revision 1.1.1.1  1999/03/29 15:42:09  jprice
 * New version without IPL.SYS
 *
 * Revision 1.4  1999/02/09 02:54:23  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.3  1999/02/01 01:43:28  jprice
 * Fixed findfirst function to find volume label with Windows long filenames
 *
 * Revision 1.2  1999/01/22 04:15:28  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:00  jprice
 * Imported sources
 *
 *
 *    Rev 1.7   09 Feb 1998  5:41:08   patv
 * Eliminated FAT12 EOF and error returns.
 *
 *    Rev 1.6   04 Jan 1998 23:14:38   patv
 * Changed Log for strip utility
 *
 *    Rev 1.5   16 Jan 1997 12:46:22   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.4   29 May 1996 21:15:14   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.3   19 Feb 1996  3:20:08   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.2   01 Sep 1995 17:48:42   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:50:26   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:04:56   patv
 * Initial revision.
 */

#ifdef PROTO
UCOUNT link_fat12(struct dpb FAR *, UCOUNT, UCOUNT);
UCOUNT link_fat16(struct dpb FAR *, UCOUNT, UCOUNT);
UWORD next_cl12(struct dpb FAR *, UCOUNT);
UWORD next_cl16(struct dpb FAR *, UCOUNT);
#else
UCOUNT link_fat12();
UCOUNT link_fat16();
UWORD next_cl12();
UWORD next_cl16();
#endif

/************************************************************************/
/*                                                                      */
/*                      cluster/sector routines                         */
/*                                                                      */
/************************************************************************/

/*                                                              */
/* The FAT file system is difficult to trace through FAT table. */
/* There are two kinds of FAT's, 12 bit and 16 bit. The 16 bit  */
/* FAT is the easiest, since it is noting more than a series of */
/* UWORD's. The 12 bit FAT is difficult, because it packs 3 FAT */
/* entries into two BYTE's. The are packed as follows:          */
/*                                                              */
/*      0x0003 0x0004 0x0005 0x0006 0x0007 0x0008 0x0009 ...    */
/*                                                              */
/*      are packed as                                           */
/*                                                              */
/*      0x03 0x40 0x00 0x05 0x60 0x00 0x07 0x80 0x00 0x09 ...   */
/*                                                              */
/*      12 bytes are compressed to 9 bytes                      */
/*                                                              */

UCOUNT link_fat(struct dpb FAR *dpbp, UCOUNT Cluster1, REG UCOUNT Cluster2)
{
  if (ISFAT12(dpbp))
    return link_fat12(dpbp, Cluster1, Cluster2);
  else if (ISFAT16(dpbp))
    return link_fat16(dpbp, Cluster1, Cluster2);
  else
    return DE_BLKINVLD;
}

UCOUNT link_fat16(struct dpb FAR *dpbp, UCOUNT Cluster1, UCOUNT Cluster2)
{
  UCOUNT idx;
  struct buffer FAR *bp;
  UWORD Cl2 = Cluster2;

  /* Get the block that this cluster is in                */
  bp = getblock((ULONG) (((ULONG) Cluster1) * SIZEOF_CLST16) / dpbp->dpb_secsize + dpbp->dpb_fatstrt,
                dpbp->dpb_unit);
#ifdef DISPLAY_GETBLOCK
  printf("FAT (link_fat16)\n");
#endif
  if (bp == NULL)
    return DE_BLKINVLD;
  bp->b_flag &= ~(BFR_DATA | BFR_DIR);
  bp->b_flag |= BFR_FAT;
  bp->b_copies = dpbp->dpb_fats;
  bp->b_offset_lo = dpbp->dpb_fatsize;
  bp->b_offset_hi = dpbp->dpb_fatsize >> 8;

  /* form an index so that we can read the block as a     */
  /* byte array                                           */
  idx = (UWORD)((((LONG) Cluster1) * SIZEOF_CLST16) % dpbp->dpb_secsize);

  /* Finally, put the word into the buffer and mark the   */
  /* buffer as dirty.                                     */
  fputword((WORD FAR *) & Cl2, (VOID FAR *) & (bp->b_buffer[idx]));
  bp->b_flag |= BFR_DIRTY;

  /* Return successful.                                   */
  /* update the free space count                          */
  if (Cluster2 == FREE)
  {
    /* update the free space count for returned     */
    /* cluster                                      */
    if (dpbp->dpb_nfreeclst != UNKNCLSTFREE)
      ++dpbp->dpb_nfreeclst;
  }
  else
  {
    /* update the free space count for removed      */
    /* cluster                                      */
    if (dpbp->dpb_nfreeclst != UNKNCLSTFREE)
      --dpbp->dpb_nfreeclst;
  }

  return SUCCESS;
}

UCOUNT link_fat12(struct dpb FAR *dpbp, UCOUNT Cluster1, UCOUNT Cluster2)
{
  REG UBYTE FAR *fbp0,
    FAR * fbp1;
  UCOUNT idx;
  struct buffer FAR *bp,
    FAR * bp1;

  /* Get the block that this cluster is in                */
  bp = getblock((ULONG) ((((Cluster1 << 1) + Cluster1) >> 1) / dpbp->dpb_secsize + dpbp->dpb_fatstrt),
                dpbp->dpb_unit);
#ifdef DISPLAY_GETBLOCK
  printf("FAT (link_fat12)\n");
#endif
  if (bp == NULL)
    return DE_BLKINVLD;
  bp->b_flag &= ~(BFR_DATA | BFR_DIR);
  bp->b_flag |= BFR_FAT;
  bp->b_copies = dpbp->dpb_fats;
  bp->b_offset_lo = dpbp->dpb_fatsize;
  bp->b_offset_hi = dpbp->dpb_fatsize >> 8;

  /* form an index so that we can read the block as a     */
  /* byte array                                           */
  idx = (((Cluster1 << 1) + Cluster1) >> 1) % dpbp->dpb_secsize;

  /* Test to see if the cluster straddles the block. If   */
  /* it does, get the next block and use both to form the */
  /* the FAT word. Otherwise, just point to the next      */
  /* block.                                               */
  if (idx >= dpbp->dpb_secsize - 1)
  {
    bp1 = getblock((ULONG) (dpbp->dpb_fatstrt +
               ((((Cluster1 << 1) + Cluster1) >> 1) / dpbp->dpb_secsize))
                   + 1,
                   dpbp->dpb_unit);
#ifdef DISPLAY_GETBLOCK
    printf("FAT (link_fat12)\n");
#endif
    if (bp1 == (struct buffer *)0)
      return DE_BLKINVLD;
    bp1->b_flag &= ~(BFR_DATA | BFR_DIR);
    bp1->b_flag |= BFR_FAT | BFR_DIRTY;
    bp1->b_copies = dpbp->dpb_fats;
    bp1->b_offset_lo = dpbp->dpb_fatsize;
    bp1->b_offset_hi = dpbp->dpb_fatsize >> 8;
    fbp1 = (UBYTE FAR *) & (bp1->b_buffer[0]);
  }
  else
    fbp1 = (UBYTE FAR *) & (bp->b_buffer[idx + 1]);
  fbp0 = (UBYTE FAR *) & (bp->b_buffer[idx]);
  bp->b_flag |= BFR_DIRTY;

  /* Now pack the value in                                */
  if (Cluster1 & 0x01)
  {
    *fbp0 = (*fbp0 & 0x0f) | ((Cluster2 & 0x0f) << 4);
    *fbp1 = (Cluster2 >> 4) & 0xff;
  }
  else
  {
    *fbp0 = Cluster2 & 0xff;
    *fbp1 = (*fbp1 & 0xf0) | ((Cluster2 >> 8) & 0x0f);
  }

  /* update the free space count                          */
  if (Cluster2 == FREE)
  {
    /* update the free space count for returned     */
    /* cluster                                      */
    if (dpbp->dpb_nfreeclst != UNKNCLSTFREE)
      ++dpbp->dpb_nfreeclst;
  }
  else
  {
    /* update the free space count for removed      */
    /* cluster                                      */
    if (dpbp->dpb_nfreeclst != UNKNCLSTFREE)
      --dpbp->dpb_nfreeclst;
  }

  return SUCCESS;
}

/* Given the disk parameters, and a cluster number, this function
   looks at the FAT, and returns the next cluster in the clain. */
UWORD next_cluster(struct dpb FAR *dpbp, UCOUNT ClusterNum)
{
  if (ISFAT12(dpbp))
    return next_cl12(dpbp, ClusterNum);
  else if (ISFAT16(dpbp))
    return next_cl16(dpbp, ClusterNum);
  else
    return LONG_LAST_CLUSTER;
}

UWORD next_cl16(struct dpb FAR *dpbp, UCOUNT ClusterNum)
{
  struct buffer FAR *bp;

  /* Get the block that this cluster is in                */
  bp = getblock((ULONG) (((ULONG) ClusterNum) * SIZEOF_CLST16) / dpbp->dpb_secsize + dpbp->dpb_fatstrt,
                dpbp->dpb_unit);
#ifdef DISPLAY_GETBLOCK
  printf("FAT (next_cl16)\n");
#endif
  if (bp == NULL)
    return DE_BLKINVLD;
  bp->b_flag &= ~(BFR_DATA | BFR_DIR);
  bp->b_flag |= BFR_FAT;
  bp->b_copies = dpbp->dpb_fats;
  bp->b_offset_lo = dpbp->dpb_fatsize;
  bp->b_offset_hi = dpbp->dpb_fatsize >> 8;



#ifndef I86
  UCOUNT idx;
  UWORD RetCluster;

  /* form an index so that we can read the block as a     */
  /* byte array                                           */
  idx = (ClusterNum * SIZEOF_CLST16) % dpbp->dpb_secsize;

  /* Get the cluster number,                              */
  
  fgetword((VOID FAR *) & (bp->b_buffer[idx]), (WORD FAR *) & RetCluster);

  /* and return successful.                               */
  return RetCluster;
#else
    /* this saves 2 WORDS of stack :-) */
    
    return *(WORD FAR *)&(bp->b_buffer[(ClusterNum * SIZEOF_CLST16) % dpbp->dpb_secsize]);
    
#endif    
  
}

UWORD next_cl12(struct dpb FAR *dpbp, REG UCOUNT ClusterNum)
{
  REG UBYTE FAR *fbp0,
    FAR * fbp1;
  UCOUNT idx;
  struct buffer FAR *bp,
    FAR * bp1;

  /* Get the block that this cluster is in                */
  bp = getblock((ULONG) ((((ClusterNum << 1) + ClusterNum) >> 1) / dpbp->dpb_secsize + dpbp->dpb_fatstrt),
                dpbp->dpb_unit);
#ifdef DISPLAY_GETBLOCK
  printf("FAT (next_cl12)\n");
#endif
  if (bp == NULL)
    return LONG_BAD;
  bp->b_flag &= ~(BFR_DATA | BFR_DIR);
  bp->b_flag |= BFR_FAT;
  bp->b_copies = dpbp->dpb_fats;
  bp->b_offset_lo = dpbp->dpb_fatsize;
  bp->b_offset_hi = dpbp->dpb_fatsize >> 8;

  /* form an index so that we can read the block as a     */
  /* byte array                                           */
  idx = (((ClusterNum << 1) + ClusterNum) >> 1) % dpbp->dpb_secsize;

  /* Test to see if the cluster straddles the block. If it */
  /* does, get the next block and use both to form the    */
  /* the FAT word. Otherwise, just point to the next      */
  /* block.                                               */
  if (idx >= dpbp->dpb_secsize - 1)
  {
    bp1 = getblock((ULONG) (dpbp->dpb_fatstrt +
           ((((ClusterNum << 1) + ClusterNum) >> 1) / dpbp->dpb_secsize))
                   + 1,
                   dpbp->dpb_unit);
#ifdef DISPLAY_GETBLOCK
    printf("FAT (next_cl12)\n");
#endif
    if (bp1 == (struct buffer *)0)
      return LONG_BAD;
    bp1->b_flag &= ~(BFR_DATA | BFR_DIR);
    bp1->b_flag |= BFR_FAT;
    bp1->b_copies = dpbp->dpb_fats;
    bp1->b_offset_lo = dpbp->dpb_fatsize;
    bp1->b_offset_hi = dpbp->dpb_fatsize >> 8;
    fbp1 = (UBYTE FAR *) & (bp1->b_buffer[0]);
  }
  else
    fbp1 = (UBYTE FAR *) & (bp->b_buffer[idx + 1]);
  fbp0 = (UBYTE FAR *) & (bp->b_buffer[idx]);

  /* Now to unpack the contents of the FAT entry. Odd and */
  /* even bytes are packed differently.                   */
  if (ClusterNum & 0x01)
    ClusterNum = ((*fbp0 & 0xf0) >> 4) | (*fbp1 << 4);
  else
    ClusterNum = *fbp0 | ((*fbp1 & 0x0f) << 8);

  if ((ClusterNum & MASK) == MASK)
    ClusterNum = LONG_LAST_CLUSTER;
  else if ((ClusterNum & BAD) == BAD)
    ClusterNum = LONG_BAD;
  return ClusterNum;
}
