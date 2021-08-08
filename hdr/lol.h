/****************************************************************/
/*                                                              */
/*                           lol.h                              */
/*                                                              */
/*              DOS List of Lists structure                     */
/*                                                              */
/*                      Copyright (c) 2003                      */
/*                         Bart Oldeman                         */
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
/* License along with DOS-C; if not, write to the Free Software */
/* Foundation, Inc., 59 Temple Place, Suite 330,                */
/* Boston, MA  02111-1307  USA.                                 */
/****************************************************************/

#ifndef LOL_H
#define LOL_H

//enum {LOC_CONV=0, LOC_HMA=1};

/* note: we start at DOSDS:0, but the "official" list of lists starts a
   little later at DOSDS:26 (this is what is returned by int21/ah=52) */

struct lol {
  char filler[0x22];
  PTR_MEMB(char) _inputptr;              /* -4 Pointer to unread CON input          */
  unsigned short _first_mcb;   /* -2 Start of user memory                 */
  __DOSFAR(struct dpb)_DPBp;       /*  0 First drive Parameter Block          */
  __DOSFAR(struct sfttbl)_sfthead; /*  4 System File Table head               */
  __DOSFAR(struct dhdr)_clock;     /*  8 CLOCK$ device                        */
  __DOSFAR(struct dhdr)_syscon;    /*  c console device                       */
  unsigned short _maxsecsize;   /* 10 max bytes per sector for any blkdev  */
  __DOSFAR(void)inforecptr;        /* 12 pointer to disk buffer info record   */
  __DOSFAR(struct cds)_CDSp;       /* 16 Current Directory Structure          */
  __DOSFAR(struct sfttbl)_FCBp;     /* 1a FCB table pointer                    */
  unsigned short _nprotfcb;     /* 1e number of protected fcbs             */
  unsigned char _nblkdev;      /* 20 number of block devices              */
  unsigned char _lastdrive;    /* 21 value of last drive                  */
  SYM_MEMB(lol, struct dhdr, _nul_dev);        /* 22 NUL device driver header(no pointer!)*/
  unsigned char _njoined;       /* 34 number of joined devices             */
  unsigned short specialptr;   /* 35 pointer to list of spec. prog(unused)*/
  __DOSFAR(void) setverPtr;    /* 37 pointer to SETVER list               */
  unsigned short a20ptr;       /* 3b CS-relative offset to fix A20 ctrl   */
  unsigned short recentpsp;    /* 3d PSP of most recently exec'ed prog    */
  unsigned short nbuffers;     /* 3f Number of buffers                    */
  unsigned short nlookahead;   /* 41 Number of lookahead buffers          */
  unsigned char _BootDrive;    /* 43 bootdrive (1=A:)                     */
  unsigned char cpu;           /* 44 CPU family [was unused dword moves]  */
  unsigned short xmssize;      /* 45 extended memory size in KB           */
  SYM_MEMB(lol, __DOSFAR(struct buffer), _firstbuf);/* 47 head of buffers linked list          */
  unsigned short dirtybuf;     /* 4b number of dirty buffers              */
  __DOSFAR(struct buffer)lookahead;/* 4d pointer to lookahead buffer          */
  unsigned short slookahead;   /* 51 number of lookahead sectors          */
  unsigned char _bufloc;       /* 53 BUFFERS loc (1=HMA)                  */
  __DOSFAR(unsigned char)_deblock_buf;      /* 54 pointer to workspace buffer          */
  char filler2[5];             /* 58 ???/unused                           */
  unsigned char int24fail;     /* 5d int24 fail while making i/o stat call*/
  unsigned char memstrat;      /* 5e memory allocation strat during exec  */
  unsigned char a20count;      /* 5f nr. of int21 calls for which a20 off */
  unsigned char _VgaSet;        /* 60 bitflags switches=/w, int21/4b05     */
  unsigned short unpack;       /* 61 offset of unpack code start          */
  unsigned char _uppermem_link;/* 63 UMB Link flag                        */
  unsigned short min_pars;     /* 64 minimum para req by program execed   */
  unsigned short _uppermem_root;/* 66 Start of umb chain (usually 9fff)    */
  unsigned short last_para;    /* 68 para: start scanning during memalloc */
  /* ANY ITEM BELOW THIS POINT MAY CHANGE */
  /* FreeDOS specific entries */
  unsigned char _os_setver_minor;/*6a settable minor DOS version           */
  unsigned char _os_setver_major;/*6b settable major DOS version           */
  unsigned char _os_minor;     /* 6c minor DOS version                    */
  unsigned char _os_major;     /* 6d major DOS version                    */
  unsigned char rev_number;    /* 6e DOS revision#, only 3 bits           */
  unsigned char _version_flags;/* 6f DOS version flags                    */
  PTR_MEMB(char) _os_release;  /* 70 near pointer to os_release string    */
#ifdef WIN31SUPPORT
  unsigned short winInstanced; /* WinInit called                          */
  unsigned long  winStartupInfo[4];
  unsigned short instanceTable[5];
#endif
} PACKED;

#endif
