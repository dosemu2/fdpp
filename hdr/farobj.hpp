#include <cstring>
#include <cassert>
#include "farptr.hpp"
#include "dosobj.h"

template <typename T>
class FarObj {
protected:
    T *ptr;
    int size;
    FarPtr<void> fobj;
    bool have_obj = false;

    template <typename T1>
    FarPtr<T1> GetObj() {
        assert(!have_obj);
        pr_dosobj(fobj, ptr, size);
        have_obj = true;
        return fobj;
    }

    template <typename T1 = T,
        typename std::enable_if<std::is_const<T1>::value>::type* = nullptr>
    void RmObj() {
        if (!have_obj)
            return;
        rm_dosobj(fobj);
    }
    template <typename T1 = T,
        typename std::enable_if<!std::is_const<T1>::value>::type* = nullptr>
    void RmObj() {
        if (!have_obj)
            return;
        cp_dosobj(ptr, fobj, size);
        rm_dosobj(fobj);
    }

public:
    template <typename T1 = T,
        typename std::enable_if<!std::is_void<T1>::value &&
            !std::is_pointer<T1>::value>::type* = nullptr>
    FarObj(T1& obj) : ptr(&obj), size(sizeof(T1)) {
        fobj = mk_dosobj(ptr, size);
    }
    FarObj(T* obj, unsigned sz) : ptr(obj), size(sz) {
        fobj = mk_dosobj(ptr, size);
    }
    template <typename T1 = T,
        typename std::enable_if<std::is_array<T1>::value>::type* = nullptr>
    FarPtr<typename std::remove_extent<T1>::type> get_obj() {
        return GetObj<typename std::remove_extent<T1>::type>();
    }
    template <typename T1 = T,
        typename std::enable_if<!std::is_array<T1>::value>::type* = nullptr>
    FarPtr<typename std::remove_const<T1>::type> get_obj() {
        return GetObj<typename std::remove_const<T1>::type>();
    }

    template <typename T1 = T,
        typename std::enable_if<std::is_array<T1>::value>::type* = nullptr>
    NearPtr<typename std::remove_extent<T1>::type> get_near() {
        FarPtr<typename std::remove_extent<T1>::type> f = get_obj();
        return NearPtr<typename std::remove_extent<T1>::type>(f.__off());
    }
    template <typename T1 = T,
        typename std::enable_if<!std::is_array<T1>::value>::type* = nullptr>
    NearPtr<typename std::remove_const<T1>::type> get_near() {
        FarPtr<typename std::remove_const<T1>::type> f = get_obj();
        return NearPtr<typename std::remove_const<T1>::type>(f.__off());
    }

    ~FarObj() { RmObj(); }
};

template <typename T>
class FarObjSt : public FarObj<T> {
public:
    FarObjSt(T& obj) : FarObj<T>(obj) {}
    FarObjSt(T* obj, unsigned sz) : FarObj<T>(obj, sz) {}
    template <typename T1 = T,
        typename std::enable_if<!std::is_array<T1>::value>::type* = nullptr>
    FarPtr<typename std::remove_const<T1>::type> get_obj() {
        pr_dosobj(this->fobj, this->ptr, this->size);
        return this->fobj;
    }
    template <typename T1 = T,
        typename std::enable_if<std::is_array<T1>::value>::type* = nullptr>
    FarPtr<typename std::remove_extent<T1>::type> get_obj() {
        pr_dosobj(this->fobj, this->ptr, this->size);
        return this->fobj;
    }

    template <typename T1 = T,
        typename std::enable_if<!std::is_array<T1>::value>::type* = nullptr>
    NearPtr<typename std::remove_const<T1>::type> get_near() {
        FarPtr<typename std::remove_const<T1>::type> f = get_obj();
        return NearPtr<typename std::remove_const<T1>::type>(f.__off());
    }
    template <typename T1 = T,
        typename std::enable_if<std::is_array<T1>::value>::type* = nullptr>
    NearPtr<typename std::remove_extent<T1>::type> get_near() {
        FarPtr<typename std::remove_extent<T1>::type> f = get_obj();
        return NearPtr<typename std::remove_extent<T1>::type>(f.__off());
    }

};

#define _RP(t) typename std::remove_pointer<t>::type
#define _RE(t) typename std::remove_extent<t>::type
#define _RR(t) typename std::remove_reference<t>::type
#define _R(o) _RP(_RE(_RR(decltype(o))))

#define _MK_FAR_ST(n, o) static FarObjSt<decltype(o)> __obj_##n(o)
#define _MK_FAR_STR_ST(n, o) static FarObjSt<_R(o)> __obj_##n(o, strlen(o))
#define _MK_FAR(n, o) FarObj<decltype(o)> __obj_##n(o)
#define _MK_NEAR(n, o) FarObj<decltype(o)::type> __obj_##n(o, decltype(o)::len)
#define _MK_FAR_PTR(n, o) FarObj<_R(o)> __obj_##n(*o)
#define __MK_FAR(n) __obj_##n.get_obj()
#define __MK_NEAR(n) __obj_##n.get_near()
#define _MK_FAR_STR(n, o) FarObj<_R(o)> __obj_##n(o, strlen(o))
#define _MK_FAR_SZ(n, o, sz) FarObj<_R(o)> __obj_##n(o, sz)
