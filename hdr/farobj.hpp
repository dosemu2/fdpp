#include <string.h>
#include "farptr.hpp"

template <typename T>
class FarObj {
    T *ptr;
    int size;

public:
    template <typename T1 = T,
        typename std::enable_if<!std::is_void<T1>::value &&
            !std::is_pointer<T1>::value>::type* = nullptr>
        FarObj(T1& obj) : ptr(&obj), size(sizeof(T1)) {}
    FarObj(T* obj, unsigned sz) : ptr(obj), size(sz) {}
    template <typename T1 = T,
        typename std::enable_if<std::is_array<T1>::value>::type* = nullptr>
        FarPtr<typename std::remove_extent<T1>::type> get_obj();
    template <typename T1 = T,
        typename std::enable_if<!std::is_array<T1>::value>::type* = nullptr>
        FarPtr<T1> get_obj();
    ~FarObj();
};

template <typename T>
class FarObjSt {
public:
    FarObjSt(T& obj);
    FarObjSt(T* obj, unsigned sz);
    FarPtr<T> get_obj();
};

#define _RP(t) typename std::remove_pointer<t>::type
#define _RE(t) typename std::remove_extent<t>::type
#define _RR(t) typename std::remove_reference<t>::type
#define _R(o) _RP(_RE(_RR(decltype(o))))

#define _MK_FAR_ST(n, o) static FarObjSt<decltype(o)> __obj_##n(o)
#define _MK_FAR_ST_STR(n, o) static FarObjSt<_R(o)> __obj_##n(o, strlen(o))
#define _MK_FAR(n, o) FarObj<decltype(o)> __obj_##n(o)
#define _MK_FAR_PTR(n, o) FarObj<_R(o)> __obj_##n(*o)
#define __MK_FAR(n) __obj_##n.get_obj()
#define _MK_FAR_STR(n, o) FarObj<_R(o)> __obj_##n(o, strlen(o))
#define _MK_FAR_SZ(n, o, sz) FarObj<_R(o)> __obj_##n(o, sz)
