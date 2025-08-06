/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2017-2025  @stsp
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 /* Purpose: detect nasty multi-write statements. */

#include <unordered_map>
#include <cassert>
#include <cstdint>
#include "portab.h"
#include "objlock.hpp"

struct lock_s {
    int refcnt = 0;
    int lockcnt = 0;
};

typedef std::unordered_map<uint32_t, lock_s> lmap_t;
static lmap_t lmap;

static uint32_t addr(far_t fp)
{
    return (fp.seg << 4) + fp.off;
}

void objlock_ref(far_t fp)
{
    lock_s &ent = lmap[addr(fp)];  // inserts if needed
    ent.refcnt++;
}

void objlock_unref(far_t fp)
{
    uint32_t a = addr(fp);
    lock_s &ent = lmap[a];
    assert(ent.refcnt > 0);
    ent.refcnt--;
    if (!ent.refcnt)
        lmap.erase(a);
}

void objlock_lock(far_t fp)
{
    lock_s &ent = lmap[addr(fp)];
    assert(ent.refcnt > 0);
    ___assert(ent.lockcnt == 0);
    ent.lockcnt++;
}
