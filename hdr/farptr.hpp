#include <type_traits>

#define far_typed FarPtr

template<typename T>
class FarPtr : public far_s {
public:
    FarPtr() = default;
    FarPtr(uint16_t, uint16_t);
    FarPtr(far_s);
    FarPtr(std::nullptr_t);
    FarPtr(T*);
    FarPtr(const FarPtr<void>&);
    template<typename T0, typename T1 = T, typename =
        typename std::enable_if<std::is_const<T1>::value>::type,
        typename T2 = typename std::remove_const<T1>::type, typename =
        typename std::enable_if<std::is_same<T0, T2>::value>::type>
        FarPtr(const FarPtr<T0>&);
    template<typename T0, typename T1 = T, typename =
        typename std::enable_if<(std::is_void<T1>::value &&
        !std::is_void<T0>::value) ||
        (!std::is_void<T1>::value && std::is_const<T1>::value &&
        std::is_void<T0>::value && std::is_const<T0>::value)>::type>
        FarPtr(const FarPtr<T0>&);
    T* operator ->();
    operator T*();
    FarPtr<T*> operator &();
    FarPtr<T> operator ++(int);
    FarPtr<T> operator ++();
    FarPtr<T> operator --();
    void operator +=(int);
    FarPtr<T> operator +(int);
    uint16_t seg();
    uint16_t off();
};

template<typename T>
class AsmFarPtr {
public:
    AsmFarPtr();
    AsmFarPtr(T**);
    AsmFarPtr(const FarPtr<void>&);
    FarPtr<T> operator *();
    FarPtr<T*> operator ->();
    uint32_t operator()();
    T*** get_ref();
};

#define __FAR(t) FarPtr<t>
#define __ASMFAR(t) AsmFarPtr<t>
#define __ASMFARREF(f) f.get_ref()
#define FP_SEG(fp)            ((fp).seg())
#define FP_OFF(fp)            ((fp).off())
#define MK_FP(seg,ofs)        (__FAR(void)(seg, ofs))

#define __DOSFAR(t) far_typed<t>
#define _MK_FP(t, f) f
#define _FP_SEG(f) ((f).seg())
#define _FP_OFF(f) ((f).off())
#define _DOS_FP(p) (p)
#define _MK_DOS_FP(t, s, o) far_typed<t>(s, o)

#define _CNV_T(t, v) (t)(v)

#undef NULL
#define NULL           nullptr
