#ifndef FARPTR_HPP
#define FARPTR_HPP

#include <type_traits>

#define _P(T1) std::is_pointer<T1>::value
#define _C(T1) std::is_const<T1>::value
#define _RP(T1) typename std::remove_pointer<T1>::type

template<typename T> class SymWrp;
template<typename T> class SymWrp2;
template<typename T, int max_len = 0>
class ArSymBase {
public:
    template <typename T1 = T,
        typename std::enable_if<std::is_class<T1>::value>::type* = nullptr>
        SymWrp<T1>& operator [](unsigned);
    template <typename T1 = T,
        typename std::enable_if<!std::is_class<T1>::value>::type* = nullptr>
        SymWrp2<T1>& operator [](unsigned);
};

template<typename T>
class FarPtr : protected far_s, public ArSymBase<T> {
public:
    FarPtr() = default;
    FarPtr(uint16_t, uint16_t);
    FarPtr(std::nullptr_t);
#define ALLOW_CNV0(T0, T1) std::is_convertible<T0*, T1*>::value
#define ALLOW_CNV1(T0, T1) \
        std::is_void<T0>::value || std::is_same<T0, char>::value || \
        std::is_same<T1, char>::value || \
        std::is_same<T0, unsigned char>::value || \
        std::is_same<T1, unsigned char>::value
#define ALLOW_CNV(T0, T1) (ALLOW_CNV0(T0, T1) || ALLOW_CNV1(T0, T1))
    template<typename T0, typename T1 = T,
        typename std::enable_if<ALLOW_CNV(T0, T1)>::type* = nullptr>
        FarPtr(const FarPtr<T0>&);
    template<typename T0, typename T1 = T,
        typename std::enable_if<!ALLOW_CNV(T0, T1)>::type* = nullptr>
        explicit FarPtr(const FarPtr<T0>&);
    T* operator ->();
    operator T*();
    template<typename T0, typename T1 = T,
        typename std::enable_if<ALLOW_CNV1(T1, T0)>::type* = nullptr>
        operator T0*();
    FarPtr<T> operator ++(int);
    FarPtr<T> operator ++();
    FarPtr<T> operator --();
    void operator +=(int);
    FarPtr<T> operator +(int);
    FarPtr<T> operator -(int);
    uint16_t __seg();
    uint16_t __off();
};

template<typename T>
class SymWrp : public T {
public:
    SymWrp() = default;
    SymWrp(const T&);
    SymWrp(const SymWrp&) = delete;
    FarPtr<T> operator &();
};

template<typename T>
class SymWrp2 {
public:
    SymWrp2() = default;
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
class AsmRef : public T {
public:
    AsmRef(const AsmRef<T> &);
    T* operator ->();
    operator FarPtr<T> ();
    template <typename T1 = T,
        typename std::enable_if<!std::is_void<T1>::value>::type* = nullptr>
        operator FarPtr<void> ();
    uint16_t __seg();
    uint16_t __off();
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
    AsmRef<T> operator &();

    /* everyone with get_ref() method should have no copy ctor */
    AsmSym() = default;
    AsmSym(const AsmSym<T> &) = delete;
    T** get_ref();
};

template<typename T>
class AsmFSym {
public:
    FarPtr<T> get_sym();

    AsmFSym() = default;
    AsmFSym(const AsmFSym<T> &) = delete;
    T** get_ref();
};

template<typename T>
class AsmCSym {
public:
    AsmCSym(const FarPtr<T> &);
    SymWrp2<T*>& operator *();
};

template<typename T>
class NearPtr {
public:
    NearPtr(uint16_t);
    operator uint16_t ();
    FarPtr<T> operator &();
    operator T *();
    NearPtr<T> operator -(const NearPtr<T> &);
    uint16_t __off();

    NearPtr() = default;
};

template<typename T, int max_len = 0>
class ArSym : public ArSymBase<T> {
public:
    ArSym(const FarPtr<void> &);
    operator FarPtr<void> ();
    template <typename T1 = T,
        typename std::enable_if<!_C(T1)>::type* = nullptr>
        operator FarPtr<const void> ();
    template <typename T1 = T,
        typename std::enable_if<!_C(T1)>::type* = nullptr>
        operator FarPtr<const T1> ();
    operator FarPtr<T> ();
    operator NearPtr<T> ();
    operator SymWrp2<T*>& ();
    operator T *();
    template <typename T0, typename T1 = T,
        typename std::enable_if<!std::is_same<T0, T1>::value>::type* = nullptr>
        explicit operator T0 *();
    FarPtr<T> operator +(int);
    FarPtr<T> operator +(unsigned);
    FarPtr<T> operator +(size_t);
    FarPtr<T> operator -(int);

    ArSym() = default;
    ArSym(const ArSym<T> &) = delete;
};

template<typename T, int max_len = 0>
class AsmArSym : public ArSymBase<T> {
public:
    AsmArSym() = default;
    AsmArSym(const AsmArSym<T> &) = delete;
    T*** get_ref();
};

template<typename T, int max_len = 0>
class AsmArNSym : public AsmArSym<T> {
public:
    NearPtr<T> get_sym();

    AsmArNSym() = default;
    AsmArNSym(const AsmArNSym<T> &) = delete;
};

template<typename T, int max_len = 0>
class AsmArFSym : public AsmArSym<T> {
public:
    FarPtr<T> get_sym();

    AsmArFSym() = default;
    AsmArFSym(const AsmArFSym<T> &) = delete;
};

template<typename T>
class FarPtrAsm {
public:
    /* some apps do the following: *(UWORD *)&f_ptr = new_offs; */
    explicit operator uint16_t *();
    operator FarPtr<T> *();
    /* via mk_dosobj() I guess */
    uint16_t __seg();
    uint16_t __off();
};

template<typename T>
class AsmFarPtr {
public:
    AsmFarPtr(const FarPtr<T>&);
    template <typename T1 = T,
        typename std::enable_if<!std::is_void<T1>::value>::type* = nullptr>
        AsmFarPtr(const FarPtr<void>&);
    template <typename T0, typename T1 = T,
        typename std::enable_if<!std::is_same<T1, T0>::value &&
            std::is_void<T1>::value>::type* = nullptr>
        AsmFarPtr(const FarPtr<T0>&);
    operator FarPtr<T> &();
    FarPtr<T>& get_sym();
    FarPtrAsm<T> operator &();
    uint16_t __seg();
    uint16_t __off();

    AsmFarPtr() = default;
    AsmFarPtr(const AsmFarPtr<T> &) = delete;
    T*** get_ref();
};

#define __ASMSYM(t) AsmSym<t>
#define __ASMFSYM(t) AsmFSym<t>
#define __ASMARSYM(t, v, l) AsmArFSym<t, l> v
#define __ASMARISYM(t, v) AsmArNSym<t> v
#define __ASMARIFSYM(t, v) AsmArFSym<t> v
#define __FAR(t) FarPtr<t>
#define __ASMFAR(t) AsmFarPtr<t>
#define __ASMREF(f) f.get_ref()
#define __ASMADDR(v) &__##v
#define __ASMCALL(t, f) AsmCSym<t> f
#define __ASYM(x) x.get_sym()
#define ASMREF(t) AsmRef<t>
#define AR_MEMB(t, n, l) ArSym<t, l> n
#define SYM_MEMB(t) SymWrp<t>
#define SYM_MEMB_T(t) SymWrp2<t>
#define PTR_MEMB(t) NearPtr<t>
#define FP_SEG(fp)            ((fp).__seg())
#define FP_OFF(fp)            ((fp).__off())
#define MK_FP(seg,ofs)        (__FAR(void)(seg, ofs))
#define __DOSFAR(t) FarPtr<t>
#define _MK_DOS_FP(t, s, o) __FAR(t)MK_FP(s, o)

#undef NULL
#define NULL           nullptr

#endif
