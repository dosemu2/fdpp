#include "farptr.hpp"

template <typename T>
class FarObj {
public:
    template <typename T1 = T,
        typename std::enable_if<!std::is_void<T1>::value>::type* = nullptr>
        FarObj(const T1& obj);
    FarObj(const T* obj, unsigned size);
    FarObj(const char *str);
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
    FarObjSt(const T& obj);
    FarObjSt(const char *str);
    FarPtr<T> get_obj();
};

#define _MK_FAR_ST(n, o) static FarObjSt<decltype(o)> __obj_##n(o)
#define _MK_FAR_ST_STR(n, o) static FarObjSt<char> __obj_##n(o)
#define _MK_FAR(n, o) FarObj<decltype(o)> __obj_##n(o)
#define _MK_FAR_PTR(n, o) FarObj<typename std::remove_pointer<decltype(o)>::type> __obj_##n(*o)
#define __MK_FAR(n) __obj_##n.get_obj()
#define _MK_FAR_STR(n, o) FarObj<char> __obj_##n(o)
#define _MK_FAR_SZ(n, o, sz) FarObj<typename std::remove_pointer<decltype(o)>::type> __obj_##n(o, sz)
