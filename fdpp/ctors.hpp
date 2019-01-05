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

#ifndef CTORS_HPP
#define CTORS_HPP

#include <cstring>

class ctor_base;

void add_ctor(ctor_base *ptr);
void rm_ctor(ctor_base *ptr);
void run_ctors();

class ctor_base {
public:
    virtual void init() = 0;
    ctor_base() { add_ctor(this); }
    virtual ~ctor_base() { rm_ctor(this); }
};

template <typename T>
class ctor : public ctor_base {
    T *ptr;
    const T holder;
public:
    ctor(T *p, const T i) : ptr(p), holder(i) {}
    virtual void init() { *ptr = holder; }
};

template <typename T, int L>
class ctor_a : public ctor_base {
    T *ptr;
public:
    ctor_a(T *p) : ptr(p) {}
    virtual void init() { std::memset(ptr, 0, sizeof(T) * L); }
};

template <typename T>
class ctor_z : public ctor_base {
    T *ptr;
public:
    ctor_z(T *p) : ptr(p) {}
    virtual void init() { std::memset(ptr, 0, sizeof(T)); }
};

#define CTOR(t, n, i) t n; static ctor<t> _ctor_##n(&n, i)
#define CTORA(t, n, l) t n[l]; static ctor_a<t,l> _ctor_##n(n)
#define CTORZ(t, n) t n; static ctor_z<t> _ctor_##n(&n)

#endif
