/****************************************************************/
/*                                                              */
/*                            dsk.c                             */
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
static BYTE *dskRcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.16  2001/04/29 17:34:40  bartoldeman
 * A new SYS.COM/config.sys single stepping/console output/misc fixes.
 *
 * Revision 1.15  2001/04/16 01:45:26  bartoldeman
 * Fixed handles, config.sys drivers, warnings. Enabled INT21/AH=6C, printf %S/%Fs
 *
 * Revision 1.14  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
 *
 * Revision 1.13  2001/03/27 20:27:43  bartoldeman
 * dsk.c (reported by Nagy Daniel), inthndlr and int25/26 fixes by Tom Ehlert.
 *
 * Revision 1.12  2001/03/24 22:13:05  bartoldeman
 * See history.txt: dsk.c changes, warning removal and int21 entry handling.
 *
 * Revision 1.11  2001/03/21 02:56:25  bartoldeman
 * See history.txt for changes. Bug fixes and HMA support are the main ones.
 *
 * Revision 1.9  2001/03/08 21:15:00  bartoldeman
 * Space saving fixes from Tom Ehlert
 *
 * Revision 1.8  2000/06/21 18:16:46  jimtabor
 * Add UMB code, patch, and code fixes
 *
 * Revision 1.7  2000/06/01 06:37:38  jimtabor
 * Read History for Changes
 *
 * Revision 1.6  2000/05/26 19:25:19  jimtabor
 * Read History file for Change info
 *
 * Revision 1.5  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.4  2000/05/17 19:15:12  jimtabor
 * Cleanup, add and fix source.
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
 * Revision 1.6  2000/04/29 05:13:16  jtabor
 *  Added new functions and clean up code
 *
 * Revision 1.5  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.4  1999/08/10 18:07:57  jprice
 * ror4 2011-04 patch
 *
 * Revision 1.3  1999/04/16 21:43:40  jprice
 * ror4 multi-sector IO
 *
 * Revision 1.2  1999/04/16 00:53:32  jprice
 * Optimized FAT handling
 *
 * Revision 1.1.1.1  1999/03/29 15:40:51  jprice
 * New version without IPL.SYS
 *
 * Revision 1.5  1999/02/14 04:26:46  jprice
 * Changed check media so that it checks if a floppy disk has been changed.
 *
 * Revision 1.4  1999/02/08 05:55:57  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.3  1999/02/01 01:48:41  jprice
 * Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
 *
 * Revision 1.2  1999/01/22 04:13:25  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *    Rev 1.7   06 Dec 1998  8:45:18   patv
 * Changed due to new I/O subsystem.
 *
 *    Rev 1.6   04 Jan 1998 23:15:16   patv
 * Changed Log for strip utility
 *
 *    Rev 1.5   10 Jan 1997  5:41:48   patv
 * Modified for extended partition support
 *
 *    Rev 1.4   29 May 1996 21:03:32   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.3   19 Feb 1996  3:21:36   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.2   01 Sep 1995 17:54:18   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:52:00   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:32:42   patv
 * Initial revision.
 */

#if defined(DEBUG) 
    #define PartCodePrintf(x) printf x 
#else    
    #define PartCodePrintf(x) 
#endif    

#define STATIC 




#ifdef PROTO
BOOL fl_reset(WORD);
COUNT fl_readdasd(WORD);
COUNT fl_diskchanged(WORD);
COUNT fl_rd_status(WORD);
COUNT fl_read(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
COUNT fl_write(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
COUNT fl_verify(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
BOOL fl_format(WORD, BYTE FAR *);
#else
BOOL fl_reset();
COUNT fl_readdasd();
COUNT fl_diskchanged();
COUNT fl_rd_status();
COUNT fl_read();
COUNT fl_write();
COUNT fl_verify();
BOOL fl_format();
#endif

#define NDEV            8       /* only one for demo            */
#define SEC_SIZE        512     /* size of sector in bytes      */
#define N_RETRY         5       /* number of retries permitted  */
#define NENTRY          26      /* total size of dispatch table */

extern BYTE FAR nblk_rel;


                            /* this buffer must not overlap a 64K boundary
                               due to DMA transfers
                               this is certainly true, if located somewhere
                               at 0xf+1000 and must hold already during BOOT time
                            */   
union
{
  BYTE bytes[1 * SEC_SIZE];
  boot boot_sector;
} DiskTransferBuffer;


STATIC struct media_info
{
  ULONG mi_size;                /* physical sector count        */
  UWORD mi_heads;               /* number of heads (sides)      */
  UWORD mi_cyls;                /* number of cyl/drive          */
  UWORD mi_sectors;             /* number of sectors/cyl        */
  ULONG mi_offset;              /* relative partition offset    */
  BYTE mi_drive;                /* BIOS drive number            */
  ULONG mi_FileOC;              /* Count of Open files on Drv   */
  
  UWORD mi_BeginCylinder;
  BYTE  mi_BeginHead;
  BYTE  mi_BeginSector;

  struct FS_info
    {
    ULONG serialno;
    BYTE  volume[11];
    BYTE  fstype[8];
    }fs;
};



static struct Access_info
{
  BYTE  AI_spec;
  BYTE  AI_Flag;
};

STATIC struct media_info miarray[NDEV]; /* Internal media info structs  */
STATIC bpb bpbarray[NDEV];      /* BIOS parameter blocks        */
STATIC bpb *bpbptrs[NDEV];      /* pointers to bpbs             */

/*TE - array access functions */
struct media_info *getPMiarray(int dev) { return &miarray[dev];}
       bpb        *getPBpbarray(unsigned dev){ return &bpbarray[dev];}

#define N_PART 4                /* number of partitions per
                                   table partition              */

STATIC COUNT nUnits;            /* number of returned units     */

#define PARTOFF 0x1be


#ifdef PROTO
WORD _dsk_init(rqptr),
  mediachk(rqptr),
  bldbpb(rqptr),
  blockio(rqptr),
  IoctlQueblk(rqptr),
  Genblkdev(rqptr),
  Getlogdev(rqptr),
  Setlogdev(rqptr),
  blk_Open(rqptr),
  blk_Close(rqptr),
  blk_Media(rqptr),
  blk_noerr(rqptr),
  blk_nondr(rqptr),
  blk_error(rqptr);
COUNT ltop(WORD *, WORD *, WORD *, COUNT, COUNT, ULONG, byteptr);
WORD dskerr(COUNT);
COUNT processtable(int table_type,COUNT ptDrive, BYTE ptHead, UWORD ptCylinder, BYTE ptSector, LONG ptAccuOff, UWORD PartitionDone);
#else
WORD _dsk_init(),
  mediachk(),
  bldbpb(),
  blockio(),
  IoctlQueblk(),
  Genblkdev(),
  Getlogdev(),
  Setlogdev(),
  blk_Open(),
  blk_Close(),
  blk_Media(),
  blk_noerr(),
  blk_nondr(),
  blk_error();
WORD dskerr();
COUNT processtable();
#endif

/*                                                                      */
/* the function dispatch table                                          */
/*                                                                      */

#ifdef PROTO
static WORD(*dispatch[NENTRY]) (rqptr) =
#else
static WORD(*dispatch[NENTRY]) () =
#endif
{
      _dsk_init,                     /* Initialize                   */
      mediachk,                 /* Media Check                  */
      bldbpb,                   /* Build BPB                    */
      blk_error,                /* Ioctl In                     */
      blockio,                  /* Input (Read)                 */
      blk_nondr,                /* Non-destructive Read         */
      blk_noerr,                /* Input Status                 */
      blk_noerr,                /* Input Flush                  */
      blockio,                  /* Output (Write)               */
      blockio,                  /* Output with verify           */
      blk_noerr,                /* Output Status                */
      blk_noerr,                /* Output Flush                 */
      blk_error,                /* Ioctl Out                    */
      blk_Open,                 /* Device Open                  */
      blk_Close,                /* Device Close                 */
      blk_Media,                /* Removable Media              */
      blk_noerr,                /* Output till busy             */
      blk_error,                /* undefined                    */
      blk_error,                /* undefined                    */
      Genblkdev,                /* Generic Ioctl Call           */
      blk_error,                /* undefined                    */
      blk_error,                /* undefined                    */
      blk_error,                /* undefined                    */
      Getlogdev,                /* Get Logical Device           */
      Setlogdev,                /* Set Logical Device           */
      IoctlQueblk               /* Ioctl Query                  */
};

#define SIZEOF_PARTENT  16

#define PRIMARY         0x01
#define PRIMARY2        0x02

#define FAT12           0x01
#define FAT16SMALL      0x04
#define EXTENDED        0x05
#define FAT16LARGE      0x06
#define EXTENDED_INT32  0x0f  /* like 0x05, but uses extended INT32 */

#define hd(x)   ((x) & 0x80)



ULONG StartSector(WORD ptDrive,     unsigned  BeginCylinder,
                                    unsigned BeginHead, 
                                    unsigned BeginSector,   
                                    ULONG    peStartSector,
                                    ULONG    ptAccuOff)
{
        iregs regs;
        
        unsigned heads,sectors;
        ULONG startPos;
        ULONG oldStartPos;
        
        regs.a.x = 0x0800;    /* get drive parameters */
        regs.d.x = ptDrive;
        intr(0x13, &regs);
        
        if ((regs.a.x & 0xff) != 0)
            {
            PartCodePrintf(("error getting drive parameters for drive %x\n", ptDrive));
            return peStartSector+ptAccuOff;
            }
            
        /* cylinders = (regs.c.x >>8) | ((regs.c.x & 0x0c) << 2); */
        heads     = (regs.d.x >> 8) + 1; 
        sectors   = regs.c.x & 0x3f;
        
        startPos = ((ULONG)BeginCylinder * heads + BeginHead) * sectors + BeginSector - 1;

        PartCodePrintf((" CHS %x %x %x (%d %d %d) --> %lx ( %ld)\n",
                             BeginCylinder, BeginHead, BeginSector,
                             BeginCylinder, BeginHead, BeginSector,
                             startPos, startPos));

                             
        oldStartPos = peStartSector + ptAccuOff;
          
        PartCodePrintf(("oldStartPos = %lx - ", oldStartPos));
    
                                        
        if (startPos != oldStartPos)
          {
          PartCodePrintf(("PART TABLE mismatch for drive %x, CHS=%d %d %d, startsec %d, offset %ld\n",
                        ptDrive, BeginCylinder, BeginHead,BeginSector,   
                        peStartSector, ptAccuOff));
                        
          PartCodePrintf((" old startpos = %ld, new startpos = %ld, using new\n", 
                oldStartPos, startPos));
          }                                      
        
        return startPos;
}                                    

/* 
    thats what MSDN says:

    How Windows 2000 Assigns, Reserves, and Stores Drive Letters
    ID: q234048 
 
  BASIC Disk - Drive Letter Assignment Rules
The following are the basic disk drive letter assignment rules for Windows 2000: 
Scan all fixed hard disks as they are enumerated, assign drive letters 
starting with any active primary partitions (if there is one), otherwise,
scan the first primary partition on each drive. Assign next available 
letter starting with C:


Repeat scan for all fixed hard disks and removable (JAZ, MO) disks 
and assign drive letters to all logical drives in an extended partition, 
or the removable disk(s) as enumerated. Assign next available letter 
starting with C: 


Finally, repeat scan for all fixed hard disk drives, and assign drive 
letters to all remaining primary partitions. Assign next available letter 
starting with C:

Floppy drives. Assign letter starting with A:

CD-ROM drives. Assign next available letter starting with D:

*************************************************************************
Order in Which MS-DOS and Windows Assign Drive Letters
ID: q51978 
 
MORE INFORMATION
The following occurs at startup: 

MS-DOS checks all installed disk devices, assigning the drive letter A 
to the first physical floppy disk drive that is found.

If a second physical floppy disk drive is present, it is assigned drive letter B. If it is not present, a logical drive B is created that uses the first physical floppy disk drive.


Regardless of whether a second floppy disk drive is present, 
MS-DOS then assigns the drive letter C to the primary MS-DOS 
partition on the first physical hard disk, and then goes on 
to check for a second hard disk. 


If a second physical hard disk is found, and a primary partition exists 
on the second physical drive, the primary MS-DOS partition on the second
physical hard drive is assigned the letter D. MS-DOS version 5.0, which 
supports up to eight physical drives, will continue to search for more 
physical hard disk drives at this point. For example, if a third physical 
hard disk is found, and a primary partition exists on the third physical 
drive, the primary MS-DOS partition on the third physical hard drive is 
assigned the letter E.


MS-DOS returns to the first physical hard disk drive and assigns drive 
letters to any additional logical drives (in extended MS-DOS partitions) 
on that drive in sequence.


MS-DOS repeats this process for the second physical hard disk drive, 
if present. MS-DOS 5.0 will repeat this process for up to eight physical 
hard drives, if present. After all logical drives (in extended MS-DOS 
partitions) have been assigned drive letters, MS-DOS 5.0 returns to 
the first physical drive and assigns drive letters to any other primary 
MS-DOS partitions that exist, then searches other physical drives for 
additional primary MS-DOS partitions. This support for multiple primary 
MS-DOS partitions was added to version 5.0 for backward compatibility 
with the previous OEM MS-DOS versions that support multiple primary partitions.


After all logical drives on the hard disk(s) have been assigned drive 
letters, drive letters are assigned to drives installed using DRIVER.SYS 
or created using RAMDRIVE.SYS in the order in which the drivers are loaded 
in the CONFIG.SYS file. Which drive letters are assigned to which devices 
can be influenced by changing the order of the device drivers or, if necessary, 
by creating "dummy" drive letters with DRIVER.SYS.

********************************************************

I don't know, if I did it right, but I tried to do it that way. TE

*/
COUNT processtable(int table_type,COUNT ptDrive, BYTE ptHead, UWORD ptCylinder,
                   BYTE ptSector, LONG ptAccuOff, UWORD PartitionDone )
{
  struct                        /* Temporary partition table    */
  {
    BYTE peBootable;
    BYTE peBeginHead;
    BYTE peBeginSector;
    UWORD peBeginCylinder;
    BYTE peFileSystem;
    BYTE peEndHead;
    BYTE peEndSector;
    UWORD peEndCylinder;
    LONG peStartSector;
    LONG peSectors;
  }
  temp_part[N_PART], 
  	*ptemp_part;			/*TE*/

  int retry;
  UBYTE packed_byte,
    pb1;
  BYTE *p;
  int partition_chain = 0;
  int ret;
/*  ULONG newStartPos;*/
  UWORD partMask;
  int loop;
  
restart:                    /* yes, it's a GOTO >:-) */

                            /* if someone has a circular linked 
                                extended partition list, stop it sooner or later */
    if (partition_chain > 64)
        return PartitionDone;

  
    PartCodePrintf(("searching partition table at %x %x %x %x %lx\n", 
         ptDrive,  ptCylinder, ptHead, ptSector, ptAccuOff));

    /* Read partition table                         */
    for ( retry = N_RETRY; --retry >= 0; )
    {
    ret = fl_read((WORD) ptDrive, (WORD) ptHead, (WORD) ptCylinder,
                  (WORD) ptSector, (WORD) 1, (byteptr) & DiskTransferBuffer);
    if (ret == 0)
        break;                  
    }
    if (ret != 0)
        return PartitionDone;

  /* Read each partition into temporary array     */
  
    p = (BYTE *) & DiskTransferBuffer.bytes[PARTOFF];
  
    for (ptemp_part = &temp_part[0];
       ptemp_part < &temp_part[N_PART]; ptemp_part++)
    {

        getbyte((VOID *) (p+0), &ptemp_part->peBootable);
        getbyte((VOID *) (p+1), &ptemp_part->peBeginHead);
        getbyte((VOID *) (p+2), &packed_byte);
        ptemp_part->peBeginSector = packed_byte & 0x3f;
        getbyte((VOID *) (p+3), &pb1);
        ptemp_part->peBeginCylinder = pb1 + ((UWORD) (0xc0 & packed_byte) << 2);
        getbyte((VOID *) (p+4), &ptemp_part->peFileSystem);
        getbyte((VOID *) (p+5), &ptemp_part->peEndHead);
        getbyte((VOID *) (p+6), &packed_byte);
        ptemp_part->peEndSector = packed_byte & 0x3f;
        getbyte((VOID *) (p+7), &pb1);
        ptemp_part->peEndCylinder = pb1 + ((UWORD) (0xc0 & packed_byte) << 2);
        getlong((VOID *) (p+8), &ptemp_part->peStartSector);
        getlong((VOID *) (p+12), &ptemp_part->peSectors);
        
        p += SIZEOF_PARTENT; /* == 16 */
    }

  /* Walk through the table, add DOS partitions to global
     array and process extended partitions         */
     
                                    /* when searching the EXT chain, 
                                       must skip primary partitions */  	
    if (   table_type==PRIMARY  ||
         table_type==PRIMARY2 ||
         partition_chain!=0   ) 
    {
                                    /* do this for 
                                        0) all active partitions
                                        1) the first primary partition
                                    */
        for (loop = 0; loop < 2; loop++)
        {     
            for (ptemp_part = &temp_part[0], partMask = 1;
                ptemp_part < &temp_part[N_PART] && nUnits < NDEV; 
                partMask <<= 1,ptemp_part++)
            {
        
        
            if (loop == 0 &&           /* scan for only for active */
                !ptemp_part->peBootable) 
                {
                continue;
                }
            
            if (PartitionDone & partMask)    /* don't reassign partitions */
                {
                continue;    
                }
    
            if  ( ptemp_part->peFileSystem == FAT12      ||
            	  ptemp_part->peFileSystem == FAT16SMALL ||
        	      ptemp_part->peFileSystem == FAT16LARGE  )
            {
                  
                  struct media_info *pmiarray = getPMiarray(nUnits);
        
                  pmiarray->mi_drive   = ptDrive;
                  	
                  pmiarray->mi_offset  = StartSector(ptDrive, 
                                                ptemp_part->peBeginCylinder, 
                                                ptemp_part->peBeginHead,
                                                ptemp_part->peBeginSector,   
                                                ptemp_part->peStartSector,
                                                ptAccuOff);
                                                
                  pmiarray->mi_BeginCylinder = ptemp_part->peBeginCylinder;
                  pmiarray->mi_BeginHead     = ptemp_part->peBeginHead;
                  pmiarray->mi_BeginSector   = ptemp_part->peBeginSector;
                                                
                  
                  nUnits++;
            
                  
                  PartitionDone |= partMask;
                  
                  if (   table_type==PRIMARY  )
                  {
                        return PartitionDone;        /* we are done */
                  }
        
                } /* end FAT16 detected */
            } /* end PArtitionentry 0..3 */
        }  /* end of loop 0..1 for active, nonactive */  
    } /* if (   table_type==PRIMARY ) */
  
                                /* search for EXT partitions only on 2. run */
    if (table_type==EXTENDED)
    {
        for (ptemp_part = &temp_part[0];
           ptemp_part < &temp_part[N_PART] && nUnits < NDEV; ptemp_part++)
        {
            if ( (ptemp_part->peFileSystem == EXTENDED ||
                  ptemp_part->peFileSystem == EXTENDED_INT32 ) )
            {
              /* restart with new extended part table, don't recurs */
                partition_chain++;
              
                ptHead = ptemp_part->peBeginHead;
                ptCylinder = ptemp_part->peBeginCylinder;
                ptSector = ptemp_part->peBeginSector;
                ptAccuOff = ptemp_part->peStartSector + ptAccuOff;
                
                PartitionDone = 0; /* not important for EXTENDED */
                
                goto restart;
            }
        }          
    }
  
  return PartitionDone;
}

COUNT FAR blk_driver(rqptr rp)
{
  if (rp->r_unit >= nUnits && rp->r_command != C_INIT)
    return failure(E_UNIT);
  if (rp->r_command > NENTRY)
  {
    return failure(E_FAILURE);  /* general failure */
  }
  else
    return ((*dispatch[rp->r_command]) (rp));
}

WORD _dsk_init(rqptr rp)
{
  extern COUNT fl_nrdrives(VOID);
  COUNT HardDrive,
    nHardDisk,
    Unit;
  struct media_info *pmiarray;
  bpb *pbpbarray;
  UBYTE foundPartitions[16];
    

  /* Reset the drives                                             */
  fl_reset(0x80);

  /* Initial number of disk units                                 */
  nUnits = 2;

  /* Setup media info and BPBs arrays                             */
  for (Unit = 0; Unit < NDEV; Unit++)
  {
  	pmiarray = getPMiarray(Unit);
  	
    pmiarray->mi_size = 720l;
    pmiarray->mi_heads = 2;
    pmiarray->mi_cyls = 40;
    pmiarray->mi_sectors = 9;
    pmiarray->mi_offset = 0l;
    pmiarray->mi_drive = Unit;
    
    pmiarray->mi_BeginCylinder = 0;     /* initialize for floppies */
    pmiarray->mi_BeginHead     = 0;
    pmiarray->mi_BeginSector   = 1;
    

    pmiarray->fs.serialno = 0x12345678l;

	pbpbarray = getPBpbarray(Unit);    

    pbpbarray->bpb_nbyte = SEC_SIZE;
    pbpbarray->bpb_nsector = 2;
    pbpbarray->bpb_nreserved = 1;
    pbpbarray->bpb_nfat = 2;
    pbpbarray->bpb_ndirent = 112;
    pbpbarray->bpb_nsize = 720l;
    pbpbarray->bpb_mdesc = 0xfd;
    pbpbarray->bpb_nfsect = 2;

    bpbptrs[Unit] = pbpbarray;
  }

  nHardDisk = fl_nrdrives();

  nHardDisk = min(nHardDisk,sizeof(foundPartitions));
  
  /* as rather well documented, DOS searches 1st) 1 primary patitions on 
     all drives, 2nd) all extended partitions. that  
     makes many people (including me) unhappy, as all DRIVES D:,E:... 
     on 1st disk will move up/down, if other disk with 
     primary partitions are added/removed, but
     thats the way it is (hope I got it right) 
     TE (with a little help from my friends) 
     see also above for WIN2000,DOS,MSDN */
     
  PartCodePrintf(("DSK init: found %d disk drives\n",nHardDisk)); 
     
  
    /* Process primary partition table   1 partition only      */
  for (HardDrive = 0; HardDrive < nHardDisk; HardDrive++)
  {
    foundPartitions[HardDrive] = 
            processtable(PRIMARY, (HardDrive | 0x80), 0, 0l, 1, 0l,0);
  }
    /* Process extended partition table                      */
  for (HardDrive = 0; HardDrive < nHardDisk; HardDrive++)
  {
    processtable(EXTENDED, (HardDrive | 0x80), 0, 0l, 1, 0l,0);
  }

    /* Process primary a 2nd time */
  for (HardDrive = 0; HardDrive < nHardDisk; HardDrive++)
  {
    processtable(PRIMARY2, (HardDrive | 0x80), 0, 0l, 1, 0l,foundPartitions[HardDrive]);
  }

  rp->r_nunits = nUnits;
  rp->r_bpbptr = bpbptrs;
  rp->r_endaddr = device_end();
  nblk_rel = nUnits;            /* make device header reflect units */
  return S_DONE;
}

static WORD mediachk(rqptr rp)
{
  COUNT drive = miarray[rp->r_unit].mi_drive;
  COUNT result;

  /* if it's a hard drive, media never changes */
  if (hd(drive))
    rp->r_mcretcode = M_NOT_CHANGED;
  else
    /* else, check floppy status */
  {
    if ((result = fl_readdasd(drive)) == 2)	/* if we can detect a change ... */
    {
      if ((result = fl_diskchanged(drive)) == 1) /* check if it has changed... */
        rp->r_mcretcode = M_CHANGED;
      else if (result == 0)
        rp->r_mcretcode = M_NOT_CHANGED;
      else
        rp->r_mcretcode = tdelay((LONG) 37) ? M_DONT_KNOW : M_NOT_CHANGED;
    }
    else if (result == 3)       /* if it's a fixed disk, then no change */
      rp->r_mcretcode = M_NOT_CHANGED;
    else                        /* can not detect or error... */
      rp->r_mcretcode = tdelay((LONG) 37) ? M_DONT_KNOW : M_NOT_CHANGED;
  }

  return S_DONE;
}

/*
 *  Read Write Sector Zero or Hard Drive Dos Bpb
 */
STATIC WORD RWzero(rqptr rp, WORD t)
{
  REG retry = N_RETRY;
  WORD ret;
  struct media_info *pmiarray = getPMiarray(rp->r_unit);

  do
  {
    ret = (t == 0 ? fl_read : fl_write)(
                  (WORD) pmiarray->mi_drive,
                  (WORD) pmiarray->mi_BeginHead,
                  (WORD) pmiarray->mi_BeginCylinder,
                  (WORD) pmiarray->mi_BeginSector, 
                  (WORD) 1, (byteptr) & DiskTransferBuffer);
  }
  while (ret != 0 && --retry > 0);
  return ret;
}

/*
   0 if not set, 1 = a, 2 = b, etc, assume set.
   page 424 MS Programmer's Ref.
 */
static WORD Getlogdev(rqptr rp)
{
    BYTE x = rp->r_unit;
    x++;
    if( x > nblk_rel )
        return failure(E_UNIT);

    rp->r_unit = x;
    return S_DONE;
}

static WORD Setlogdev(rqptr rp)
{
	UNREFERENCED_PARAMETER(rp);
    return S_DONE;
}

static WORD blk_Open(rqptr rp)
{
    miarray[rp->r_unit].mi_FileOC++;
    return S_DONE;
}

static WORD blk_Close(rqptr rp)
{
   miarray[rp->r_unit].mi_FileOC--;
   return S_DONE;
}

static WORD blk_nondr(rqptr rp)
{
	UNREFERENCED_PARAMETER(rp);
    return S_BUSY|S_DONE;
}

static WORD blk_Media(rqptr rp)
{
  if (hd( miarray[rp->r_unit].mi_drive))
    return S_BUSY|S_DONE;	/* Hard Drive */
  else
    return S_DONE;      	/* Floppy */
}

STATIC WORD bldbpb(rqptr rp)
{
  ULONG count;
  bpb *pbpbarray;
  struct media_info *pmiarray;
  WORD head,/*track,*/sector,ret;

  ret = RWzero( rp, 0);

  if (ret != 0)
    return (dskerr(ret));

/*TE ~ 200 bytes*/    
  pbpbarray = getPBpbarray(rp->r_unit);

  getword(&((((BYTE *) & DiskTransferBuffer.bytes[BT_BPB]))[BPB_NBYTE]), &pbpbarray->bpb_nbyte);
  getbyte(&((((BYTE *) & DiskTransferBuffer.bytes[BT_BPB]))[BPB_NSECTOR]), &pbpbarray->bpb_nsector);
  getword(&((((BYTE *) & DiskTransferBuffer.bytes[BT_BPB]))[BPB_NRESERVED]), &pbpbarray->bpb_nreserved);
  getbyte(&((((BYTE *) & DiskTransferBuffer.bytes[BT_BPB]))[BPB_NFAT]), &pbpbarray->bpb_nfat);
  getword(&((((BYTE *) & DiskTransferBuffer.bytes[BT_BPB]))[BPB_NDIRENT]), &pbpbarray->bpb_ndirent);
  getword(&((((BYTE *) & DiskTransferBuffer.bytes[BT_BPB]))[BPB_NSIZE]), &pbpbarray->bpb_nsize);
  getword(&((((BYTE *) & DiskTransferBuffer.bytes[BT_BPB]))[BPB_NSIZE]), &pbpbarray->bpb_nsize);
  getbyte(&((((BYTE *) & DiskTransferBuffer.bytes[BT_BPB]))[BPB_MDESC]), &pbpbarray->bpb_mdesc);
  getword(&((((BYTE *) & DiskTransferBuffer.bytes[BT_BPB]))[BPB_NFSECT]), &pbpbarray->bpb_nfsect);
  getword(&((((BYTE *) & DiskTransferBuffer.bytes[BT_BPB]))[BPB_NSECS]), &pbpbarray->bpb_nsecs);
  getword(&((((BYTE *) & DiskTransferBuffer.bytes[BT_BPB]))[BPB_NHEADS]), &pbpbarray->bpb_nheads);
  getlong(&((((BYTE *) & DiskTransferBuffer.bytes[BT_BPB])[BPB_HIDDEN])), &pbpbarray->bpb_hidden);
  getlong(&((((BYTE *) & DiskTransferBuffer.bytes[BT_BPB])[BPB_HUGE])), &pbpbarray->bpb_huge);


  pmiarray = getPMiarray(rp->r_unit);

/* Needs fat32 offset code */

  getlong(&((((BYTE *) & DiskTransferBuffer.bytes[0x27])[0])), &pmiarray->fs.serialno);

  memcpy(pmiarray->fs.volume,&DiskTransferBuffer.bytes[0x2B], 11);
  memcpy(pmiarray->fs.fstype,&DiskTransferBuffer.bytes[0x36], 8);



#ifdef DSK_DEBUG
  printf("BPB_NBYTE     = %04x\n", pbpbarray->bpb_nbyte);
  printf("BPB_NSECTOR   = %02x\n", pbpbarray->bpb_nsector);
  printf("BPB_NRESERVED = %04x\n", pbpbarray->bpb_nreserved);
  printf("BPB_NFAT      = %02x\n", pbpbarray->bpb_nfat);
  printf("BPB_NDIRENT   = %04x\n", pbpbarray->bpb_ndirent);
  printf("BPB_NSIZE     = %04x\n", pbpbarray->bpb_nsize);
  printf("BPB_MDESC     = %02x\n", pbpbarray->bpb_mdesc);
  printf("BPB_NFSECT    = %04x\n", pbpbarray->bpb_nfsect);
#endif
  rp->r_bpptr = pbpbarray;

  
  count = pmiarray->mi_size =
      pbpbarray->bpb_nsize == 0 ?
      pbpbarray->bpb_huge :
      pbpbarray->bpb_nsize;
  getword((&(((BYTE *) & DiskTransferBuffer.bytes[BT_BPB])[BPB_NHEADS])), &pmiarray->mi_heads);
  head = pmiarray->mi_heads;
  getword((&(((BYTE *) & DiskTransferBuffer.bytes[BT_BPB])[BPB_NSECS])), &pmiarray->mi_sectors);
  if (pmiarray->mi_size == 0)
    getlong(&((((BYTE *) & DiskTransferBuffer.bytes[BT_BPB])[BPB_HUGE])), &pmiarray->mi_size);
  sector = pmiarray->mi_sectors;

  if (head == 0 || sector == 0)
  {
    tmark();
    return failure(E_FAILURE);
  }
  pmiarray->mi_cyls = count / (head * sector);
  tmark();

#ifdef DSK_DEBUG
  printf("BPB_NSECS     = %04x\n", sector);
  printf("BPB_NHEADS    = %04x\n", head);
  printf("BPB_HIDDEN    = %08lx\n", pbpbarray->bpb_hidden);
  printf("BPB_HUGE      = %08lx\n", pbpbarray->bpb_huge);
#endif
  return S_DONE;
}

STATIC COUNT write_and_verify(WORD drive, WORD head, WORD track, WORD sector,
                              WORD count, BYTE FAR * buffer)
{
  REG COUNT ret;

  ret = fl_write(drive, head, track, sector, count, buffer);
  if (ret != 0)
    return ret;
  return fl_verify(drive, head, track, sector, count, buffer);
}

static WORD IoctlQueblk(rqptr rp)
{
    switch(rp->r_count){
        case 0x0846:
        case 0x0847:
        case 0x0860:
        case 0x0866:
        case 0x0867:
            break;
        default:
            return failure(E_CMD);
    }
  return S_DONE;

}

static WORD Genblkdev(rqptr rp)
{
    int ret;
    struct media_info *pmiarray = getPMiarray(rp->r_unit);
    
    switch(rp->r_count){
        case 0x0860:            /* get device parameters */
        {
        struct gblkio FAR * gblp = (struct gblkio FAR *) rp->r_trans;
        REG COUNT x = 5,y = 1,z = 0;

        if (!hd(pmiarray->mi_drive)){
            y = 2;
	        x = 8;      /* any odd ball drives return this */
            if (pmiarray->mi_size <= 0xffff)
              switch((UWORD)pmiarray->mi_size)
              {
                case 640:
                case 720:      /* 320-360 */
                    x = 0;
                    z = 1;
                break;
                case 1440:     /* 720 */
                    x = 2;
                break;
                case 2400:     /* 1.2 */
                    x = 1;
                break;
                case 2880:     /* 1.44 */
                    x = 7;
                break;
                case 5760:     /* 2.88 almost forgot this one*/
                    x = 9;
                break;
              }
        }
        gblp->gbio_devtype = (UBYTE) x;
        gblp->gbio_devattrib = (UWORD) y;
        gblp->gbio_media = (UBYTE) z;
        gblp->gbio_ncyl = pmiarray->mi_cyls;
        fmemcpy(&gblp->gbio_bpb, &bpbarray[rp->r_unit], sizeof(gblp->gbio_bpb));
        gblp->gbio_nsecs = bpbarray[rp->r_unit].bpb_nsector;
        break;
        }
        case 0x0866:        /* get volume serial number */
        {
        struct Gioc_media FAR * gioc = (struct Gioc_media FAR *) rp->r_trans;
        struct FS_info FAR * fs = &pmiarray->fs;

            gioc->ioc_serialno = fs->serialno;
            fmemcpy(gioc->ioc_volume,fs->volume,11);
            fmemcpy(gioc->ioc_fstype, fs->fstype,8);
        }
        break;
        case 0x0846:        /* set volume serial number */
        {
        struct Gioc_media FAR * gioc = (struct Gioc_media FAR *) rp->r_trans;
        struct FS_info FAR * fs = (struct FS_info FAR *) &DiskTransferBuffer.bytes[0x27];

            ret = RWzero( rp, 0);
            if (ret != 0)
                return (dskerr(ret));

            fs->serialno =  gioc->ioc_serialno;
            pmiarray->fs.serialno = fs->serialno;

            ret = RWzero( rp, 1);
            if (ret != 0)
                return (dskerr(ret));
        }
        break;
        case 0x0867:        /* get access flag, always on*/
        {
        struct Access_info FAR * ai = (struct Access_info FAR *) rp->r_trans;
        ai->AI_Flag = 1;
        }
        break;
        case 0x0847:        /* set access flag, no real use*/
        break;
        default:
            return failure(E_CMD);
    }
  return S_DONE;
}

WORD blockio(rqptr rp)
{
  REG retry = N_RETRY,
    remaining;
  UWORD cmd,
    total;
  ULONG start;
  byteptr trans;
  WORD head,track,sector,ret,count;
  
  COUNT(*action) (WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
  
  struct media_info *pmiarray = getPMiarray(rp->r_unit);
  

  cmd = rp->r_command;
  total = 0;
  trans = rp->r_trans;
  tmark();
  remaining = rp->r_count;
  start = (rp->r_start != HUGECOUNT ? rp->r_start : rp->r_huge)
        + pmiarray->mi_offset;
  while(remaining > 0)
  {
    count = ltop(&track, &sector, &head, rp->r_unit, remaining, start, trans);
    
    /*printf("dskAction %02x THS=%x-%x-%x block=%lx\n", rp->r_unit,track, head, sector, start);*/

    
    do
    {
      switch (cmd)
      {
        case C_INPUT:
          action = fl_read;
          break;
        case C_OUTPUT:
          action = fl_write;
          break;
        case C_OUTVFY:
          action = write_and_verify;
          break;
        default:
          return failure(E_FAILURE);
      }
      
      
      if (count && FP_SEG(trans) != 0xffff)
        {    
        ret = action((WORD) pmiarray->mi_drive, head, track, sector,
                     count, trans);
        }                     
      else
      {
        count = 1;
        /* buffer crosses DMA boundary, use scratchpad  */
        /* use scratchpad also, if going to HIGH memory */

        if (cmd != C_INPUT)
          fbcopy(trans, &DiskTransferBuffer, SEC_SIZE);
        ret = action((WORD) pmiarray->mi_drive, head, track, sector,
                     1, (byteptr)&DiskTransferBuffer);
        if (cmd == C_INPUT)
          fbcopy(&DiskTransferBuffer, trans, SEC_SIZE);
      }
      if (ret != 0)
        fl_reset((WORD) pmiarray->mi_drive);
    }
    while (ret != 0 && --retry > 0);
    if (ret != 0)
    {
      rp->r_count = total;
      return dskerr(ret);
    }
    total += count;
    remaining -= count;
    trans += count * SEC_SIZE;
    start += count;
  }
  rp->r_count = total;
  return S_DONE;
}

static WORD blk_error(rqptr rp)
{
  rp->r_count = 0;
  return failure(E_FAILURE);    /* general failure */
}


static WORD blk_noerr(rqptr rp)
{
	UNREFERENCED_PARAMETER(rp);
    return S_DONE;
}

static WORD dskerr(COUNT code)
{
/*      printf("diskette error:\nhead = %d\ntrack = %d\nsector = %d\ncount = %d\n",
   head, track, sector, count); */
  switch (code & 0x03)
  {
    case 1:                    /* invalid command - general failure */
      if (code & 0x08)
        return (E_FAILURE);
      else
        return failure(E_CMD);

    case 2:                    /* address mark not found - general  failure */
      return failure(E_FAILURE);

    case 3:                    /* write protect */
      return failure(E_WRPRT);

    default:
      if (code & 0x80)          /* time-out */
        return failure(E_NOTRDY);
      else if (code & 0x40)     /* seek error */
        return failure(E_SEEK);
      else if (code & 0x10)     /* CRC error */
        return failure(E_CRC);
      else if (code & 0x04)
        return failure(E_NOTFND);
      else
        return failure(E_FAILURE);
  }
}

/*                                                                      */
/* Do logical block number to physical head/track/sector mapping        */
/*                                                                      */
COUNT ltop(WORD * trackp, WORD * sectorp, WORD * headp, COUNT unit, COUNT count, ULONG strt_sect, byteptr strt_addr)
{
#ifdef I86
  UWORD utemp;
#endif
	struct media_info *pmiarray;
	

#ifdef I86
/*TE*/
  /* Adjust for segmented architecture                            */
  utemp = (((UWORD) mk_segment(strt_addr) << 4) + mk_offset(strt_addr));
  /* Test for 64K boundary crossing and return count large        */
  /* enough not to exceed the threshold.                          */
  
#define SEC_SHIFT 9	/* = 0x200 = 512 */  
  
  utemp >>= SEC_SHIFT;
  
  if (count > (0xffff >> SEC_SHIFT) - utemp)
  	{
  	count = (0xffff >> SEC_SHIFT) - utemp;
  	}

#endif

/*TE*/
  pmiarray = getPMiarray(unit);

  *trackp = strt_sect / (pmiarray->mi_heads * pmiarray->mi_sectors);
  *sectorp = strt_sect % pmiarray->mi_sectors + 1;
  *headp = (strt_sect % (pmiarray->mi_heads * pmiarray->mi_sectors))
      / pmiarray->mi_sectors;
  if (*sectorp + count > pmiarray->mi_sectors + 1)
    count = pmiarray->mi_sectors + 1 - *sectorp;
  return count;
}

