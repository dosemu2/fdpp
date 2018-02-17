#include <cstring>
#include "farptr.hpp"
#include "dosobj.h"

template <typename T>
class FarObj : public ObjIf {
protected:
    T *ptr;
    int size;
    __DOSFAR(uint8_t) fobj;
    bool have_obj = false;

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
    using obj_type = typename std::conditional<std::is_array<T>::value,
        typename std::remove_extent<T>::type,
        typename std::remove_const<T>::type>::type;

    template <typename T1 = T,
        typename std::enable_if<!std::is_void<T1>::value &&
            !std::is_pointer<T1>::value>::type* = nullptr>
    FarObj(T1& obj) : ptr(&obj), size(sizeof(T1)) {
        fobj = mk_dosobj(ptr, size);
    }
    FarObj(T* obj, unsigned sz) : ptr(obj), size(sz) {
        fobj = mk_dosobj(ptr, size);
    }
    virtual far_s get_obj() {
        _assert(!have_obj);
        pr_dosobj(fobj, ptr, size);
        have_obj = true;
        return fobj.get_far();
    }

    NearPtr<obj_type> get_near() {
        far_s f = get_obj();
        return NearPtr<obj_type>(f.off);
    }

    virtual ~FarObj() { RmObj(); }
};

template <typename T>
class FarObjSt : public FarObj<T> {
public:
    using obj_type = typename FarObj<T>::obj_type; // inherit type from parent

    FarObjSt(T& obj) : FarObj<T>(obj) {}
    FarObjSt(T* obj, unsigned sz) : FarObj<T>(obj, sz) {}

    virtual far_s get_obj() {
        pr_dosobj(this->fobj, this->ptr, this->size);
        return this->fobj.get_far();
    }
    NearPtr<obj_type> get_near() {
        far_s f = get_obj();
        return NearPtr<obj_type>(f.off);
    }
};

#define _RP(t) typename std::remove_pointer<t>::type
#define _RE(t) typename std::remove_extent<t>::type
#define _RR(t) typename std::remove_reference<t>::type
#define _R(o) _RP(_RE(_RR(decltype(o))))

#define _MK_FAR_ST(n, o) static FarObjSt<decltype(o)> __obj_##n(o)
#define _MK_FAR_STR_ST(n, o) static FarObjSt<_R(o)> __obj_##n(o, strlen(o))
#define MK_FAR(o) FarPtr<decltype(o)>(new FarObj<decltype(o)>(o))
#define _MK_NEAR(n, o) FarObj<decltype(o)::type> __obj_##n(o, decltype(o)::len)
#define _MK_FAR_PTR(n, o) FarObj<_R(o)> __obj_##n(*o)
#define __MK_FAR(n) FarPtr<decltype(__obj_##n)::obj_type>(__obj_##n.get_obj())
#define __MK_NEAR(n) __obj_##n.get_near()
#define _MK_FAR_STR(n, o) FarObj<_R(o)> __obj_##n(o, strlen(o))
#define _MK_FAR_SZ(n, o, sz) FarObj<_R(o)> __obj_##n(o, sz)
#define MK_FAR_SCP(o) FarPtr<decltype(o)>(FarObj<decltype(o)>(o).get_obj())
#define MK_FAR_PTR_SCP(o) FarPtr<_R(o)>(FarObj<_R(o)>(*o).get_obj())
#define MK_FAR_STR_SCP(o) FarPtr<_R(o)>(FarObj<_R(o)>(o, strlen(o)).get_obj())
