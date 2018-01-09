#include <type_traits>

#define far_typed FarPtr

template<typename T>
class FarPtr : protected far_s {
public:
    FarPtr() = default;
    FarPtr(uint16_t, uint16_t);
    FarPtr(std::nullptr_t);
    FarPtr(T*);
#define ALLOW_CNV(T0, T1) (std::is_convertible<T0*, T1*>::value || \
        std::is_void<T0>::value || std::is_same<T0, char>::value || \
        std::is_same<T1, char>::value || \
        std::is_same<T0, unsigned char>::value || \
        std::is_same<T1, unsigned char>::value)
    template<typename T0, typename T1 = T,
        typename std::enable_if<ALLOW_CNV(T0, T1)>::type* = nullptr>
        FarPtr(const FarPtr<T0>&);
    template<typename T0, typename T1 = T,
        typename std::enable_if<!std::is_same<T0, T1>::value &&
        (ALLOW_CNV(T0, T1) || (std::is_pointer<T0>::value &&
            std::is_pointer<T1>::value &&
            ALLOW_CNV(typename std::remove_pointer<T0>::type,
            typename std::remove_pointer<T1>::type)))>::type* = nullptr>
        FarPtr(T0*);
    template<typename T0, typename T1 = T,
        typename std::enable_if<!ALLOW_CNV(T0, T1)>::type* = nullptr>
        explicit FarPtr(const FarPtr<T0>&);
    T* operator ->();
    operator T*();
    FarPtr<T> operator ++(int);
    FarPtr<T> operator ++();
    FarPtr<T> operator --();
    void operator +=(int);
    FarPtr<T> operator +(int);
    uint16_t seg();
    uint16_t off();
};

template<typename T> class AsmFarPtr;
template<typename T>
class FarPtrAsm : public FarPtr<T> {
public:
    FarPtrAsm(const FarPtrAsm&) = delete;
    AsmFarPtr<T> operator &();
    void operator =(const FarPtr<T>&);
};

template<typename T>
class AsmFarPtr {
public:
    AsmFarPtr();
    AsmFarPtr(T**);
    AsmFarPtr(const FarPtr<void>&);
    FarPtrAsm<T>& operator *();
    uint32_t operator()();
    operator FarPtr<T> *();
    template<typename T0>
        explicit operator T0*();
    uint16_t seg();
    uint16_t off();
    T*** get_ref();
};

#define __FAR(t) FarPtr<t>
#define __ASMFAR(t) AsmFarPtr<t>
#define __ASMFARREF(f) f.get_ref()
#define __ASMCALL(t, f) __ASMFAR(t) f
#define FP_SEG(fp)            ((fp).seg())
#define FP_OFF(fp)            ((fp).off())
#define MK_FP(seg,ofs)        (__FAR(void)(seg, ofs))

#define __DOSFAR(t) far_typed<t>
#define _MK_FP(t, f) ((__FAR(t))(f))
#define _FP_SEG(f) ((f).seg())
#define _FP_OFF(f) ((f).off())
#define _DOS_FP(p) (p)
#define _MK_DOS_FP(t, s, o) far_typed<t>(s, o)

#undef NULL
#define NULL           nullptr
