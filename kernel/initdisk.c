/****************************************************************/
/*                                                              */
/*                            initDISK.c                        */
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
#include "init-mod.h"
#include "dyndata.h"
#ifdef VERSION_STRINGS
static BYTE *dskRcsId = "$Id$";
#endif

/*
    data shared between DSK.C and INITDISK.C
*/    
extern UBYTE FAR DiskTransferBuffer[1 * SEC_SIZE];

extern COUNT FAR nUnits;

extern UWORD FAR LBA_WRITE_VERIFY;

/*
 *    Rev 1.0   13 May 2001  tom ehlert
 * Initial revision.
 *
 * this module implements the disk scanning for DOS accesible partitions
 * the drive letter ordering is somewhat chaotic, but like MSDOS does it.
 *
 * this module expects to run with CS = INIT_TEXT, like other init_code,
 * but SS = DS = DATA = DOS_DS, unlike other init_code.
 *
 * history:
 * 1.0 extracted the disk init code from DSK.C
 *     added LBA support
 *     moved code to INIT_TEXT
 *     done the funny code segment stuff to switch between INIT_TEXT and TEXT
 *     added a couple of snity checks for partitions
 *
 ****************************************************************************
 *
 * Implementation note:
 * this module needs some interfacing to INT 13
 * how to implement them
 *    
 * a) using inline assembly 
 *        _ASM mov ax,0x1314
 *
 * b) using assembly routines in some external FLOPPY.ASM
 *
 * c) using the funny TURBO-C style
 *        _AX = 0x1314
 *
 * d) using intr(intno, &regs) method.
 *
 * whynot's
 *
 * a) this is my personal favorite, combining the best aof all worlds.
 *    TURBO-C does support inline assembly, but only by using TASM,
 *    which is not free. 
 *    so - unfortunately- its excluded.
 *
 * b) keeping funny memory model in sync with external assembly
 *    routines is everything, but not fun
 *
 * c) you never know EXACT, what the compiler does, if its a bit
 *    more complicated. does
 *      _DL = drive & 0xff    
 *      _BL = driveParam.chs.Sector;
 *    destroy any other register? sure? _really_ sure?
 *    at least, it has it's surprises.
 *    and - I found a couple of optimizer induced bugs (TC 2.01)
 *      believe me.
 *    it was coded - and operational that way.
 *    but - there are many surprises waiting there. so I opted against.
 *
 *
 * d) this method is somewhat clumsy and certainly not the
 *    fastest way to do things.
 *    on the other hand, this is INIT code, executed once.
 *    and scince it's the only portable method, I opted for it.
 *
 * e) and all this is my private opinion. tom ehlert.
 *
 */


/* #define DEBUG */

#define _BETA_              /* messages for initial phase only */


#if defined(DEBUG)
    #define DebugPrintf(x) printf x
#else
    #define DebugPrintf(x)
#endif

#if defined(_BETA_)
    #define BetaPrintf(x) printf x
#else
    #define BetaPrintf(x)
#endif


#define LBA_to_CHS   init_LBA_to_CHS

/*
    interesting macros - used internally only
*/

#define SCAN_PRIMARYBOOT 0x00
#define SCAN_PRIMARY     0x01
#define SCAN_EXTENDED    0x02
#define SCAN_PRIMARY2    0x03


#define FAT12           0x01
#define FAT16SMALL      0x04
#define EXTENDED        0x05
#define FAT16LARGE      0x06
#define FAT32           0x0b  /* FAT32 partition that ends before the 8.4  */
                              /* GB boundary                               */
#define FAT32_LBA       0x0c  /* FAT32 partition that ends after the 8.4GB */
                              /* boundary.  LBA is needed to access this.  */
#define FAT16_LBA       0x0e  /* like 0x06, but it is supposed to end past */
                              /* the 8.4GB boundary                        */
#define EXTENDED_LBA    0x0f  /* like 0x05, but it is supposed to end past */

#define IsExtPartition(parttyp) ((parttyp) == EXTENDED || \
                                 (parttyp) == EXTENDED_LBA )

#define IsFAT16Partition(parttyp) ((parttyp) == FAT12      || \
                                   (parttyp) == FAT16SMALL || \
                                   (parttyp) == FAT16LARGE || \
                                   (parttyp) == FAT16_LBA  )

#define MSDOS_EXT_SIGN 0x29             /* extended boot sector signature */
#define MSDOS_FAT12_SIGN "FAT12   "     /* FAT12 filesystem signature */
#define MSDOS_FAT16_SIGN "FAT16   "     /* FAT16 filesystem signature */
#define MSDOS_FAT32_SIGN "FAT32   "     /* FAT32 filesystem signature */

/* local - returned and used for BIOS interface INT 13, AH=48*/
struct _bios_LBA_disk_parameterS {
        UWORD size;
        UWORD information;
        ULONG cylinders;
        ULONG heads;
        ULONG sectors;

        ULONG totalSect;
        ULONG totalSectHigh;
        UWORD BytesPerSector;

        ULONG eddparameters;
       } ;


/* physical characteristics of a drive */

struct DriveParamS
{
    UBYTE driveno;              /* = 0x8x                           */
    BITS LBA_supported:1;        /* set, if INT13 extensions enabled */
    BITS WriteVerifySupported:1; /* */
    ULONG total_sectors;

    struct CHS chs;             /* for normal   INT 13 */
};

struct PartTableEntry     /* INTERNAL representation of partition table entry */
  {
    UBYTE Bootable;
    UBYTE FileSystem;
    struct CHS Begin;
    struct CHS End;
    ULONG RelSect;
    ULONG NumSect;
  };

/*
    internal global data
*/

UBYTE GlobalEnableLBAsupport = 1;       /* = 0 --> disable LBA support */  

/*
    translate LBA sectors into CHS addressing
    copied and pasted from dsk.c!
*/

void init_LBA_to_CHS(struct CHS *chs, ULONG LBA_address, struct DriveParamS *driveparam)
{
    chs->Sector = LBA_address% driveparam->chs.Sector + 1;

    LBA_address /= driveparam->chs.Sector;

    chs->Head     = LBA_address % driveparam->chs.Head;
    chs->Cylinder = LBA_address / driveparam->chs.Head;
}

void printCHS(char *title,struct CHS *chs)
{
    printf("%s",title);
    printf("%4lu-%u-%u",chs->Cylinder, chs->Head, chs->Sector);
}    

/*
    reason for this modules existence:
    
    we have found a partition, and add them to the global 
    partition structure.

*/

/* Compute ceil(a/b) */
#define cdiv(a, b) (((a) + (b) - 1) / (b))

/* calculates FAT data:
   code adapted by Bart Oldeman from mkdosfs from the Linux dosfstools:
      Author:       Dave Hudson
      Updated by:   Roman Hodek
      Portions copyright 1992, 1993 Remy Card
      and 1991 Linus Torvalds
*/
VOID CalculateFATData(ddt FAR *pddt, ULONG NumSectors, UBYTE FileSystem)
{
    ULONG fatlength, maxclust, clust;
    UBYTE maxclustsize;
    ULONG fatdata;
      
    bpb FAR *defbpb = &pddt->ddt_defbpb;
    
    /* FAT related items */
    defbpb->bpb_nfat = 2;
    defbpb->bpb_ndirent = 512; /* normal value of number of entries in root dir
                                  should be 0 for FAT32 drives */
    defbpb->bpb_nreserved = 1; /* 0x20 for FAT32 */

    fatdata = NumSectors - cdiv (defbpb->bpb_ndirent * 32, defbpb->bpb_nbyte) -
        defbpb->bpb_nreserved;
    maxclustsize = 128;
#ifdef DEBUG    
    if (FileSystem!=FAT12)
        DebugPrintf(( "%ld sectors for FAT+data, starting with %d sectors/cluster\n",
                  fatdata, defbpb->bpb_nsector ));
#endif    
    switch(FileSystem) {

    case FAT12:
        /* in DOS, FAT12 defaults to 4096kb (8 sector) - clusters. */
        defbpb->bpb_nsector = 8;
        /* Force maximal fatdata=32696 sectors since with our only possible sector
           size (512 bytes) this is the maximum for 4k clusters.
           #clus*secperclus+#fats*fatlength= 4084 * 8 + 2 * 12 = 32696.
           max FAT12 size for FreeDOS = 16,728,064 bytes */
        if (fatdata > 32696)
            fatdata = 32696;
        /* The factor 2 below avoids cut-off errors for nr_fats == 1.
         * The "defbpb->bpb_nfat*3" is for the reserved first two FAT entries */
        clust = 2*((ULONG) fatdata * defbpb->bpb_nbyte + defbpb->bpb_nfat*3) /
            (2*(ULONG) defbpb->bpb_nsector * defbpb->bpb_nbyte + defbpb->bpb_nfat*3);
        fatlength = cdiv (((clust+2) * 3 + 1) >> 1, defbpb->bpb_nbyte);
        /* Need to recalculate number of clusters, since the unused parts of the
         * FATS and data area together could make up space for an additional,
         * not really present cluster. */
        clust = (fatdata - defbpb->bpb_nfat*fatlength)/defbpb->bpb_nsector;
        maxclust = (fatlength * 2 * defbpb->bpb_nbyte) / 3;
        if (maxclust > FAT_MAGIC)
            maxclust = FAT_MAGIC;
        DebugPrintf(( "FAT12: #clu=%lu, fatlen=%lu, maxclu=%lu, limit=%u\n",
                      clust, fatlength, maxclust, FAT_MAGIC ));
        if (clust > maxclust-2) {
            clust = maxclust-2;
            DebugPrintf(( "FAT12: too many clusters: setting to maxclu-2\n" ));
        }
        defbpb->bpb_nfsect = fatlength;
        fmemcpy(pddt->ddt_fstype, MSDOS_FAT12_SIGN, 8);
        break;

    case FAT16SMALL:    
    case FAT16LARGE:
    case FAT16_LBA:    
        /* FAT16: start at 4 sectors per cluster */
        defbpb->bpb_nsector = 4;
        /* Force maximal fatdata=8387584 sectors (NumSectors=8387617)
           since with our only possible sectorsize (512 bytes) this is the
           maximum we can address with 64k clusters
           #clus*secperclus+#fats*fatlength=65524 * 128 + 2 * 256=8387584.
           max FAT16 size for FreeDOS = 4,294,180,864 bytes = 4GiB-786,432 */
        if (fatdata > 8387584ul)
            fatdata = 8387584ul;
        do {
            DebugPrintf(( "Trying with %d sectors/cluster:\n", defbpb->bpb_nsector ));

            clust = ((ULONG) fatdata *defbpb->bpb_nbyte + defbpb->bpb_nfat*4) /
                ((ULONG) defbpb->bpb_nsector * defbpb->bpb_nbyte + defbpb->bpb_nfat*2);
            fatlength = cdiv ((clust+2) * 2, defbpb->bpb_nbyte);
            /* Need to recalculate number of clusters, since the unused parts of the
             * FATS and data area together could make up space for an additional,
             * not really present cluster. */
            clust = (fatdata - defbpb->bpb_nfat*fatlength)/defbpb->bpb_nsector;
            maxclust = (fatlength * defbpb->bpb_nbyte) / 2;
            if (maxclust > FAT_MAGIC16)
                maxclust = FAT_MAGIC16;
            DebugPrintf(( "FAT16: #clu=%lu, fatlen=%lu, maxclu=%lu, limit=%u\n",
                          clust, fatlength, maxclust, FAT_MAGIC16 ));
            if (clust > maxclust-2) {
                DebugPrintf(( "FAT16: too many clusters\n" ));
                clust = 0;
            } else if (clust <= FAT_MAGIC) {
                /* The <= 4086 avoids that the filesystem will be misdetected as having a
                 * 12 bit FAT. */
                DebugPrintf(("FAT16: would be misdetected as FAT12\n"));
                clust = 0;
            }
            if (clust)
                break;
            defbpb->bpb_nsector <<= 1;
        }
        while (defbpb->bpb_nsector && defbpb->bpb_nsector <= maxclustsize);
        defbpb->bpb_nfsect = fatlength;
        fmemcpy(pddt->ddt_fstype, MSDOS_FAT16_SIGN, 8);
        break;
        
        /* FAT 32 code: commented out for now */
#ifdef WITHFAT32
    case FAT32:
        /* For FAT32, use 4k clusters on sufficiently large file systems,
         * otherwise 1 sector per cluster. This is also what M$'s format
         * command does for FAT32. */
        defbpb->bpb_nsector = ((long long)blocks*SECTORS_PER_BLOCK >= 512*1024 ? 8 : 1);
        do {
            clust = ((long long) fatdata *defbpb->bpb_nbyte + defbpb->bpb_nfat*8) /
                ((int) defbpb->bpb_nsector * defbpb->bpb_nbyte + defbpb->bpb_nfat*4);
            fatlength = cdiv ((clust+2) * 4, defbpb->bpb_nbyte);
            /* Need to recalculate number of clusters, since the unused parts of the
             * FATS and data area together could make up space for an additional,
             * not really present cluster. */
            clust = (fatdata - defbpb->bpb_nfat*fatlength)/defbpb->bpb_nsector;
            maxclust = (fatlength * defbpb->bpb_nbyte) / 4;
            if (maxclust > MAX_CLUST_32)
                maxclust = MAX_CLUST_32;
            DebugPrintf(( "FAT32: #clu=%u, fatlen=%u, maxclu=%u, limit=%u\n",
                          clust, fatlength, maxclust, FAT_MAGIC ));
            if (clust > maxclust) 
            {
                clust = 0;
                DebugPrintf(( "FAT32: too many clusters\n" ));
            }
            if (clust)
                break;
            defbpb->bpb_nsector <<= 1;
        }  while (defbpb->bpb_nsector && defbpb->bpb_nsector <= maxclustsize);
        bpb_nfsect = fatlength;
        defbpb->bpb_nfsect = 0;
        defbpb->fat32.fat32_length = fatlength;
        memcpy(pddt->ddt_fstype, MSDOS_FAT32_SIGN, 8);
        break;
#endif
    }  
    pddt->ddt_fstype[8] = '\0';
}


void DosDefinePartition(struct DriveParamS *driveParam,
            ULONG StartSector, struct PartTableEntry *pEntry)
{
      ddt FAR *pddt = DynAlloc("ddt", 1, sizeof(ddt));
      struct CHS chs;

      if ( nUnits >= NDEV)
        {
        printf("more Partitions detected then possible, max = %d\n", NDEV);
        return;                            /* we are done */
        }

      pddt->ddt_driveno = driveParam->driveno;
      pddt->ddt_LBASupported = driveParam->LBA_supported;
      pddt->ddt_WriteVerifySupported = driveParam->WriteVerifySupported;
      pddt->ddt_ncyl = driveParam->chs.Cylinder;
      
      if (pddt->ddt_LBASupported)
                 DebugPrintf(("LBA enabled for drive %c:\n", 'A'+nUnits));
        
      pddt->ddt_offset  = StartSector;

      pddt->ddt_defbpb.bpb_nbyte = SEC_SIZE;
      pddt->ddt_defbpb.bpb_mdesc = 0xf8;
      pddt->ddt_defbpb.bpb_nheads = driveParam->chs.Head;
      pddt->ddt_defbpb.bpb_nsecs = driveParam->chs.Sector;
      pddt->ddt_defbpb.bpb_nsize = 0;
      pddt->ddt_defbpb.bpb_hidden = pEntry->RelSect;
      if (pEntry->NumSect > 0xffff)
        pddt->ddt_defbpb.bpb_huge = pEntry->NumSect;
      else
        pddt->ddt_defbpb.bpb_nsize = (UWORD)(pEntry->NumSect);

      /* sectors per cluster, sectors per FAT etc. */
      CalculateFATData(pddt, pEntry->NumSect, pEntry->FileSystem);
      
      pddt->ddt_serialno = 0x12345678l;
      pddt->ddt_descflags = 0x200;  /* drive inaccessible until bldbpb successful */
      fmemcpy(&pddt->ddt_bpb, &pddt->ddt_defbpb, sizeof(bpb));

#ifdef _BETA_      /* Alain whishes to keep this in later versions, too */
      LBA_to_CHS(&chs,StartSector,driveParam);

      printf("%c: disk %02x",
            'A' + nUnits,
            driveParam->driveno);

      printCHS(" CHS= ",&chs);
      
      printf(" start = %5luMB,size =%5lu",
            StartSector/2048,pEntry->NumSect/2048);
      
      printf("\n");
#endif                
      
      
      nUnits++;
}



void __int__(int);


/* Get the parameters of the hard disk */
int LBA_Get_Drive_Parameters(int drive,struct DriveParamS *driveParam)
{
    iregs regs;
    
    struct _bios_LBA_disk_parameterS lba_bios_parameters;

    if (driveParam->driveno)
        return driveParam->driveno;

    driveParam->LBA_supported = FALSE;


    drive |= 0x80;

                                /* for tests - disable LBA support,
                                   even if exists                    */
    if (!GlobalEnableLBAsupport)
        {
        goto StandardBios;
        }
                                /* check for LBA support */
    regs.b.x   = 0x55aa;
    regs.a.b.h = 0x41;
    regs.d.b.l = drive;

    init_call_intr(0x13,&regs);


    if (regs.b.x != 0xaa55 || (regs.flags & 0x01) )
        {
        goto StandardBios;
        }

                                /* by ralph :
            					   if DAP cannot be used, don't use
					                LBA
                                */
    if ((regs.c.x & 1)	== 0)
        {
        goto StandardBios;
        }
    
    

                        /* drive supports LBA addressing */

                        /* version 1.0, 2.0 have different verify */
    if (regs.a.x < 0x2100)
        LBA_WRITE_VERIFY = 0x4301;


    lba_bios_parameters.size = sizeof(lba_bios_parameters);


    regs.si    = FP_OFF(&lba_bios_parameters);
    regs.ds    = FP_SEG(&lba_bios_parameters);
    regs.a.b.h = 0x48;
    regs.d.b.l = drive;
    init_call_intr(0x13,&regs);


    if (regs.flags & 0x01)
        {
        goto StandardBios;
        }

                                /* verify maximum settings, we can't handle more */

    if (lba_bios_parameters.heads   > 0xffff    ||
        lba_bios_parameters.sectors > 0xffff    ||
        lba_bios_parameters.totalSectHigh != 0  )
        {
        printf("Drive is too large to handle, using only 1'st 8 GB\n"
               " drive %02x heads %lu sectors %lu , total=0x%lx-%08lx\n",
               drive,
               (ULONG)lba_bios_parameters.heads,
               (ULONG)lba_bios_parameters.sectors,
               (ULONG)lba_bios_parameters.sectors,
               (ULONG)lba_bios_parameters.totalSectHigh);

        goto StandardBios;
        }
    
    if (lba_bios_parameters.information & 8)
        {
        driveParam->WriteVerifySupported = 1;    
        }
    else
        driveParam->WriteVerifySupported = 0;    

    driveParam->total_sectors   = lba_bios_parameters.totalSect;

                                /* if we arrive here, success */
    driveParam->LBA_supported = TRUE;




StandardBios:               /* old way to get parameters */


    regs.a.b.h = 0x08;
    regs.d.b.l = drive;

    init_call_intr(0x13,&regs);


    if (regs.flags & 0x01)
        goto ErrorReturn;


    driveParam->chs.Head     = (regs.d.x >> 8) + 1;
    driveParam->chs.Sector   = (regs.c.x & 0x3f);
    driveParam->chs.Cylinder = (regs.c.x >> 8) | ((regs.c.x & 0xc0) << 2);
    

    if (!driveParam->LBA_supported)
        {
        driveParam->total_sectors   =
              min(driveParam->chs.Cylinder,1023)
            * driveParam->chs.Head
            * driveParam->chs.Sector;
        }

    driveParam->driveno = drive;

    DebugPrintf(("drive parameters %02x - %04lu-%u-%u",
                        drive,
                        driveParam->chs.Cylinder,
                        driveParam->chs.Head,
                        driveParam->chs.Sector));
    DebugPrintf((" total size %luMB\n\n",driveParam->total_sectors/2048));

    

ErrorReturn:

    return driveParam->driveno;
}




/*
    converts physical into logical representation of partition entry
*/

ConvPartTableEntryToIntern(struct PartTableEntry *pEntry, UBYTE FAR * pDisk)
{
    int i;

    if (pDisk[0x1fe] != 0x55 || pDisk[0x1ff] != 0xaa)
        {
        memset(pEntry,0, 4 * sizeof(struct PartTableEntry));

        return FALSE;
        }

    pDisk += 0x1be;

    for (i = 0; i < 4; i++,pDisk += 16,pEntry++)
        {

        pEntry->Bootable        = *(UBYTE FAR*)(pDisk+0);
        pEntry->FileSystem      = *(UBYTE FAR*)(pDisk+4);

        pEntry->Begin.Head       = *(UBYTE FAR*)(pDisk+1);
        pEntry->Begin.Sector     = *(UBYTE FAR*)(pDisk+2) & 0x3f;
        pEntry->Begin.Cylinder   = *(UBYTE FAR*)(pDisk+3) +
                                      ((UWORD) (0xc0 & *(UBYTE FAR*)(pDisk+2)) << 2);

        pEntry->End.Head         = *(UBYTE FAR*)(pDisk+5);
        pEntry->End.Sector       = *(UBYTE FAR*)(pDisk+6) & 0x3f;
        pEntry->End.Cylinder     = *(UBYTE FAR*)(pDisk+7) +
                                      ((UWORD) (0xc0 & *(UBYTE FAR*)(pDisk+6)) << 2);


        pEntry->RelSect         = *(ULONG FAR*)(pDisk+8);
        pEntry->NumSect         = *(ULONG FAR*)(pDisk+12);
        }
    return TRUE;
}

ScanForPrimaryPartitions(struct DriveParamS *driveParam,int scan_type,
                    struct PartTableEntry *pEntry, ULONG startSector,
                    int partitionsToIgnore
                    )
{
    int i;
    struct CHS chs,end;
    ULONG  partitionStart;

    for (i = 0; i < 4; i++,pEntry++)
        {
        if (pEntry->FileSystem == 0)
            continue;

        if (partitionsToIgnore & (1 << i))
            continue;


        if (IsExtPartition(pEntry->FileSystem))
            continue;

        if (scan_type == SCAN_PRIMARYBOOT && !pEntry->Bootable)
            continue;


        partitionStart = startSector + pEntry->RelSect;

        if (!IsFAT16Partition(pEntry->FileSystem))
           {
           continue;
           }

                                    /*
                                        some sanity checks, that partition
                                        structure is OK
                                    */
        LBA_to_CHS(&chs, partitionStart,driveParam);
        LBA_to_CHS(&end, partitionStart+pEntry->NumSect-1,driveParam);


                                    /* some FDISK's enter for partitions 
                                       > 8 GB cyl = 1023, other (cyl&1023)
                                    */   

        if ( ((chs.Cylinder & 0x3ff) != pEntry->Begin.Cylinder &&
              1023                   != pEntry->Begin.Cylinder )  ||
             chs.Head              != pEntry->Begin.Head          ||
             chs.Sector            != pEntry->Begin.Sector    )
            {
            printf("NOT using suspect partition %u FS %02x:",
                        i, pEntry->FileSystem);
             printCHS(" start calc ",&chs);
             printCHS(" != ",&pEntry->Begin);
             printf("\n");

            continue;
            }


        if (((end.Cylinder & 0x3ff) != pEntry->End.Cylinder &&
               1023                 != pEntry->End.Cylinder  )  ||
             end.Head              != pEntry->End.Head          ||
             end.Sector            != pEntry->End.Sector    )
            {
            printf("NOT using suspect partition %u FS %02x:",
                        i, pEntry->FileSystem);

             printCHS(" end calc ",&end);
             printCHS(" != ",&pEntry->End);
             printf("\n");

            continue;
            }


        if (chs.Cylinder > 1023 || end.Cylinder > 1023)
           {

           if (!driveParam->LBA_supported)
               {
               printf("can't use LBA partition without LBA support - part %u FS %02x",
                       i, pEntry->FileSystem);
                       
                printCHS(" start ",&chs);
                printCHS(", end ", &end);
                printf("\n");

                continue;
                }

            /* else its a diagnostic message only */
#ifdef _BETA_            
           printf("found and using LBA partition %u FS %02x",
                       i, pEntry->FileSystem);
           printCHS(" start ",&chs);
           printCHS(", end ", &end);
           printf("\n");
#endif           
           }


        /*
            here we have a partition table in our hand !!
        */

        partitionsToIgnore |= 1 << i;
        
        DosDefinePartition(driveParam,partitionStart, pEntry);

        if (scan_type == SCAN_PRIMARYBOOT ||
            scan_type == SCAN_PRIMARY     )
            {
            return partitionsToIgnore;
            }
        }

    return partitionsToIgnore;
}

void BIOS_drive_reset(unsigned drive);

int Read1LBASector(struct DriveParamS *driveParam, unsigned drive, ULONG LBA_address, void FAR *buffer)
{
    static struct _bios_LBA_address_packet dap = {
         16,0,0,0,0,0,0
        };
    
    struct   CHS chs;
    iregs regs;
    int num_retries;

/* disabled because this should not happen and if it happens the BIOS
   should complain; also there are weird disks around with
   CMOS geometry < real geometry */
#if 0
    if (LBA_address >= driveParam->total_sectors)
        {
        printf("LBA-Transfer error : address overflow = %lu > %lu max\n",LBA_address+1,driveParam->total_sectors);
        return 1;
        }
#endif

    for ( num_retries = 0; num_retries < N_RETRY; num_retries++)
        {
        regs.d.b.l = drive | 0x80;
        if (driveParam->LBA_supported)
            {
            dap.number_of_blocks    = 1;
            dap.buffer_address      = buffer;
            dap.block_address_high  = 0;               /* clear high part */
            dap.block_address       = LBA_address;     /* clear high part */
            
            /* Load the registers and call the interrupt. */
            regs.a.x = LBA_READ;
            regs.si = FP_OFF(&dap);
            regs.ds = FP_SEG(&dap);
            }
        else
            {                            /* transfer data, using old bios functions */
            init_LBA_to_CHS(&chs, LBA_address, driveParam);
                                         /* avoid overflow at end of track */
                
            if (chs.Cylinder > 1023)
            {
                printf("LBA-Transfer error : cylinder %u > 1023\n", chs.Cylinder);
                return 1;
            }
                
            regs.a.x = 0x0201;
            regs.b.x = FP_OFF(buffer);
            regs.c.x = ((chs.Cylinder&0xff) << 8) + ((chs.Cylinder&0x300) >> 2) + chs.Sector;
            regs.d.b.h = chs.Head;
            regs.es = FP_SEG(buffer);
            }       /* end of retries */
        init_call_intr(0x13, &regs);
        if ((regs.flags & FLG_CARRY) == 0) break;
        BIOS_drive_reset(driveParam->driveno);                
        }

    return regs.flags & FLG_CARRY ? 1 : 0;
}    

/* Load the Partition Tables and get information on all drives */
int ProcessDisk(int scanType, unsigned drive, int PartitionsToIgnore)
{

   struct PartTableEntry PTable[4];
   ULONG  RelSectorOffset;
   ULONG  ExtendedPartitionOffset;
   int    iPart;
   int    strangeHardwareLoop;
   
   int num_extended_found = 0;

   struct DriveParamS driveParam;

    /* Get the hard drive parameters and ensure that the drive exists. */
    /* If there was an error accessing the drive, skip that drive. */

   memset(&driveParam, 0, sizeof(driveParam));
   
    if (!LBA_Get_Drive_Parameters(drive,&driveParam))
    {
        printf("can't get drive parameters for drive %02x\n",drive);
        return PartitionsToIgnore;
    }

    RelSectorOffset = 0;            /* boot sector */
    ExtendedPartitionOffset = 0;    /* not found yet */



    /* Read the Primary Partition Table. */


ReadNextPartitionTable:

    strangeHardwareLoop = 0;
strange_restart:


    if (Read1LBASector(&driveParam, drive, RelSectorOffset, DiskTransferBuffer))
        {
        printf("Error reading partition table drive %02x sector %lu",drive,RelSectorOffset);
        return PartitionsToIgnore;
        }

    if (!ConvPartTableEntryToIntern(PTable, DiskTransferBuffer))
        {
                        /* there is some strange hardware out in the world,
                           which returns OK on first read, but the data are
                           rubbish. simply retrying works fine.
                           there is no logic behind this, but it works TE */
            
        if (++strangeHardwareLoop < 3)
            goto strange_restart;
            
        printf("illegal partition table - drive %02x sector %lu\n",drive,RelSectorOffset);
        return PartitionsToIgnore;
        }

    if ( scanType==SCAN_PRIMARYBOOT ||
         scanType==SCAN_PRIMARY     ||
         scanType==SCAN_PRIMARY2    ||
         num_extended_found !=0   )
         {

            PartitionsToIgnore = ScanForPrimaryPartitions(&driveParam,scanType,
                                PTable, RelSectorOffset,PartitionsToIgnore);
         }

    if (scanType != SCAN_EXTENDED)
        {
        return  PartitionsToIgnore;
        }

                                /* scan for extended partitions now */
    PartitionsToIgnore = 0;


    for (iPart=0; iPart < 4; iPart++)
	{
	    if (IsExtPartition(PTable[iPart].FileSystem))
	        {
            RelSectorOffset = ExtendedPartitionOffset + PTable[iPart].RelSect;

            if (ExtendedPartitionOffset == 0)
                {
                ExtendedPartitionOffset = PTable[iPart].RelSect;
                }

            num_extended_found++;

            if (num_extended_found > 30)
                {
                printf("found more then 30 extended partitions, terminated\n");
                return 0;
                }

            goto ReadNextPartitionTable;
	        }
    }

    return PartitionsToIgnore;
}


int BIOS_nrdrives(void)
{
    iregs regs;

    regs.a.b.h = 0x08;
    regs.d.b.l = 0x80;
    init_call_intr(0x13,&regs);

    if (regs.flags & 1)
        {
        printf("no hard disks detected\n");
        return 0;
        }

    return regs.d.b.l;
}

void BIOS_drive_reset(unsigned drive)
{
   iregs regs;
   
   regs.d.b.l = drive | 0x80;
   regs.a.b.h = 0;

   init_call_intr(0x13,&regs);
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

or

  as rather well documented, DOS searches 1st) 1 primary patitions on
     all drives, 2nd) all extended partitions. that
     makes many people (including me) unhappy, as all DRIVES D:,E:...
     on 1st disk will move up/down, if other disk with
     primary partitions are added/removed, but
     thats the way it is (hope I got it right)
     TE (with a little help from my friends)
     see also above for WIN2000,DOS,MSDN

I don't know, if I did it right, but I tried to do it that way. TE

***********************************************************************/



void ReadAllPartitionTables(void)
{
    UBYTE foundPartitions[MAX_HARD_DRIVE];

    int HardDrive;
    int nHardDisk = BIOS_nrdrives();
    bpb FAR *pbpbarray;
    int Unit;
    ddt FAR *pddt;

    /* Setup media info and BPBs arrays for floppies (this is a 360kb flop) */
    for (Unit = 0; Unit < nUnits; Unit++)
    {
        pddt = DynAlloc("ddt", 1, sizeof(ddt));
        
        pbpbarray = &pddt->ddt_defbpb;    

        pbpbarray->bpb_nbyte = SEC_SIZE;
        pbpbarray->bpb_nsector = 2;
        pbpbarray->bpb_nreserved = 1;
        pbpbarray->bpb_nfat = 2;
        pbpbarray->bpb_ndirent = 112;
        pbpbarray->bpb_nsize = 360*2;
        pbpbarray->bpb_mdesc = 0xfd;
        pbpbarray->bpb_nfsect = 2;
        pbpbarray->bpb_nheads = 2;
        pbpbarray->bpb_nsecs = 9;

        pddt->ddt_driveno = 0;
        pddt->ddt_ncyl = 40;
        pddt->ddt_LBASupported = FALSE;
    
        pddt->ddt_offset = 0l;
       
    
        pddt->ddt_serialno = 0x12345678l;
        fmemcpy(&pddt->ddt_bpb, pbpbarray, sizeof(bpb));
    }    

    /*
      this is a quick patch - see if B: exists
      test for A: also, need not exist
    */
    {
        iregs r;

        init_call_intr(0x11,&r);              /* get equipment list */
        if ((r.a.x & 1) && (r.a.x & 0xc0))
            pddt->ddt_driveno = 1;
            /* floppy drives installed and a B: drive */
            /*if ((r.a.x & 1)==0) */ /* no floppy drives installed  */
    }

    nHardDisk = min(nHardDisk,MAX_HARD_DRIVE-1);

    memset(foundPartitions,0,sizeof(foundPartitions));




    DebugPrintf(("DSK init: found %d disk drives\n",nHardDisk));

    /* Reset the drives                                             */
    for (HardDrive = 0; HardDrive < nHardDisk; HardDrive++)
      BIOS_drive_reset(HardDrive);
    
    
    /* Process primary partition table   1 partition only      */
    for (HardDrive = 0; HardDrive < nHardDisk; HardDrive++)
    {
        foundPartitions[HardDrive] =
            ProcessDisk(SCAN_PRIMARYBOOT, HardDrive, 0);
    
    if (foundPartitions[HardDrive] == 0)
        foundPartitions[HardDrive] = ProcessDisk(SCAN_PRIMARY, HardDrive, 0);
    }

    /* Process extended partition table                      */
    for (HardDrive = 0; HardDrive < nHardDisk; HardDrive++)
    {
        ProcessDisk(SCAN_EXTENDED, HardDrive , 0);
    }
    
    /* Process primary a 2nd time */
    for (HardDrive = 0; HardDrive < nHardDisk; HardDrive++)
    {
        ProcessDisk(SCAN_PRIMARY2, HardDrive ,foundPartitions[HardDrive]);
    }
}

/* disk initialization: returns number of units */
COUNT dsk_init()
{
  printf(" - InitDisk\n");

#ifdef DEBUG
    {
    iregs regs;
    regs.a.x = 0x1112;    /* select 43 line mode - more space for partinfo */
    regs.b.x = 0;
    init_call_intr(0x10, &regs);
    }
#endif
  
  /* Reset the drives                                             */
  BIOS_drive_reset(0);

  /* Initial number of disk units                                 */
  nUnits = 2;
  

  ReadAllPartitionTables();

  return nUnits;
}
