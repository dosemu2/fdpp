/****************************************************************/
/*                                                              */
/*                          globals.h                           */
/*                            DOS-C                             */
/*                                                              */
/*             Global data structures and declarations          */
/*                                                              */
/*                   Copyright (c) 1995, 1996                   */
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

#ifdef VERSION_STRINGS
#ifdef MAIN
static BYTE *Globals_hRcsId =
    "$Id: globals.h 1705 2012-02-07 08:10:33Z perditionc $";
#endif
#endif

#include "device.h"
#include "mcb.h"
#include "pcb.h"
#include "date.h"
#include "time.h"
#include "fat.h"
#include "fcb.h"
#include "tail.h"
#include "process.h"
#include "sft.h"
#include "cds.h"
#include "exe.h"
#include "dirmatch.h"
#include "fnode.h"
#include "file.h"
#include "clock.h"
#include "kbd.h"
#include "error.h"
#include "version.h"
#include "network.h"
#include "buffer.h"
#include "dcb.h"
#include "xstructs.h"
#include "kconfig.h"
#include "lol.h"
#include "bprm.h"
#include "nls.h"
#include "dyn.h"
#include "memtype.h"
#include "glob_fd.h"
#include "debug.h"

/* fatfs.c */
#ifdef WITHFAT32
VOID bpb_to_dpb(__FAR(bpb) bpbp, __FAR(REG struct dpb) dpbp, BOOL extended);
#else
VOID bpb_to_dpb(__FAR(bpb) bpbp, __FAR(REG struct dpb) dpbp);
#endif

#ifdef WITHFAT32
__FAR(struct dpb) GetDriveDPB(UBYTE drive, COUNT * rc);
#endif


/* JPP: for testing/debuging disk IO */
/*#define DISPLAY_GETBLOCK */

/*                                                                      */
/* Convience switch for maintaining variables in a single location      */
/*                                                                      */
#ifdef MAIN
#define GLOBAL
#else
#define GLOBAL extern
#endif

/*                                                                      */
/* Convience definitions of TRUE and FALSE                              */
/*                                                                      */
#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

/*                                                                      */
/* Constants and macros                                                 */
/*                                                                      */
/* Defaults and limits - System wide                                    */
#define NAMEMAX         MAX_CDSPATH     /* Maximum path for CDS         */

/* internal error from failure or aborted operation                     */
#define ERROR           -1
#define OK              0

/* internal transfer direction flags                                    */
#define XFR_READ        1
#define XFR_WRITE       2
#define XFR_FORCE_WRITE 3
/* flag to update fcb_rndm field */
#define XFR_FCB_RANDOM  4

#define RDONLY          0
#define WRONLY          1
#define RDWR            2

/* special ascii code equates                                           */
#define SPCL            0x00
#define CTL_C           0x03
#define CTL_F           0x06
#define BELL            0x07
#define BS              0x08
#define HT              0x09
#define LF              0x0a
#define CR              0x0d
#define CTL_P           0x10
#define CTL_Q           0x11
#define CTL_S           0x13
#define CTL_Z           0x1a
#define ESC             0x1b
#define CTL_BS          0x7f

#define INS             0x5200
#define DEL             0x5300

#define F1              0x3b00
#define F2              0x3c00
#define F3              0x3d00
#define F4              0x3e00
#define F5              0x3f00
#define F6              0x4000
#define LEFT            0x4b00
#define RIGHT           0x4d00

/* Blockio constants                                                    */
#define DSKWRITE        1       /* dskxfr function parameters   */
#define DSKREAD         2
#define DSKWRITEINT26   3
#define DSKREADINT25    4

/* NLS character table type                                             */
typedef BYTE *UPMAP;
extern COUNT *error_tos,        /* error stack                          */
  disk_api_tos,                 /* API handler stack - disk fns         */
  char_api_tos;                 /* API handler stack - char fns         */
extern
BYTE DosLoadedInHMA;            /* if InitHMA has moved DOS up          */

#ifdef DEBUG
GLOBAL WORD bDumpRegs
#ifdef MAIN
    = FALSE;
#else
 ;
#endif
GLOBAL WORD bDumpRdWrParms
#ifdef MAIN
    = FALSE;
#else
 ;
#endif
#endif

#if 0                           /* defined in MAIN.C now to save low memory */

GLOBAL BYTE copyright[] =
    "(C) Copyright 1995-2006 Pasquale J. Villani and The FreeDOS Project.\n"
    "All Rights Reserved. This is free software and comes with ABSOLUTELY NO\n"
    "WARRANTY; you can redistribute it and/or modify it under the terms of the\n"
    "GNU General Public License as published by the Free Software Foundation;\n"
    "either version 2, or (at your option) any later version.\n";

#endif

/* Globally referenced variables - WARNING: ORDER IS DEFINED IN         */
/* KERNEL.ASM AND MUST NOT BE CHANGED. DO NOT CHANGE ORDER BECAUSE THEY */
/* ARE DOCUMENTED AS UNDOCUMENTED (?) AND HAVE MANY  PROGRAMS AND TSRs  */
/* ACCESSING THEM                                                       */

/* original interrupt vectors, at 70:xxxx */
struct lowvec {
  unsigned char intno;
  intvec isv;
} PACKED;

enum {LOC_CONV=0, LOC_HMA=1};

struct _FcbSearchBuffer {
  COUNT nDrive;
  BYTE szName[FNAME_SIZE + 1];
  BYTE szExt[FEXT_SIZE + 1];
};

struct __PriPathBuffer                  /* Path name parsing buffer             */
{
  AR_MEMB(__PriPathBuffer, BYTE, _PriPathName, 128);
};

#define PriPathName _PriPathBuffer._PriPathName

struct __SecPathBuffer                  /* Alternate path name parsing buffer   */
{
  AR_MEMB(__SecPathBuffer, BYTE, _SecPathName, 128);
};

#define SecPathName _SecPathBuffer._SecPathName

/*extern WORD
  NumFloppies; !!*//* How many floppies we have            */

extern __FAR(UBYTE) DiskTransferBuffer;
extern __FAR(ddt) ddt_buf[26];

/* start of uncontrolled variables                                      */

#ifdef DEBUG
GLOBAL iregs error_regs;        /* registers for dump                   */

GLOBAL WORD dump_regs;          /* dump registers of bad call           */

#endif

/*                                                                      */
/* Function prototypes - automatically generated                        */
/*                                                                      */
#include "proto.h"

/* Process related functions - not under automatic generation.  */
/* Typically, these are in ".asm" files.                        */
#if 0
VOID ASMCFUNC SEGM(HMA_TEXT) FAR set_stack(VOID);
VOID ASMCFUNC SEGM(HMA_TEXT) FAR restore_stack(VOID);
#endif
/*VOID INRPT FAR handle_break(VOID); */

intvec getvec(unsigned char);

/*                                                              */
/* special word packing prototypes                              */
/*                                                              */
#ifdef NATIVE
#define getlong(vp) (*(UDWORD *)(vp))
#define getword(vp) (*(UWORD *)(vp))
#define getbyte(vp) (*(UBYTE *)(vp))
#define fgetlong(vp) (*(__FAR(UDWORD))(vp))
#define fgetword(vp) (*(__FAR(UWORD))(vp))
#define fgetbyte(vp) (*(__FAR(UBYTE))(vp))
#define fputlong(vp, l) (*(__FAR(UDWORD))(vp)=l)
#define fputword(vp, w) (*(__FAR(UWORD))(vp)=w)
#define fputbyte(vp, b) (*(__FAR(UBYTE))(vp)=b)
#else
UDWORD getlong(VOID *);
UWORD getword(VOID *);
UBYTE getbyte(VOID *);
UDWORD fgetlong(__FAR(VOID));
UWORD fgetword(__FAR(VOID));
UBYTE fgetbyte(__FAR(VOID));
VOID fputlong(__FAR(VOID), UDWORD);
VOID fputword(__FAR(VOID), UWORD);
VOID fputbyte(__FAR(VOID), UBYTE);
#endif

#ifndef __WATCOMC__
//#define setvec setvec_resident
#endif
void setvec(unsigned char intno, intvec vector);
/*#define is_leap_year(y) ((y) & 3 ? 0 : (y) % 100 ? 1 : (y) % 400 ? 0 : 1) */

/* ^Break handling */
#ifdef __WATCOMC__
#pragma aux (cdecl) spawn_int23 aborts;
#endif
void ASMFUNC spawn_int23(void);        /* procsupt.asm */
void ASMFUNC DosIdle_hlt(void);        /* dosidle.asm */

extern BYTE ReturnAnyDosVersionExpected;

/* near fnodes:
 * fnode[0] is used internally for almost all cases.
 * fnode[1] is only used for:
 * 1) rename (target)
 * 2) rmdir (checks if the directory to remove is empty)
 * 3) commit (copies, than closes fnode[0])
 * 3) merge_file_changes (for SHARE)
 */
GLOBAL struct f_node fnode[2];

extern struct _bprm bprm;
extern __FAR(struct cds) old_CDSp;
