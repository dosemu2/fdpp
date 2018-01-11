#include <type_traits>

#define far_typed FarPtr

#define _P(T1) std::is_pointer<T1>::value
#define _C(T1) std::is_const<T1>::value
#define _RP(T1) typename std::remove_pointer<T1>::type
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
        (ALLOW_CNV(T0, T1) || (_P(T0) && _P(T1) &&
            ALLOW_CNV(_RP(T0), _RP(T1))))>::type* = nullptr>
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
    uint16_t __seg();
    uint16_t __off();
};

template<typename T> class AsmSym;
template<typename T>
class SymWrp : public T {
public:
    SymWrp(const T&);
    SymWrp(const SymWrp&) = delete;
    FarPtr<T> operator &();
};

template<typename T>
class SymWrp2 {
public:
    SymWrp2(const T&);
    SymWrp2(const SymWrp2&) = delete;
    FarPtr<T> operator &();
    operator T &();
    uint32_t operator()();
    /* for fmemcpy() etc that need const void* */
    template <typename T1 = T,
        typename std::enable_if<_P(T1) &&
        !std::is_void<_RP(T1)>::value>::type* = nullptr>
        operator FarPtr<const void> &();
};

template<typename T>
class AsmRef : public FarPtr<T> {
public:
    AsmRef(const AsmSym<T> *);
};

template<typename T>
class AsmSym {
public:
    template <typename T1 = T,
        typename std::enable_if<std::is_class<T1>::value>::type* = nullptr>
        SymWrp<T1>& get_sym();
    template <typename T1 = T,
        typename std::enable_if<!std::is_class<T1>::value>::type* = nullptr>
        SymWrp2<T1>& get_sym();

    /* everyone with get_ref() method should have a default ctor */
    AsmSym() = default;
    T** get_ref();
};

template<typename T>
class AsmFSym {
public:
    AsmFSym(const FarPtr<T> &);
    SymWrp2<T*>& operator *();
    FarPtr<T>& get_sym();

    AsmFSym() = default;
    T** get_ref();
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
    AsmFarPtr(T**);
    AsmFarPtr(const FarPtr<void>&);
    FarPtrAsm<T>& operator *();
    operator FarPtr<T> *();
    template<typename T0>
        explicit operator T0*();
    uint16_t __seg();
    uint16_t __off();

    AsmFarPtr() = default;
    T*** get_ref();
};

#define __ASMSYM(t) AsmSym<t>
#define __ASMFSYM(t) AsmFSym<t>
#define __FAR(t) FarPtr<t>
#define __ASMFAR(t) AsmFarPtr<t>
#define __ASMREF(f) f.get_ref()
#define __ASMADDR(v) &__##v
#define __ASMCALL(t, f) __ASMFSYM(t) f
#define __ASYM(x) x.get_sym()
#define __FSYM(x) x.get_sym()
#define ASMREF(t) AsmRef<t>
#define FP_SEG(fp)            ((fp).__seg())
#define FP_OFF(fp)            ((fp).__off())
#define MK_FP(seg,ofs)        (__FAR(void)(seg, ofs))

#define __DOSFAR(t) far_typed<t>
#define _MK_FP(t, f) ((__FAR(t))(f))
#define _FP_SEG(f) ((f).__seg())
#define _FP_OFF(f) ((f).__off())
#define _DOS_FP(p) (p)
#define _MK_DOS_FP(t, s, o) far_typed<t>(s, o)

#undef NULL
#define NULL           nullptr
