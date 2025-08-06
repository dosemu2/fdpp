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

#ifndef OBJLOCK_HPP
#define OBJLOCK_HPP

void objlock_ref(far_t fp);
void objlock_unref(far_t fp);
void objlock_lock(far_t fp);

#endif
