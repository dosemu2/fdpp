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

#include <cstring>
#include <unordered_set>
#include <algorithm>
//#include "malloca.hpp"
#include "farptr.hpp"
//#include "cppstubs.hpp"
#include "dosobj.h"
#include "objhlp.hpp"
#include "ctors.hpp"

template<typename T>
class NearPtr_DO : public NearPtr<T, dosobj_seg> {
    using NearPtr<T, dosobj_seg>::NearPtr;
};

template <typename T>
class FarObjBase {
protected:
    T *ptr;
    int size;
    __DOSFAR(uint8_t) fobj;

public:
    using obj_type = typename std::conditional<std::is_array<T>::value,
        typename std::remove_extent<T>::type, T>::type;

    FarObjBase(T* obj, unsigned sz) : ptr(obj), size(sz) {}
    virtual ~FarObjBase() = default;
};

template <typename T>
class FarObj : public FarObjBase<T>, public ObjIf, public ObjRef {
    bool have_obj = false;
    ctor_log ct;
    int refcnt = 0;
    std::unordered_set<ObjRef *> owned;
    std::unordered_set<sh_ref> owned_sh;

    void _ctor() {
        this->fobj = (__DOSFAR(uint8_t))mk_dosobj(this->ptr, this->size);
        owned = get_owned_list(this->ptr);
        owned_sh = get_owned_list_sh(this->ptr);
    }

    template <typename T1 = T,
        typename std::enable_if<!std::is_const<T1>::value>::type* = nullptr>
    void do_cp1() { cp_dosobj(this->ptr, this->fobj.get_far(), this->size); }
    template <typename T1 = T,
        typename std::enable_if<std::is_const<T1>::value>::type* = nullptr>
    void do_cp1() {}

    void own_cp() {
        std::for_each(owned.begin(), owned.end(), [] (ObjRef *o) {
            o->cp();
        });
        std::for_each(owned_sh.begin(), owned_sh.end(), [&] (sh_ref o) {
            o->cp();
        });
    }
    void own_unref() {
        std::for_each(owned.begin(), owned.end(), [] (ObjRef *o) {
            o->unref();
        });
    }
    void do_cp() { do_cp1(); own_cp(); }
    void RmObj() {
        if (!have_obj)
            return;
        do_cp();
        own_unref();
        rm_dosobj(this->fobj.get_far());
    }

public:
    using obj_type = typename FarObjBase<T>::obj_type; // inherit type from parent

    template <typename T1 = T,
        typename std::enable_if<!std::is_void<T1>::value &&
            !std::is_pointer<T1>::value>::type* = nullptr>
    FarObj(T1& obj, const char *nm) : FarObjBase<T>(&obj, sizeof(T1)), ct(nm) {
        _ctor();
    }
    FarObj(T* obj, unsigned sz, const char *nm) : FarObjBase<T>(obj, sz), ct(nm) {
        _ctor();
    }
    virtual far_s get_obj() {
        _assert(!have_obj);
        pr_dosobj(this->fobj.get_far(), this->ptr, this->size);
        have_obj = true;
        return this->fobj.get_far();
    }
    virtual void ref(const void *owner) {
        if (track_owner(owner, this))
            refcnt++;
    }
    virtual void cp() { do_cp(); }
    virtual void unref() { refcnt--; }

    NearPtr_DO<obj_type> get_near() {
        far_s f = get_obj();
        return NearPtr_DO<obj_type>(f.off);
    }

    virtual ~FarObj() { _assert(!refcnt); RmObj(); }

    /* make it non-copyable */
    FarObj(const FarObj &) = delete;
    FarObj& operator =(const FarObj &) = delete;
};

#define _RP(t) typename std::remove_pointer<t>::type
#define _RE(t) typename std::remove_extent<t>::type
//#define _RR(t) typename std::remove_reference<t>::type
#define _R(o) _RP(_RE(_RR(decltype(o))))
#define _RC(t) typename std::remove_const<_R(t)>::type

#define MK_FAR(o) FarPtr<decltype(o)>(_MK_FAR(o))
#define MK_FAR_SZ(o, sz) \
    FarPtr<_RC(o)>(std::make_shared<FarObj<_R(o)>>(o, sz))
#define MK_NEAR_OBJ(p, o) ({ \
    std::shared_ptr<FarObj<decltype(o)>> _sh = std::make_shared<FarObj<decltype(o)>>(o, NM); \
    track_owner_sh(p, _sh); \
    _sh->get_near(); \
})
#define MK_NEAR_STR_OBJ(p, o) ({ \
    std::shared_ptr<FarObj<_R(o)>> _sh = \
        std::make_shared<FarObj<_R(o)>>(o, strlen(o) + 1, NM); \
    track_owner_sh(p, _sh); \
    ((FarObj<_RC(o)> *)_sh.get())->get_near(); \
})
#define MK_FAR_PTR_SCP(o) FarPtr<_RC(o)>(FarObj<_R(o)>(*o, NM).get_obj())
#define __MK_FAR(n) FarPtr<decltype(__obj_##n)::obj_type>(__obj_##n.get_obj())
#define __MK_NEAR(n) __obj_##n.get_near()
#define __MK_NEAR2(n, t) t(__obj_##n.get_near().off())
#define _MK_FAR_STR(n, o) FarObj<_R(o)> __obj_##n(o, strlen(o) + 1, NM)
#define MK_FAR_STR_OBJ(p, o) ({ \
    std::shared_ptr<ObjRef> _sh = \
        std::make_shared<FarObj<_R(o)>>(o, strlen(o) + 1, NM); \
    track_owner_sh(p, _sh); \
    FarPtrBase<_RC(o)> (((FarObj<_R(o)> *)_sh.get())->get_obj()); \
})
#define _MK_FAR_SZ(n, o, sz) FarObj<_R(o)> __obj_##n(o, sz, NM)
#define MK_FAR_SZ_OBJ(p, o, sz) ({ \
    std::shared_ptr<ObjRef> _sh = std::make_shared<FarObj<_R(o)>>(o, sz, NM); \
    track_owner_sh(p, _sh); \
    FarPtrBase<_RC(o)> (((FarObj<_R(o)> *)_sh.get())->get_obj()); \
})
#define MK_FAR_SCP(o) FarPtr<decltype(o)>(FarObj<decltype(o)>(o, NM).get_obj())
#define MK_FAR_STR_SCP(o) FarPtr<_RC(o)>(FarObj<_R(o)>(o, strlen(o) + 1, NM).get_obj())
#define MK_FAR_SZ_SCP(o, sz) FarPtr<_RC(o)>(FarObj<_R(o)>(o, sz, NM).get_obj())

#define PTR_MEMB(t) NearPtr_DO<t>
#define NEAR_PTR_DO(t) NearPtr_DO<t>
