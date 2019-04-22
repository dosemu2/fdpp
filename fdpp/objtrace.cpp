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
#include <vector>
#include <cassert>
#include "portab.h"
#include "dosobj.h"
#include "objtrace.hpp"

struct gc_s {
    int mark = 0;
    std::forward_list<far_t> list;
};

static std::vector<gc_s> gc_vec;
static int gc_idx;

void objtrace_enter()
{
    gc_s *gc;
    int cnt = 0;

    gc_idx++;
    if (gc_vec.size() < gc_idx)
        gc_vec.resize(gc_idx);
    gc = &gc_vec[gc_idx - 1];
    if (!gc->mark) {
        assert(gc->list.empty());
        return;
    }
    /* not the right place for this, but... */
    std::for_each(gc->list.begin(), gc->list.end(), [&] (far_t f) {
        rm_dosobj(f);
        cnt++;
    });
    gc->list.clear();
    gc->mark = 0;
    if (cnt)
        fdlogprintf("gc'ed %i objects\n", cnt);
}

void objtrace_leave()
{
    assert(gc_idx > 0);
    gc_idx--;
}

void objtrace_mark()
{
    gc_s *gc;

    assert(gc_idx > 0);
    gc = &gc_vec[gc_idx - 1];
    gc->mark = 1;
}

void objtrace_gc(far_t f)
{
    gc_s *gc;

    assert(gc_idx > 0);
    gc = &gc_vec[gc_idx - 1];
    if (gc->mark)
        gc->list.push_front(f);
    else
        rm_dosobj(f);
}
