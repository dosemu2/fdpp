/****************************************************************/
/*                                                              */
/*                             pcb.h                            */
/*                                                              */
/*           Process Control and Interrupt data structures      */
/*                                                              */
/*                       November 26, 1991                      */
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

#ifdef MAIN
#ifdef VERSION_STRINGS
static BYTE *pcb_hRcsId = "$Id$";
#endif
#endif

/*
 * $Log$
 * Revision 1.2  2000/05/08 04:28:22  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  1999/03/29 15:39:32  jprice
 * New version without IPL.SYS
 *
 * Revision 1.3  1999/02/01 01:40:06  jprice
 * Clean up
 *
 * Revision 1.2  1999/01/22 04:17:40  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *         Rev 1.6   07 Feb 1998 20:47:40   patv
 *      Modified stack frame to match DOS standard
 *
 *         Rev 1.5   04 Jan 1998 23:14:18   patv
 *      Changed Log for strip utility
 *
 *         Rev 1.4   29 May 1996 21:25:16   patv
 *      bug fixes for v0.91a
 *
 *         Rev 1.3   19 Feb 1996  3:15:32   patv
 *      Added NLS, int2f and config.sys processing
 *
 *         Rev 1.2   01 Sep 1995 17:35:44   patv
 *      First GPL release.
 *
 *         Rev 1.1   30 Jul 1995 20:43:50   patv
 *      Eliminated version strings in ipl
 *
 *         Rev 1.0   02 Jul 1995 10:39:48   patv
 *      Initial revision.
 */

/*                                                                      */
/* interrupt handler structure definition                               */
/*                                                                      */
typedef union
{
  UWORD x;                      /* access mode for ax, bx, etc.         */
  struct
  {
    UBYTE l;                    /* access mode for al, bl, etc.         */
    UBYTE h;                    /* access mode for ah, bh, etc.         */
  }
  b;
}
xreg;

/* maps MS-DOS unique stacking order                                    */
typedef struct
{
  xreg a,
    b,
    c,
    d;
  UWORD si,
    di,
    bp,
    ds,
    es;
  UWORD ip,
    cs,
    flags;
}
iregs;

/* Process control block for task switching                             */
typedef struct
{
  UWORD pc_ss;
  UWORD pc_sp;
  iregs pc_regs;
}
pcb;

/* For MSC, the following offsets must match the assembly process       */
/* support offsets                                                      */
/* NOTE: Alignemnts must be set to 1 (-Zp1)                             */
/*      ss:     0                                                       */
/*      sp:     2                                                       */
/*      es:     4                                                       */
/*      ds:     6                                                       */
/*      di:     8                                                       */
/*      si:     10                                                      */
/*      bp:     12                                                      */
/*      sp:     14      NOTE: not used in this structure                */
/*      bx:     16                                                      */
/*      dx:     18                                                      */
/*      cx:     20                                                      */
/*      ax:     22                                                      */
/*      ip:     24                                                      */
/*      cs:     26                                                      */
/*      flags:  28                                                      */
/*                                                                      */
/* For Borland C, the following offsets must match the assembly process */
/* support offsets                                                      */
/*      ss:     0                                                       */
/*      sp:     2                                                       */
/*      bp:     4                                                       */
/*      di:     6                                                       */
/*      si:     8                                                       */
/*      ds:     10                                                      */
/*      es:     12                                                      */
/*      dx:     14                                                      */
/*      cx:     16                                                      */
/*      bx:     18                                                      */
/*      ax:     20                                                      */
/*      ip:     22                                                      */
/*      cs:     24                                                      */
/*      flags:  26                                                      */

#define BP      bp
#define DI      di
#define SI      si
#define DS      ds
#define ES      es
#define DX      d.x
#define CX      c.x
#define BX      b.x
#define AX      a.x
#define DH      d.b.h
#define CH      c.b.h
#define BH      b.b.h
#define AH      a.b.h
#define DL      d.b.l
#define CL      c.b.l
#define BL      b.b.l
#define AL      a.b.l
#define IP      ip
#define CS      cs
#define FLAGS   flags

#define FLG_ZERO        0x0040
#define FLG_CARRY       0x0001
