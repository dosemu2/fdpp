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

#ifndef OBJHLP_HPP
#define OBJHLP_HPP

#include <memory>

class ObjRef {
public:
    virtual void cp() = 0;
    virtual void unref() = 0;
    virtual ~ObjRef() = default;
};

bool track_owner(const void *owner, ObjRef *obj);
std::unordered_set<ObjRef *> get_owned_list(const void *owner);

typedef std::shared_ptr<ObjRef> sh_ref;
bool track_owner_sh(const void *owner, sh_ref obj);
std::unordered_set<sh_ref> get_owned_list_sh(const void *owner);
void objhlp_reset();

#endif
