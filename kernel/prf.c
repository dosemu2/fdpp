/****************************************************************/
/*                                                              */
/*                            prf.c                             */
/*                                                              */
/*                  Abbreviated printf Function                 */
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
#include "pcb.h"
#include "globals.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"


static char large_buf[1024];

void put_console(int c)
{
  if (c == '\n')
    put_console('\r');

  iregs r = {};
  r.a.b.l = c;
  call_intr(0x29, MK_FAR_SCP(r));
}

void hexd(const char *title, VOID FAR * v_p, COUNT numBytes)
{
  UBYTE FAR * p = v_p;
  int loop, start = 0;
  put_string(title);
  if (numBytes > 16)
    put_console('\n');

  for (start = 0; start < numBytes; start += 16)
  {
    put_unsigned(FP_SEG(p), 16, 4);
    put_console(':');
    put_unsigned(FP_OFF(p + start), 16, 4);
    put_console('|');
    for (loop = start; loop < numBytes && loop < start+16;loop++)
    {
      put_unsigned(p[loop], 16, 2);
      put_console(' ');
    }
    for (loop = start; loop < numBytes && loop < start+16;loop++)
      put_console(p[loop] < 0x20 ? '.' : p[loop]);
    put_console('\n');
  }
}

/* put_unsigned -- print unsigned int in base 2--16 */
void put_unsigned(unsigned n, int base, int width)
{
  char s[6];
  int i;

  for (i = 0; i < width; i++)
  {                             /* generate digits in reverse order */
    s[i] = "0123456789abcdef"[(UWORD) (n % base)];
    n /= base;
  }

  while(i != 0)
  {                             /* print digits in reverse order */
    put_console(s[--i]);
  }
}

void put_string(const char *s)
{
  while(*s != '\0')
    put_console(*s++);
}

int _printf(const char *fmt, ...)
{
  va_list va;
  int ret;

  va_start(va, fmt);
  ret = stbsp_vsnprintf(large_buf, sizeof(large_buf), fmt, va);
  va_end(va);

  put_string(large_buf);

  return ret;
}

int _vprintf(const char *fmt, va_list va)
{
  return stbsp_vsnprintf(large_buf, sizeof(large_buf), fmt, va);
}

int _sprintf(char *buf, const char *fmt, ...)
{
  va_list va;
  int ret;

  va_start(va, fmt);
  ret = stbsp_vsnprintf(buf, /* FIXME, what size should this be? */ 120, fmt, va);
  va_end(va);

  return ret;
}

int _snprintf(char *buf, size_t size, const char *fmt, ...)
{
  va_list va;
  int ret;

  va_start(va, fmt);
  ret = stbsp_vsnprintf(buf, size, fmt, va);
  va_end(va);

  return ret;
}

int _vsnprintf(char *buf, size_t size, const char *fmt, va_list va)
{
  return stbsp_vsnprintf(buf, size, fmt, va);
}
