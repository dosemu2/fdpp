#include <type_traits>

template<typename T>
struct far_typed : public far_s{};

template<typename T>
class FarPtr {
public:
    FarPtr();
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
    T** operator &();
    FarPtr<T> operator ++(int);
    FarPtr<T> operator ++();
    FarPtr<T> operator --();
    void operator +=(int);
    FarPtr<T> operator +(int);
    far_typed<T> dosfar() const;
    uint16_t seg();
    uint16_t off();
};
template<typename T>
class AsmFarPtr {
public:
    AsmFarPtr();
    AsmFarPtr(T**);
    FarPtr<T> operator *();
    FarPtr<T*> operator ->();
    T*** get_ref();
};


template <typename T>
static far_typed<T> to_fp(const FarPtr<T>& p)
{
    return p.dosfar();
}
template <typename T>
static far_typed<T> to_fp(const FarPtr<void>& p)
{
    return p;
}

#define __FAR(t) FarPtr<t>
#define __ASMFAR(t) AsmFarPtr<t>
#define __ASMFARREF(f) f.get_ref()
#define FP_SEG(fp)            ((fp).seg())
#define FP_OFF(fp)            ((fp).off())
#define MK_FP(seg,ofs)        (__FAR(void)(seg, ofs))

#define __DOSFAR(t) far_typed<t>
#define _MK_FP(t, f) ((__FAR(t)) MK_FP(f.seg, f.off))
#define _FP_SEG(f) ((f).seg)
#define _FP_OFF(f) ((f).off)
#define _DOS_FP(p) to_fp(p)
#define _MK_DOS_FP(t, s, o) ({ \
    far_typed<t> __p; \
    __p.off = (UWORD)(o); \
    __p.seg = (UWORD)(s); \
    __p; \
})

template <typename T1, typename T2, typename =
    typename std::enable_if<std::is_pod<T2>::value>::type>
T1 _cnv(T2 &v)
{
    return (T1)v;
}
template <typename T, typename T1>
T _cnv(const FarPtr<T1> &v)
{
    return v.dosfar();
}

#define _CNV_T(t, v) _cnv<t>(v)

#undef NULL
#define NULL           nullptr
