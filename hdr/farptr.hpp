#include <type_traits>

struct far_s {
    UWORD off;
    UWORD seg;
};

/* test template taken from here:
 * https://stackoverflow.com/a/6560691/5515547
 */
template<typename testType>
struct is_function_pointer
{
    static const bool value =
        std::is_pointer<testType>::value ?
        std::is_function<typename std::remove_pointer<testType>::type>::value :
        false;
};
/* end of taken code */

template<typename T>
class FarPtr {
public:
    FarPtr();
    FarPtr(uint16_t, uint16_t);
    FarPtr(far_s);
    FarPtr(std::nullptr_t);
    FarPtr(T*);
    FarPtr(const FarPtr<void>&);
    template<typename T1 = T, typename =
        typename std::enable_if<is_function_pointer<T1>::value>::type>
        FarPtr(T1&);
    template<typename T0, typename T1 = T, typename =
        typename std::enable_if<std::is_const<T1>::value>::type,
        typename T2 = typename std::remove_const<T1>::type, typename =
        typename std::enable_if<std::is_same<T0, T2>::value>::type>
        FarPtr(const FarPtr<T0>&);
    template<typename T0, typename T1 = T, typename =
        typename std::enable_if<std::is_void<T1>::value &&
        !std::is_void<T0>::value>::type>
        FarPtr(const FarPtr<T0>&);
    template<typename T0, typename T1 = T, typename =
        typename std::enable_if<std::is_void<T1>::value>::type>
        operator T0*&();
    T* operator ->();
    operator T*();
    T** operator &();
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
    FarPtr<T> operator *();
    FarPtr<T*> operator ->();
    T*** get_ref();
};
#define __FAR(t) FarPtr<t>
#define __FAR_FUNC(t, a) FarPtr<t (*) (a)>
#define __ASMFAR(t) AsmFarPtr<t>
#define __ASMFARREF(f) f.get_ref()
#define FP_SEG(fp)            ((fp).seg())
#define FP_OFF(fp)            ((fp).off())
#define MK_FP(seg,ofs)        (__FAR(void)(seg, ofs))

#undef NULL
#define NULL           nullptr
