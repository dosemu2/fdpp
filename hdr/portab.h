/****************************************************************/
/*                                                              */
/*                           portab.h                           */
/*                                                              */
/*                 DOS-C portability typedefs, etc.             */
/*                                                              */
/*                         May 1, 1995                          */
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
static char *portab_hRcsId =
    "$Id$";
#endif
#endif

/****************************************************************/
/*                                                              */
/* Machine dependant portable types. Note that this section is  */
/* used primarily for segmented architectures. Common types and */
/* types used relating to segmented operations are found here.  */
/*                                                              */
/* Be aware that segmented architectures impose on linear       */
/* architectures because they require special types to be used  */
/* throught the code that must be reduced to empty preprocessor */
/* replacements in the linear machine.                          */
/*                                                              */
/* #ifdef <segmeted machine>                                    */
/* # define FAR far                                             */
/* # define NEAR near                                           */
/* #endif                                                       */
/*                                                              */
/* #ifdef <linear machine>                                      */
/* # define FAR                                                 */
/* # define NEAR                                                */
/* #endif                                                       */
/*                                                              */
/****************************************************************/

                                                        /* commandline overflow - removing -DI86 TE */
#if defined(__TURBOC__)

#define I86
#define CDECL   cdecl
void __int__(int);

#elif defined	(_MSC_VER)

#define I86
#define CDECL   _cdecl
#define __int__(intno) asm int intno;

#if defined(M_I286)             /* /G3 doesn't set M_I386, but sets M_I286 TE */
#define I386
#endif

#elif defined(__WATCOMC__)      /* don't know a better way */

#define I86
#define __int__(intno) asm int intno;
#define asm __asm
#define far __far
#define CDECL   __cdecl

#if _M_IX86 >= 300
#define I386
#endif

#elif defined (_MYMC68K_COMILER_)

#define MC68K

#elif defined(__GNUC__)
/* for warnings only ! */
#define MC68K

#else
anyone knows a _portable_ way to create nice errors ? ?
    at least this causes the compiler not to compile : -)
#endif

#ifdef MC68K
#define far                     /* No far type          */
#define interrupt               /* No interrupt type    */
#define VOID           void
#define FAR                     /* linear architecture  */
#define NEAR                    /*    "        "        */
#define INRPT          interrupt
#define REG            register
#define API            int      /* linear architecture  */
#define NONNATIVE
#define PARASIZE       4096     /* "paragraph" size     */
#define CDECL
#ifdef __GNUC__
#define CONST          const
#define PROTO
typedef __SIZE_TYPE__  size_t;
#else
#define CONST
typedef unsigned       size_t;
#endif
#endif
#ifdef I86
#define VOID           void
#define FAR            far      /* segment architecture */
#define NEAR           near     /*    "          "      */
#define INRPT          interrupt
#define CONST          const
#define REG            register
#define API            int far pascal   /* segment architecture */
#define NATIVE
#define PARASIZE       16       /* "paragraph" size     */
typedef unsigned       size_t;
#endif
           /* functions, that are shared between C and ASM _must_ 
              have a certain calling standard. These are declared
              as 'ASMCFUNC', and is (and will be ?-) cdecl */
#define ASMCFUNC CDECL
#define ASM ASMCFUNC
/*                                                              */
/* Boolean type & definitions of TRUE and FALSE boolean values  */
/*                                                              */
typedef int BOOL;
#define FALSE           (1==0)
#define TRUE            (1==1)

/*                                                              */
/* Common pointer types                                         */
/*                                                              */
#ifndef NULL
#define NULL            0
#endif

/*                                                              */
/* Convienence defines                                          */
/*                                                              */
#define FOREVER         while(TRUE)
#ifndef max
#define max(a,b)       (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)       (((a) < (b)) ? (a) : (b))
#endif

/*                                                              */
/* Common byte, 16 bit and 32 bit types                         */
/*                                                              */
typedef char BYTE;
typedef short WORD;
typedef long DWORD;

typedef unsigned char UBYTE;
typedef unsigned short UWORD;
typedef unsigned long UDWORD;

typedef short SHORT;

typedef unsigned int BITS;      /* for use in bit fields(!)     */

typedef int COUNT;
typedef unsigned int UCOUNT;
typedef unsigned long ULONG;

#ifdef WITHFAT32
typedef unsigned long CLUSTER;
#else
typedef unsigned short CLUSTER;
#endif
typedef unsigned short UNICODE;

#ifdef STATICS
#define STATIC static		 /* local calls inside module */
#else
#define STATIC
#endif

#ifdef UNIX
typedef char FAR *ADDRESS;
#else
typedef void FAR *ADDRESS;
#endif

#ifdef STRICT
typedef signed long LONG;
#else
#define LONG long
#endif

/* General far pointer macros                                           */
#ifdef I86
#ifndef MK_FP
#ifdef __WATCOMC__
#define MK_FP(seg,ofs) 	      (((UWORD)(seg)):>((VOID *)(ofs)))
#else
#define MK_FP(seg,ofs)        ((void FAR *)(((ULONG)(seg)<<16)|(UWORD)(ofs)))
#endif
#define FP_SEG(fp)            ((unsigned)(UWORD)((ULONG)(VOID FAR *)(fp)>>16))
#define FP_OFF(fp)            ((unsigned)(UWORD)(fp))
#endif
#endif

#ifdef MC68K
#define MK_FP(seg,ofs)         ((VOID *)(&(((BYTE *)(size_t)(seg))[(ofs)])))
#define FP_SEG(fp)             (0)
#define FP_OFF(fp)             ((size_t)(fp))
#endif

typedef VOID (FAR ASMCFUNC * intvec) (void);

/*
	this suppresses the warning 
	unreferenced parameter 'x'
	and (hopefully) generates no code
*/

#if defined(__TURBOC__)
#define UNREFERENCED_PARAMETER(x) if (x);
#elif defined(__GNUC__)
#define UNREFERENCED_PARAMETER(x) x = 0;
#else
#define UNREFERENCED_PARAMETER(x) x;
#endif

#ifdef I86                      /* commandline overflow - removing /DPROTO TE */
#define PROTO
#endif

/*
 * Log: portab.h,v 
 * Revision 1.2  1999/08/25 03:17:11  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.1.1.1  1999/03/29 15:39:33  jprice
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
 *         Rev 1.5   04 Jan 1998 23:14:16   patv
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
 *         Rev 1.0   02 Jul 1995 10:39:50   patv
 *      Initial revision.
 */
