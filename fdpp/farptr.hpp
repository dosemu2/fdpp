/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2018  Stas Sergeev (stsp)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FARPTR_HPP
#define FARPTR_HPP

#include <type_traits>
#include <new>
#include <memory>
#include <cstring>
#include <unordered_set>
//#include "cppstubs.hpp"
#include "thunks_priv.h"
#include "farhlp.h"

static inline far_s _lookup_far(const void *ptr)
{
    far_s f = lookup_far(&g_farhlp[FARHLP1], ptr);
    _assert(f.seg || f.off);
    /* purge for faster look-ups - very risky! */
    purge_far(&g_farhlp[FARHLP1]);
    return f;
}

static inline void _store_far(const void *ptr, far_s fptr)
{
    /* because of things like adjust_far() we can have clashes */
    store_far_replace(&g_farhlp[FARHLP1], ptr, fptr);
}

static inline void do_store_far(far_s fptr)
{
    store_far_replace(&g_farhlp[FARHLP2], resolve_segoff(fptr), fptr);
}

#define _MK_S(s, o) (far_s){o, s}

#define _P(T1) std::is_pointer<T1>::value
#define _C(T1) std::is_const<T1>::value
#define _RP(T1) typename std::remove_pointer<T1>::type
#define _RC(T1) typename std::remove_const<T1>::type
template<typename T> class SymWrp;
template<typename T> class SymWrp2;

template<typename T>
class WrpType {
public:
    using type = typename std::conditional<std::is_class<T>::value,
        SymWrp<T>, SymWrp2<T>>::type;
    using ref_type = typename std::add_lvalue_reference<type>::type;
};

template<typename T>
class FarPtrBase {
protected:
    far_s ptr;

    using wrp_type = typename WrpType<T>::type;
public:
    FarPtrBase() = default;
    FarPtrBase(uint16_t s, uint16_t o) : ptr(_MK_S(s, o)) {}
    FarPtrBase(std::nullptr_t) : ptr(_MK_S(0, 0)) {}
    explicit FarPtrBase(const far_s& f) : ptr(f) {}

    T* operator ->() {
        do_store_far(get_far());
        return (T*)resolve_segoff(ptr);
    }
    operator T*() {
        static_assert(std::is_standard_layout<T>::value ||
                std::is_void<T>::value, "need std layout");
        if (!ptr.seg && !ptr.off)
            return NULL;
        return (T*)resolve_segoff(ptr);
    }
    template<typename T0>
    explicit operator T0*() {
        if (!ptr.seg && !ptr.off)
            return NULL;
        return (T0*)resolve_segoff(ptr);
    }

    wrp_type& get_wrp() {
        wrp_type *s = new(get_buf()) wrp_type;
        _store_far(s, get_far());
        return *s;
    }
    wrp_type& operator [](int idx) {
        return FarPtrBase<T>(*this + idx).get_wrp();
    }

    FarPtrBase<T> operator ++(int) {
        FarPtrBase<T> f = *this;
        ptr.off += sizeof(T);
        return f;
    }
    FarPtrBase<T> operator ++() {
        ptr.off += sizeof(T);
        return *this;
    }
    FarPtrBase<T> operator --() {
        ptr.off -= sizeof(T);
        return *this;
    }
    FarPtrBase<T> operator +=(int inc) { ptr.off += inc * sizeof(T); return *this; }
    FarPtrBase<T> operator +(int inc) { return FarPtrBase<T>(ptr.seg, ptr.off + inc * sizeof(T)); }
    FarPtrBase<T> operator -(int dec) { return FarPtrBase<T>(ptr.seg, ptr.off - dec * sizeof(T)); }
    bool operator == (std::nullptr_t) { return (!ptr.seg && !ptr.off); }
    bool operator != (std::nullptr_t) { return (ptr.seg || ptr.off); }
    uint16_t seg() const { return ptr.seg; }
    uint16_t off() const { return ptr.off; }
    uint32_t get_fp32() const { return ((ptr.seg << 16) | ptr.off); }
    far_s get_far() const { return ptr; }
    far_s& get_ref() { return ptr; }
    const far_s *get_addr() { return &ptr; }
    T* get_ptr() { return (T*)resolve_segoff(ptr); }
    void *get_buf() { return (void*)resolve_segoff(ptr); }
    explicit operator uint32_t () const { return get_fp32(); }
};

class ObjIf {
public:
    virtual far_s get_obj() = 0;
    virtual void ref(const void *owner) = 0;
    virtual ~ObjIf() = default;
};

template<typename T>
class FarPtr : public FarPtrBase<T>
{
    typedef std::shared_ptr<ObjIf> sh_obj;
    sh_obj obj;
    bool nonnull = false;

public:
    /* first, tell a few secret phrases to the compiler :) */
    template <typename> friend class FarPtr;
    using FarPtrBase<T>::FarPtrBase;
    FarPtr() = default;

    FarPtr(const FarPtrBase<T>& f) : FarPtrBase<T>(f) {
        /* XXX for things like "p = sym->ptr; a = p->arr[i];"
         * All sym pointer members should be marked with __DOSFAR(). */
        do_store_far(f.get_far());
    }
    explicit FarPtr(uint32_t f) : FarPtrBase<T>(f >> 16, f & 0xffff) {}
    explicit FarPtr(const sh_obj& o) :
            FarPtrBase<T>(o->get_obj()), obj(o) {}
    FarPtr(const FarPtr<T>& f) = default;
    FarPtr(uint16_t s, uint16_t o, bool nnull) :
            FarPtrBase<T>(_MK_S(s, o)), nonnull(nnull) {}

#define ALLOW_CNV(T0, T1) (( \
        std::is_void<T0>::value || \
        std::is_void<T1>::value || \
        std::is_same<_RC(T0), char>::value || \
        std::is_same<_RC(T1), char>::value || \
        std::is_same<_RC(T0), unsigned char>::value || \
        std::is_same<_RC(T1), unsigned char>::value) && \
        (_C(T1) || !_C(T0)))
    template<typename T0, typename T1 = T,
        typename std::enable_if<ALLOW_CNV(T0, T1)>::type* = nullptr>
    FarPtr(const FarPtrBase<T0>& f) : FarPtrBase<T1>(f.seg(), f.off()) {}
    template<typename T0, typename T1 = T,
        typename std::enable_if<ALLOW_CNV(T0, T1)>::type* = nullptr>
    FarPtr(const FarPtr<T0>& f) : FarPtrBase<T1>(f._seg(), f._off()),
        obj(f.obj), nonnull(f.nonnull) {}

    template<typename T0, typename T1 = T,
        typename std::enable_if<!ALLOW_CNV(T0, T1)>::type* = nullptr>
    explicit FarPtr(const FarPtrBase<T0>& f) : FarPtrBase<T1>(f.seg(), f.off()) {}
    template<typename T0, typename T1 = T,
        typename std::enable_if<!ALLOW_CNV(T0, T1)>::type* = nullptr>
    explicit FarPtr(const FarPtr<T0>& f) : FarPtrBase<T1>(f._seg(), f._off()),
        obj(f.obj), nonnull(f.nonnull) {}

    template<typename T0, typename T1 = T,
        typename std::enable_if<ALLOW_CNV(T1, T0) && !_C(T0)>::type* = nullptr>
    operator FarPtrBase<T0>() const & {
        return FarPtrBase<T0>(this->_seg(), this->_off());
    }
    template<typename T0, typename T1 = T,
        typename std::enable_if<ALLOW_CNV(T1, T0) && !_C(T0)>::type* = nullptr>
    operator FarPtrBase<T0>() && {
        _assert(!obj);
        return FarPtrBase<T0>(this->seg(), this->off());
    }

    template<typename T0, typename T1 = T,
        typename std::enable_if<ALLOW_CNV(T1, T0) && !_C(T0)>::type* = nullptr>
    operator T0*() { return (T0*)resolve_segoff(this->ptr); }

    using FarPtrBase<T>::operator ==;
    template <typename T0, typename T1 = T,
        typename std::enable_if<!std::is_void<T0>::value>::type* = nullptr,
        typename std::enable_if<std::is_void<T1>::value>::type* = nullptr>
    bool operator == (const FarPtrBase<T0>& f) const {
        return ((f.seg() == this->ptr.seg) && (f.off() == this->ptr.off));
    }

    uint16_t seg() const { _assert(!obj); return this->ptr.seg; }
    uint16_t off() const { _assert(!obj); return this->ptr.off; }
    uint16_t seg(void *owner) const {
        _assert(obj);
        obj->ref(owner);
        return this->ptr.seg;
    }
    uint16_t off(void *owner) const {
        _assert(obj);
        obj->ref(owner);
        return this->ptr.off;
    }
    uint16_t _seg() const { return this->ptr.seg; }
    uint16_t _off() const { return this->ptr.off; }
    operator T*() {
        static_assert(std::is_standard_layout<T>::value ||
                std::is_void<T>::value, "need std layout");
        if (!nonnull && !this->ptr.seg && !this->ptr.off)
            return NULL;
        return (T*)resolve_segoff(this->ptr);
    }
};

#define _MK_F(f, s) f(s)

/* These SymWrp are tricky, and are needed only because we
 * can't provide 'operator.':
 * https://isocpp.org/blog/2016/02/a-bit-of-background-for-the-operator-dot-proposal-bjarne-stroustrup
 * Consider the following code (1):
 * void FAR *f = &fp[idx];
 * In this case & should create "void FAR *" from ref, not just "void *".
 * Wrapper helps with this.
 * And some other code (2) does this:
 * int a = fp[idx].a_memb;
 * in which case we should better have no wrapper.
 * Getting both cases to work together is challenging.
 * Note that the simplest solution "operator T &()" for case 2
 * currently doesn't work, but it may work in the future:
 * http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0352r1.pdf
 * But I don't think waiting for this document to materialize in
 * gcc/g++ is a good idea, as it is not even a part of the upcoming C++20.
 * So what I do is a public inheritance of T. This kind of works,
 * but puts an additional restrictions on the wrapper, namely, that
 * it should be a POD, it should not add any data members and it should
 * be non-copyable.
 * Fortunately the POD requirement is satisfied with 'ctor = default'
 * trick, and non-copyable is simple, but having no data members means
 * I can't easily retrieve the far ptr for case 1.
 * So the only solution I can come up with, is to put the static
 * map somewhere that will allow to look up the "lost" far pointers.
 * Thus farhlp.cpp
 */
template<typename T>
class SymWrp : public T {
public:
    SymWrp() = default;
    SymWrp(const SymWrp&) = delete;
    SymWrp<T>& operator =(T& f) { *(T *)this = f; return *this; }
    FarPtr<T> operator &() const { return _MK_F(FarPtr<T>, _lookup_far(this)); }
};

template<typename T>
class SymWrp2 {
    /* remove const or default ctor will be deleted */
    _RC(T) sym;

public:
    SymWrp2() = default;
    SymWrp2(const SymWrp2&) = delete;
    SymWrp2<T>& operator =(const T& f) { sym = f; return *this; }
    FarPtr<T> operator &() const { return _MK_F(FarPtr<T>, _lookup_far(this)); }
    operator T &() { return sym; }
    /* for fmemcpy() etc that need const void* */
    template <typename T1 = T,
        typename std::enable_if<_P(T1) &&
        !std::is_void<_RP(T1)>::value>::type* = nullptr>
    operator FarPtr<const void> () const {
        return _MK_F(FarPtr<const void>, _lookup_far(this));
    }
};

template<typename T> class AsmSym;
template<typename T>
class AsmRef {
    FarPtr<T> *sym;

public:
    AsmRef(FarPtr<T> *s) : sym(s) {}
    T* operator ->() { return *sym; }
    operator FarPtr<T> () { return *sym; }
    template <typename T1 = T,
        typename std::enable_if<!std::is_void<T1>::value>::type* = nullptr>
    operator FarPtr<void> () { return FarPtr<void>(*sym); }
    uint16_t seg() const { return sym->seg(); }
    uint16_t off() const { return sym->off(); }
};

template<typename T>
class AsmSym {
    FarPtr<T> sym;

public:
    using sym_type = typename WrpType<T>::ref_type;
    sym_type get_sym() { return sym.get_wrp(); }
    AsmRef<T> get_addr() { return AsmRef<T>(&sym); }

    /* everyone with get_ref() method should have no copy ctor */
    AsmSym() = default;
    AsmSym(const AsmSym<T> &) = delete;
    far_s* get_ref() { return &sym.get_ref(); }
};

template<typename T>
class AsmFSym {
    FarPtr<T> sym;

public:
    using sym_type = decltype(sym);
    sym_type get_sym() { return sym; }

    AsmFSym() = default;
    AsmFSym(const AsmFSym<T> &) = delete;
    far_s* get_ref() { return &sym.get_ref(); }
};

template<typename T>
class CallSym {
    FarPtr<T> ptr;

public:
    CallSym(const FarPtr<T>& f) { ptr = f; }
    template <typename T1 = T,
        typename std::enable_if<std::is_void<T1>::value>::type* = nullptr>
    T1 operator()() { thunk_call_void(ptr.get_far()); }
    template <typename T1 = T,
        typename std::enable_if<!std::is_void<T1>::value>::type* = nullptr>
    T1 operator()() { return thunk_call_void(ptr.get_far()); }
};

template<typename T>
class AsmCSym {
    CallSym<T> sym;

public:
    AsmCSym(const FarPtr<T>& f) : sym(f) {}
    CallSym<T>& operator *() { return sym; }
};

template<typename T, uint16_t (*SEG)(void)>
class NearPtr {
    uint16_t _off;

public:
    explicit NearPtr(uint16_t o) : _off(o) {}    // for farobj only
    NearPtr(std::nullptr_t) : _off(0) {}
    explicit operator uint16_t () { return _off; }
    operator T *() { return FarPtr<T>(SEG(), _off); }
    NearPtr<T, SEG> operator - (const NearPtr<T, SEG>& n) const {
        return NearPtr<T, SEG>(_off - n.off());
    }
    NearPtr<T, SEG> operator +=(const int inc) {
        _off += inc * sizeof(T);
        return *this;
    }
    NearPtr<T, SEG> operator -=(const int dec) {
        _off -= dec * sizeof(T);
        return *this;
    }
    bool operator == (std::nullptr_t) { return !_off; }
    bool operator != (std::nullptr_t) { return !!_off; }
    uint16_t off() const { return _off; }

    NearPtr() = default;
};

template<typename T, int max_len = 0>
class ArSymBase {
protected:
    FarPtrBase<T> sym;

    using wrp_type = typename WrpType<T>::type;
public:
    wrp_type& operator [](int idx) {
        _assert(!max_len || idx < max_len);
        return sym[idx];
    }

    far_s* get_ref() { return &sym.get_ref(); }
};

template<typename T, int (*F)(void)>
class MembBase {
protected:
    FarPtr<T> lookup_sym() const {
        FarPtr<T> fp;
        /* find parent first */
        const uint8_t *ptr = (const uint8_t *)this - F();
        far_s f = lookup_far_st(ptr);
        if (!f.seg && !f.off) {
            f = lookup_far(&g_farhlp[FARHLP2], ptr);
            if (f.seg || f.off) {
                /* purge for faster look-ups - very risky! */
                purge_far(&g_farhlp[FARHLP2]);
            }
        }
        _assert(f.seg || f.off);
        fp = _MK_F(FarPtr<uint8_t>, f) + F();
        /* for arrays of struct that contain arrays */
        do_store_far(fp.get_far());
        return fp;
    }
};

template<typename T, int max_len, int (*F)(void)>
class ArMemb : public MembBase<T, F> {
    T sym[max_len];

    using wrp_type = typename WrpType<T>::type;
public:
    using type = T;
    static constexpr decltype(max_len) len = max_len;

    ArMemb(const T s[]) { strncpy(sym, s, max_len); }
    template <typename T1 = T,
        typename std::enable_if<!_C(T1)>::type* = nullptr>
    operator FarPtr<void> () { return this->lookup_sym(); }
    template <typename T1 = T,
        typename std::enable_if<!_C(T1)>::type* = nullptr>
    operator FarPtr<const T1> () { return this->lookup_sym(); }
    operator FarPtr<T> () { return this->lookup_sym(); }
    template <uint16_t (*SEG)(void)>
    operator NearPtr<T, SEG> () {
        FarPtr<T> f = this->lookup_sym();
        _assert(f.seg() == SEG());
        return NearPtr<T, SEG>(f.off());
    }
    operator T *() { return sym; }
    template <typename T0, typename T1 = T,
        typename std::enable_if<!std::is_same<T0, T1>::value>::type* = nullptr>
    explicit operator T0 *() { return (T0 *)sym; }
    FarPtr<T> operator +(int inc) { return this->lookup_sym() + inc; }
    FarPtr<T> operator -(int dec) { return this->lookup_sym() - dec; }

    wrp_type& operator [](int idx) {
        _assert(!max_len || idx < max_len);
        FarPtr<T> f = this->lookup_sym();
        return f[idx];
    }

    ArMemb() = default;
};

template<typename T, int max_len = 0>
class AsmArNSym : public ArSymBase<T, max_len> {
public:
    using sym_type = T*;
    sym_type get_sym() { return this->sym.get_ptr(); }

    AsmArNSym() = default;
    AsmArNSym(const AsmArNSym<T> &) = delete;
};

template<typename T, int max_len = 0>
class AsmArFSym : public ArSymBase<T, max_len> {
public:
    using sym_type = FarPtr<T>;
    sym_type get_sym() { return this->sym; }

    AsmArFSym() = default;
    AsmArFSym(const AsmArFSym<T> &) = delete;
};

template<typename T>
class FarPtrAsm {
    FarPtr<FarPtrBase<T>> ptr;

public:
    explicit FarPtrAsm(const FarPtr<FarPtrBase<T>>& f) : ptr(f) {}
    operator FarPtrBase<T> *() { return ptr.get_ptr(); }
    /* some apps do the following: *(UWORD *)&f_ptr = new_offs; */
    explicit operator uint16_t *() { return &ptr->get_ref().off; }
    uint16_t seg() const { return ptr.seg(); }
    uint16_t off() const { return ptr.off(); }
};

template<typename T>
class AsmFarPtr {
    FarPtr<FarPtrBase<T>> ptr;

public:
    using sym_type = FarPtrBase<T>&;
    sym_type get_sym() { return *ptr.get_ptr(); }
    FarPtrAsm<T> get_addr() { return FarPtrAsm<T>(ptr); }

    AsmFarPtr() = default;
    AsmFarPtr(const AsmFarPtr<T> &) = delete;
    far_s* get_ref() { return &ptr.get_ref(); }
};

template<typename T, uint16_t (*SEG)(void)>
class AsmNearPtr {
    FarPtr<NearPtr<T, SEG>> ptr;

public:
    using sym_type = NearPtr<T, SEG>&;
    sym_type get_sym() { return *ptr.get_ptr(); }

    AsmNearPtr() = default;
    AsmNearPtr(const AsmNearPtr<T, SEG> &) = delete;
    far_s* get_ref() { return &ptr.get_ref(); }
};

template<typename T, int (*F)(void)>
class SymMemb : public T, public MembBase<T, F> {
public:
    SymMemb() = default;
    SymMemb(const SymMemb&) = delete;
    SymWrp<T>& operator =(T& f) { *(T *)this = f; return *(SymWrp<T> *)this; }
    FarPtr<T> operator &() const { return this->lookup_sym(); }
};

template<typename T, int (*F)(void)>
class SymMemb2 : public MembBase<T, F> {
    T sym;

public:
    SymMemb2() = default;
    SymMemb2(const SymMemb2&) = delete;
    SymWrp2<T>& operator =(const T& f) { sym = f; return *(SymWrp2<T> *)this; }
    FarPtr<T> operator &() const { return this->lookup_sym(); }
    operator T &() { return sym; }
};

#undef _P
#undef _C
#undef _RP
#undef _RC

#define __ASMSYM(t) AsmSym<t>
#define __ASMFSYM(t) AsmFSym<t>
#define __ASMARSYM(t, l) AsmArFSym<t, l>
#define __ASMARISYM(t) AsmArNSym<t>
#define __ASMARIFSYM(t) AsmArFSym<t>
#define __FAR(t) FarPtr<t>
#define __ASMFAR(t) AsmFarPtr<t>
#define __ASMNEAR(t, s) AsmNearPtr<t, s>
#define __ASMREF(f) f.get_ref()
#define __ASMADDR(v) __##v.get_addr()
#define __ASMCALL(t, f) AsmCSym<t> f
#define __ASYM(x) x.get_sym()
#define ASMREF(t) AsmRef<t>
#define AR_MEMB(p, t, n, l) \
    static int off_##n() { \
        return offsetof(p, n); \
    } \
    ArMemb<t, l, off_##n> n
#define SYM_MEMB(p, t, n) \
    static int off_##n() { \
        return offsetof(p, n); \
    } \
    SymMemb<t, off_##n> n
#define SYM_MEMB2(c, m, p, t, n) \
    static int off_##n() { \
        return offsetof(c, m) + offsetof(p, n); \
    } \
    SymMemb<t, off_##n> n
#define SYM_MEMB_T(p, t, n) \
    static int off_##n() { \
        return offsetof(p, n); \
    } \
    SymMemb2<t, off_##n> n
#define FP_SEG(fp) ((fp).seg())
#define FP_OFF(fp) ((fp).off())
#define FP_SEG_OBJ(o, fp) ((fp).seg(o))
#define FP_OFF_OBJ(o, fp) ((fp).off(o))
#define MK_FP(seg, ofs) ({ \
    uint16_t _s = seg; \
    uint16_t _o = ofs; \
    do_store_far(_MK_S(_s, _o)); \
    __FAR(void)(_s, _o); \
})
#define MK_FP_N(seg, ofs) (__FAR(void)(seg, ofs, true))
#define __DOSFAR(t) FarPtrBase<t>
#define _MK_DOS_FP(t, s, o) __FAR(t)(MK_FP(s, o))
#define GET_FP32(f) (f).get_fp32()
#define GET_FAR(f) (f).get_far()
#define GET_PTR(f) (f).get_ptr()
#define _DOS_FP(p) ({ do_store_far(p.get_far()); p; })

#undef NULL
#define NULL           nullptr

#endif
