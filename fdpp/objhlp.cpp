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

/* silly farobj helpers */

#include <unordered_set>
#include <unordered_map>
#include "objhlp.hpp"

static std::unordered_map<const void *, std::unordered_set<ObjRef *> > omap;

bool track_owner(const void *owner, ObjRef *obj)
{
    std::unordered_set<ObjRef *>& ent = omap[owner];
    return ent.insert(obj).second;
}

std::unordered_set<ObjRef *> get_owned_list(const void *owner)
{
    std::unordered_set<ObjRef *> ret;
    if (omap.find(owner) != omap.end()) {
        ret = omap[owner];
        omap.erase(owner);
    }
    return ret;
}
