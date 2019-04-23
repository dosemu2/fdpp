/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2018  Stas Sergeev (stsp)
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
/* purpose: trace dosobjs and defer their destruction */

#include <forward_list>
#include <cassert>
#include "portab.h"
#include "dosobj.h"
#include "objtrace.hpp"

struct gc_s {
    int mark = 0;
    int idx = 0;
    std::forward_list<far_t> list;
};

static gc_s gc;

void objtrace_enter()
{
    int cnt = 0;

    /* assumption: enter is never called directly after mark */
    assert(!gc.mark);
    gc.idx++;
    if (gc.list.empty())
        return;
    /* not the right place for this, but... */
    std::for_each(gc.list.begin(), gc.list.end(), [&] (far_t f) {
        rm_dosobj(f);
        cnt++;
    });
    gc.list.clear();
    if (cnt)
        fdlogprintf("gc'ed %i objects\n", cnt);
}

void objtrace_leave()
{
    assert(gc.idx > 0);
    gc.mark = 0;
    gc.idx--;
}

void objtrace_mark()
{
    assert(gc.idx > 0);
    gc.mark = 1;
}

void objtrace_gc(far_t f)
{
    assert(gc.idx > 0);
    if (gc.mark)
        gc.list.push_front(f);
    else
        rm_dosobj(f);
}
