/****************************************************************/
/*                                                              */
/*                          strings.c                           */
/*                                                              */
/*                Global String Handling Functions              */
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
static BYTE *stringsRcsId =
    "$Id: strings.c 653 2003-08-09 09:35:18Z bartoldeman $";
#endif

#ifndef USE_STDLIB
size_t strlen(REG CONST BYTE * s)
{
  REG size_t cnt = 0;

  while (*s++ != 0)
    ++cnt;
  return cnt;
}
#endif

COUNT fstrlen(REG CONST BYTE FAR * s)
{
  REG size_t cnt = 0;

  while (*s++ != 0)
    ++cnt;
  return cnt;
}

#ifndef USE_STDLIB
int strcmp(REG CONST BYTE * d, REG CONST BYTE * s)
{
  while (*s != '\0' && *d != '\0')
  {
    if (*d == *s)
      ++s, ++d;
    else
      return *d - *s;
  }
  return *d - *s;
}
#endif

int fstrcmp(CONST BYTE FAR * d, CONST BYTE FAR * s)
{
  while (*s != '\0' && *d != '\0')
  {
    if (*d == *s)
      ++s, ++d;
    else
      return *d - *s;
  }
  return *d - *s;
}

int fstrcmp_n(const char * d, CONST BYTE FAR * s)
{
  while (*s != '\0' && *d != '\0')
  {
    if (*d == *s)
      ++s, ++d;
    else
      return *d - *s;
  }
  return *d - *s;
}

#ifndef USE_STDLIB
int strncmp(REG const char *d, REG const char *s, size_t l)
{
  size_t index = 1;
  while (*s != '\0' && *d != '\0' && index++ <= l)
  {
    if (*d == *s)
      ++s, ++d;
    else
      return *d - *s;
  }
  return *d - *s;
}
#endif

COUNT fstrncmp(REG BYTE FAR * d, REG BYTE FAR * s, COUNT l)
{
  COUNT index = 1;
  while (*s != '\0' && *d != '\0' && index++ <= l)
  {
    if (*d == *s)
      ++s, ++d;
    else
      return *d - *s;
  }
  return *d - *s;
}

#ifndef USE_STDLIB
char *strchr(const char * s, int c)
{
  REG CONST BYTE *p;
  p = s - 1;
  do
  {
    if (*++p == (char)c)
      return (char *)p;
  }
  while (*p);
  return 0;
}
#endif

VOID fstrcpy(char FAR * d, const char FAR * s)
{
  fmemcpy(d, s, fstrlen(s) + 1);
}

VOID n_fstrcpy(char FAR * d, const char * s)
{
  n_fmemcpy(d, s, strlen(s) + 1);
}

VOID fstrcpy_n(char * d, const char FAR * s)
{
  fmemcpy_n(d, s, fstrlen(s) + 1);
}

char FAR * fstrchr(const char FAR * s, int c)
{
  CONST BYTE FAR *p;
  p = s - 1;
  do
  {
    if (*++p == (char)c)
      return (char FAR *)p;
  }
  while (*p);
  return NULL;
}

#ifndef USE_STDLIB
void *memchr(const void * s, int c, size_t n)
{
  const unsigned char *p = s;
  while (n--)
  {
    if (*p == (unsigned char)c)
      return (void *)p;
    p++;
  }
  return NULL;
}
#endif

void FAR * fmemchr(const void FAR * s, int c, size_t n)
{
  const unsigned char FAR *p = s;
  while (n--)
  {
    if (*p == (unsigned char)c)
      return (void FAR *)p;
    p++;
  }
  return NULL;
}

#ifndef USE_STDLIB
int memcmp(CONST VOID * c_d, CONST VOID * c_s, size_t n)
{
  CONST BYTE * d = (CONST BYTE *)c_d;
  CONST BYTE * s = (CONST BYTE *)c_s;

  while (n--)
  {
    if (*d == *s)
      ++s, ++d;
    else
      return *d - *s;
  }
  return 0;
}
#endif

int fmemcmp(CONST VOID FAR * c_d, CONST VOID FAR * c_s, size_t n)
{
  CONST BYTE FAR *d = (CONST BYTE FAR *)c_d;
  CONST BYTE FAR *s = (CONST BYTE FAR *)c_s;

  while (n--)
  {
    if (*d == *s)
      ++s, ++d;
    else
      return *d - *s;
  }
  return 0;
}

int n_fmemcmp(CONST VOID FAR * c_d, CONST VOID * c_s, size_t n)
{
  CONST BYTE FAR *d = (CONST BYTE FAR *)c_d;
  CONST BYTE *s = (CONST BYTE *)c_s;

  while (n--)
  {
    if (*d == *s)
      ++s, ++d;
    else
      return *d - *s;
  }
  return 0;
}
