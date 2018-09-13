/****************************************************************/
/*                                                              */
/*                          initoem.c                           */
/*                                                              */
/*                  OEM Initializattion Functions               */
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
#include "init-mod.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId =
    "$Id: initoem.c 1321 2007-05-21 02:16:36Z bartoldeman $";
#endif

#define EBDASEG 0x40e
#define RAMSIZE 0x413

unsigned init_oem(void)
{
  iregs r = {};
  init_call_intr(0x12, &r);
  return r.a.x;
}

void movebda(size_t bytes, UWORD new_seg)
{
  UWORD old_seg = peekw(0, EBDASEG);
  fmemcpy(MK_FP(new_seg, 0), MK_FP(old_seg, 0), bytes);
  pokew(0, EBDASEG, new_seg);
  pokew(0, RAMSIZE, ram_top);
}

unsigned ebdasize(void)
{
  unsigned ebdaseg = peekw(0, EBDASEG);
  unsigned ramsize = ram_top;

  if (ramsize == peekw(0, RAMSIZE))
    if (ramsize * 64 == ebdaseg && ramsize < 640 && peekw(0, RAMSIZE) == ramsize)
    {
    unsigned ebdasz = peekb(ebdaseg, 0);

    /* sanity check: is there really no more than 63 KB?
     * must be at 640k (all other values never seen and are untested)
     */
    if (ebdasz <= 63 && ramsize + ebdasz == 640)
      return ebdasz * 1024U;
    }
  return 0;
}
