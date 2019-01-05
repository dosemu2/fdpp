/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2019  Stas Sergeev (stsp)
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

#include <forward_list>
#include <algorithm>
#include "ctors.hpp"

static std::forward_list<ctor_base*>& ctor_list()
{
    /* https://isocpp.org/wiki/faq/ctors#static-init-order-on-first-use */
    static std::forward_list<ctor_base*> *lst = new
	    std::forward_list<ctor_base*>();
    return *lst;
}

void add_ctor(ctor_base *ptr)
{
    ctor_list().push_front(ptr);
}

void run_ctors()
{
    std::for_each(ctor_list().begin(), ctor_list().end(), [] (ctor_base *c) {
        c->init();
    });
}
