#include "farptr.hpp"

template <typename T>
class FarObj {
public:
    FarObj(const T& obj);
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
    FarPtr<T> get_obj();
};

#define _MK_FAR_ST(o) static FarObjSt<decltype(o)> __obj_o(o)
#define __MK_FAR_ST(o) __obj_o.get_obj()

#define _MK_FAR(o) FarObj<decltype(o)> __obj_o(o)
#define __MK_FAR(o) __obj_o.get_obj()
#define _MK_FAR_PTR(o) FarObj<typename std::remove_pointer<decltype(o)>::type> __obj_o(*o)
#define __MK_FAR_SCP(o) __obj_o.get_obj()
