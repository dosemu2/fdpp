/****************************************************************/
/*                                                              */
/*                          blockio.c                           */
/*                            DOS-C                             */
/*                                                              */
/*      Block cache functions and device driver interface       */
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

#include "portab.h"
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *blockioRcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.8  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
 *
 * Revision 1.7  2001/03/21 02:56:25  bartoldeman
 * See history.txt for changes. Bug fixes and HMA support are the main ones.
 *
 * Revision 1.6  2000/10/30 00:32:08  jimtabor
 * Minor Fixes
 *
 * Revision 1.5  2000/10/30 00:21:15  jimtabor
 * Adding Brian Reifsnyder Fix for Int 25/26
 * 2000/9/04   Brian Reifsnyder
 * Modified dskxfer() such that error codes are now returned.
 * Functions that rely on dskxfer() have also been modified accordingly.
 *
 * Revision 1.4  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.3  2000/05/11 04:26:26  jimtabor
 * Added code for DOS FN 69 & 6C
 *
 * Revision 1.2  2000/05/08 04:29:59  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.15  2000/04/29 05:13:16  jtabor
 *  Added new functions and clean up code
 *
 * Revision 1.14  2000/03/09 06:07:10  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.13  1999/08/25 03:18:07  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.12  1999/08/10 18:03:39  jprice
 * ror4 2011-03 patch
 *
 * Revision 1.11  1999/05/03 06:25:45  jprice
 * Patches from ror4 and many changed of signed to unsigned variables.
 *
 * Revision 1.10  1999/05/03 04:55:35  jprice
 * Changed getblock & getbuf so that they leave at least 3 buffer for FAT data.
 *
 * Revision 1.9  1999/04/21 01:44:40  jprice
 * no message
 *
 * Revision 1.8  1999/04/18 05:28:39  jprice
 * no message
 *
 * Revision 1.7  1999/04/16 21:43:40  jprice
 * ror4 multi-sector IO
 *
 * Revision 1.6  1999/04/16 00:53:32  jprice
 * Optimized FAT handling
 *
 * Revision 1.5  1999/04/12 23:41:53  jprice
 * Using getbuf to write data instead of getblock
 * using getblock made it read the block before it wrote it
 *
 * Revision 1.4  1999/04/11 05:28:10  jprice
 * Working on multi-block IO
 *
 * Revision 1.3  1999/04/11 04:33:38  jprice
 * ror4 patches
 *
 * Revision 1.1.1.1  1999/03/29 15:41:43  jprice
 * New version without IPL.SYS
 *
 * Revision 1.5  1999/02/09 02:54:23  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.4  1999/02/01 01:43:27  jprice
 * Fixed findfirst function to find volume label with Windows long filenames
 *
 * Revision 1.3  1999/01/30 08:25:34  jprice
 * Clean up; Fixed bug with set attribute function.  If you tried to
 * change the attributes of a directory, it would erase it.
 *
 * Revision 1.2  1999/01/22 04:15:28  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:00  jprice
 * Imported sources
 *
 *
 *    Rev 1.8   06 Dec 1998  8:43:16   patv
 * Changes in block I/O because of new I/O subsystem.
 *
 *    Rev 1.7   22 Jan 1998  4:09:00   patv
 * Fixed pointer problems affecting SDA
 *
 *    Rev 1.6   04 Jan 1998 23:14:36   patv
 * Changed Log for strip utility
 *
 *    Rev 1.5   03 Jan 1998  8:36:02   patv
 * Converted data area to SDA format
 *
 *    Rev 1.4   16 Jan 1997 12:46:34   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.3   29 May 1996 21:15:10   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.2   01 Sep 1995 17:48:46   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:50:28   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:04:06   patv
 * Initial revision.
 */

/************************************************************************/
/*                                                                      */
/*                      block cache routines                            */
/*                                                                      */
/************************************************************************/
/* #define DISPLAY_GETBLOCK */


/*                                                                      */
/* Initialize the buffer structure                                      */
/*                                                                      */
/* XXX: This should go into `INIT_TEXT'. -- ror4 */

/* this code moved to CONFIG.C 
VOID FAR reloc_call_init_buffers(void)
{
  REG WORD i;
  REG WORD count;

  printf("init_buffers %d buffers at %p\n",Config.cfgBuffers, (void FAR*)&buffers[0]);

  for (i = 0; i < Config.cfgBuffers; ++i)
  {
	struct buffer FAR *pbuffer = &buffers[i];
  	
    pbuffer->b_unit = 0;
    pbuffer->b_flag = 0;
    pbuffer->b_blkno = 0;
    pbuffer->b_copies = 0;
    pbuffer->b_offset_lo = 0;
    pbuffer->b_offset_hi = 0;
    if (i < (Config.cfgBuffers - 1))
      pbuffer->b_next = pbuffer + 1;
    else
      pbuffer->b_next = NULL;
  }
  firstbuf = &buffers[0];
  lastbuf = &buffers[Config.cfgBuffers - 1];
}
*/
/* Extract the block number from a buffer structure. */
#if 0 /*TE*/
ULONG getblkno(struct buffer FAR * bp)
{
  if (bp->b_blkno == 0xffffu)
    return bp->b_huge_blkno;
  else
    return bp->b_blkno;
}
#else
#define getblkno(bp) (bp)->b_blkno
#endif

/*  Set the block number of a buffer structure. (The caller should  */
/*  set the unit number before calling this function.)     */
#if 0 /*TE*/
VOID setblkno(struct buffer FAR * bp, ULONG blkno)
{
  if (blkno >= 0xffffu)
  {
    bp->b_blkno = 0xffffu;
    bp->b_huge_blkno = blkno;
  }
  else
  {
    bp->b_blkno = blkno;
/*    bp->b_dpbp = &blk_devices[bp->b_unit]; */

      bp->b_dpbp = CDSp->cds_table[bp->b_unit].cdsDpb;

  }
}
#else
#define setblkno(bp, blkno) (bp)->b_blkno = (blkno)
#endif

/*
    this searches the buffer list for the given disk/block.
    
    if found, the buffer is returned.
    
    if not found, NULL is returned, and *pReplacebp
    contains a buffer to throw out.
*/    

struct buffer FAR *searchblock(ULONG blkno, COUNT dsk, 
                            struct buffer FAR ** pReplacebp)
{
    int    fat_count = 0;
    struct buffer FAR *bp;
    struct buffer FAR *lbp        = NULL;
    struct buffer FAR *lastNonFat = NULL;

    
#ifdef DISPLAY_GETBLOCK
    printf("[searchblock %d, blk %ld, buf ", dsk, blkno);
#endif
    
    

  /* Search through buffers to see if the required block  */
  /* is already in a buffer                               */

  for (bp = firstbuf; bp != NULL;lbp = bp, bp = bp->b_next)
  {
    if ((getblkno(bp) == blkno)  &&
        (bp->b_flag & BFR_VALID) && (bp->b_unit == dsk))
    {
      /* found it -- rearrange LRU links      */
      if (lbp != NULL)
      {
	    lbp->b_next = bp->b_next;
	    bp->b_next = firstbuf;
	    firstbuf = bp;
      }
#ifdef DISPLAY_GETBLOCK
      printf("HIT %04x:%04x]\n", FP_SEG(bp), FP_OFF(bp));
#endif
      
      return (bp);
    }

    if (bp->b_flag & BFR_FAT)
    	fat_count++;
    else
        lastNonFat = bp;
  }
  
  /*
    now take either the last buffer in chain (not used recently)
    or, if we are low on FAT buffers, the last non FAT buffer
  */  
  
  if (lbp ->b_flag & BFR_FAT && fat_count < 3 && lastNonFat)
  {
    lbp = lastNonFat;  
  }
  *pReplacebp = lbp;
  
#ifdef DISPLAY_GETBLOCK
  printf("MISS, replace %04x:%04x]\n", FP_SEG(lbp), FP_OFF(lbp));
#endif
  

  if (lbp != firstbuf)      /* move to front */
    {
    for (bp = firstbuf; bp->b_next != lbp; bp = bp->b_next)
        ;
    bp->b_next = bp->b_next->b_next;
    lbp->b_next = firstbuf;
    firstbuf = lbp;
    }
  
  return NULL;
}                                


/*                                                                      */
/*      Return the address of a buffer structure containing the         */
/*      requested block.                                                */
/*                                                                      */
/*      returns:                                                        */
/*              requested block with data                               */
/*      failure:                                                        */
/*              returns NULL                                            */
/*                                                                      */
struct buffer FAR *getblock(ULONG blkno, COUNT dsk)
{
    struct buffer FAR *bp;
    struct buffer FAR *Replacebp;



  /* Search through buffers to see if the required block  */
  /* is already in a buffer                               */

    bp = searchblock(blkno, dsk, &Replacebp);
    
    if (bp)
    {
      return (bp);
    }

  /* The block we need is not in a buffer, we must make a buffer  */
  /* available, and fill it with the desired block                */


  /* take the buffer that lbp points to and flush it, then read new block. */
  if (flush1(Replacebp) && fill(Replacebp, blkno, dsk))     /* success             */
  {
    return Replacebp;
  }
  else
  {
    return NULL;
  }
}

/*
   Return the address of a buffer structure for the
   requested block.  This is for writing new data to a block, so
   we really don't care what is in the buffer now.

   returns:
   TRUE = buffer available, flushed if necessary
   parameter is filled with pointer to buffer
   FALSE = there was an error flushing the buffer.
   parameter is set to NULL
 */
BOOL getbuf(struct buffer FAR ** pbp, ULONG blkno, COUNT dsk)
{
    struct buffer FAR *bp;
    struct buffer FAR *Replacebp;

  /* Search through buffers to see if the required block  */
  /* is already in a buffer                               */

    bp = searchblock(blkno, dsk, &Replacebp);
    
    if (bp)
    {
      *pbp = bp;
      return TRUE;
    }      

  /* The block we need is not in a buffer, we must make a buffer  */
  /* available. */

  /* take the buffer than lbp points to and flush it, then make it available. */
  if (flush1(Replacebp))              /* success              */
  {
    Replacebp->b_flag = 0;
    Replacebp->b_unit = dsk;
    setblkno(Replacebp, blkno);
    *pbp = Replacebp;
    return TRUE;
  }
  else
    /* failure              */
  {
    *pbp = NULL;
    return FALSE;
  }
}
/*                                                                      */
/*      Mark all buffers for a disk as not valid                        */
/*                                                                      */
VOID setinvld(REG COUNT dsk)
{
  REG struct buffer FAR *bp;

  bp = firstbuf;
  while (bp)
  {
    if (bp->b_unit == dsk)
      bp->b_flag = 0;
    bp = bp->b_next;
  }
}

/*                                                                      */
/*                      Flush all buffers for a disk                    */
/*                                                                      */
/*      returns:                                                        */
/*              TRUE on success                                         */
/*                                                                      */
BOOL flush_buffers(REG COUNT dsk)
{
  REG struct buffer FAR *bp;
  REG BOOL ok = TRUE;

  bp = firstbuf;
  while (bp)
  {
    if (bp->b_unit == dsk)
      if (!flush1(bp))
	ok = FALSE;
    bp = bp->b_next;
  }
  return ok;
}

/*                                                                      */
/*      Write one disk buffer                                           */
/*                                                                      */
BOOL flush1(struct buffer FAR * bp)
{
/* All lines with changes on 9/4/00 by BER marked below */  
  
  UWORD result;              /* BER 9/4/00 */

  if ((bp->b_flag & BFR_VALID) && (bp->b_flag & BFR_DIRTY))
  {
    result = dskxfer(bp->b_unit, getblkno(bp),
		 (VOID FAR *) bp->b_buffer, 1, DSKWRITE); /* BER 9/4/00  */
    if (bp->b_flag & BFR_FAT)
    {
      int i = bp->b_copies;
      LONG blkno = getblkno(bp);
      UWORD offset = ((UWORD) bp->b_offset_hi << 8) | bp->b_offset_lo;

      while (--i > 0)
      {
	blkno += offset;
	result = dskxfer(bp->b_unit, blkno, 
		     (VOID FAR *) bp->b_buffer, 1, DSKWRITE);  /* BER 9/4/00 */
      }
    }
  }
  else
    result = TRUE; /* This negates any error code returned in result...BER */
  bp->b_flag &= ~BFR_DIRTY;     /* even if error, mark not dirty */
  if (!result)                      /* otherwise system has trouble  */
    bp->b_flag &= ~BFR_VALID;   /* continuing.           */
  return (TRUE);   /* Forced to TRUE...was like this before dskxfer()  */
		   /* returned error codes...BER */
}

/*                                                                      */
/*      Write all disk buffers                                          */
/*                                                                      */
BOOL flush(void)
{
  REG struct buffer FAR *bp;
  REG BOOL ok;

  ok = TRUE;
  bp = firstbuf;
  while (bp)
  {
    if (!flush1(bp))
      ok = FALSE;
    bp->b_flag &= ~BFR_VALID;
    bp = bp->b_next;
  }

  int2f_Remote_call(REM_FLUSHALL, 0, 0, 0, 0, 0, 0);

  return (ok);
}

/*                                                                      */
/*      Fill the indicated disk buffer with the current track and sector */
/*                                                                      */
/* This function assumes that the buffer is ready for use and that the
   sector is not already in the buffer ring  */
BOOL fill(REG struct buffer FAR * bp, ULONG blkno, COUNT dsk)
{
/* Changed 9/4/00 BER */  
  UWORD result;

  result = dskxfer(dsk, blkno, (VOID FAR *) bp->b_buffer, 1, DSKREAD);
/* End of change */  
  bp->b_flag = BFR_VALID | BFR_DATA;
  bp->b_unit = dsk;
  setblkno(bp, blkno);
  
/* Changed 9/4/00 BER */
  if(result==0) return(TRUE);  /* Temporary code to convert the result to */
  else return(FALSE);          /* the old BOOL result...BER               */

  /* return (result);         This is what should eventually be returned */
/* End of change */
}

/************************************************************************/
/*                                                                      */
/*              Device Driver Interface Functions                       */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/* Transfer one or more blocks to/from disk                             */
/*                                                                      */

/* Changed to UWORD  9/4/00  BER */
UWORD dskxfer(COUNT dsk, ULONG blkno, VOID FAR * buf, UWORD numblocks, COUNT mode)
/* End of change */
{
/*  REG struct dpb *dpbp = &blk_devices[dsk]; */

  REG struct dpb FAR *dpbp = CDSp->cds_table[dsk].cdsDpb;

  for (;;)
  {
    IoReqHdr.r_length = sizeof(request);
    IoReqHdr.r_unit = dpbp->dpb_subunit;
    IoReqHdr.r_command =
	mode == DSKWRITE ?
	(verify_ena ? C_OUTVFY : C_OUTPUT)
	: C_INPUT;
    IoReqHdr.r_status = 0;
    IoReqHdr.r_meddesc = dpbp->dpb_mdb;
    IoReqHdr.r_trans = (BYTE FAR *) buf;
    IoReqHdr.r_count = numblocks;
    if (blkno >= MAXSHORT)
    {
      IoReqHdr.r_start = HUGECOUNT;
      IoReqHdr.r_huge = blkno;
    }
    else
      IoReqHdr.r_start = blkno;
    execrh((request FAR *) & IoReqHdr, dpbp->dpb_device);
    if (!(IoReqHdr.r_status & S_ERROR) && (IoReqHdr.r_status & S_DONE))
      break;
    else
    {
/* Changed 9/4/00   BER */    
    return (IoReqHdr.r_status);
    
    /* Skip the abort, retry, fail code...it needs fixed...BER */
/* End of change */

    loop:
      switch (block_error(&IoReqHdr, dpbp->dpb_unit, dpbp->dpb_device))
      {
	case ABORT:
	case FAIL:
	  return (IoReqHdr.r_status);

	case RETRY:
	  continue;

	case CONTINUE:
	  break;

	default:
	  goto loop;
      }
    }
  }
/* *** Changed 9/4/00  BER */
  return 0;   /* Success!  Return 0 for a successful operation. */
/* End of change */

}
