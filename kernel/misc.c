/****************************************************************/
/*                                                              */
/*                           misc.c                             */
/*                                                              */
/*                 Miscellaneous Kernel Functions               */
/*                                                              */
/*                      Copyright (c) 1993                      */
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

#ifdef VERSION_STRINGS
static BYTE *miscRcsId =
    "$Id: misc.c 653 2003-08-09 09:35:18Z bartoldeman $";
#endif

#include "globals.h"
#ifndef I86

#ifndef USE_STDLIB
VOID * memcpy(REG VOID * d, REG CONST VOID * s, REG size_t n)
{
  char *cd = (char *)d;
  CONST char *cs = (CONST char *)s;

  while (n--)
    *cd++ = *cs++;
  return d;
}
#endif

VOID fmemcpy(VOID FAR * d, CONST VOID FAR * s, size_t n)
{
  thunk_fmemcpy(GET_FAR(d), GET_FAR(s), n);
}

VOID n_fmemcpy(VOID FAR * d, CONST VOID * s, size_t n)
{
  thunk_n_fmemcpy(GET_FAR(d), s, n);
}

#if 0
VOID fmemmove(REG VOID FAR * d, REG CONST VOID FAR * s, REG size_t n)
{
  if (d < s) {
    size_t l;
    for (l = 0; l < n; l++)
      ((BYTE FAR *) d)[l] = ((BYTE FAR *) s)[l];
  } else {
    while (n--)
      ((BYTE FAR *) d)[n] = ((BYTE FAR *) s)[n];
  }
}
#endif

#ifndef USE_STDLIB
VOID fmemcpy_n(REG VOID * d, REG CONST VOID FAR * s, REG size_t n)
{
  while (n--)
    ((BYTE *) d)[n] = ((BYTE FAR *) s)[n];
}

VOID *memset(VOID * s, int ch, size_t n)
{
  while (n--)
    ((BYTE *) s)[n] = ch;
  return s;
}
#endif

VOID fmemset(VOID FAR * d, int ch, size_t n)
{
  thunk_fmemset(GET_FAR(d), ch, n);
}

#endif

