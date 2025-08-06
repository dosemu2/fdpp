/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2018-2025  Stas Sergeev (stsp)
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
#include <memory>
#include <cstring>
#include <unordered_set>
#include "thunks_priv.h"
#include "objlock.hpp"

#if !defined(__clang__) || (__clang_major__ >= 16) || defined(__ANDROID__)
#define NONPOD_PACKED __attribute__((packed))
#define MAYBE_PACKED
#else
#define NONPOD_PACKED
#define MAYBE_PACKED __attribute__((packed))
#endif

static inline far_s _MK_S(uint32_t s, uint16_t o)
{
    if (s > 0xffff) {
        int delta = s - 0xffff;
        if (delta > 1)
            fdloudprintf("strange segment %x\n", s);
        o += delta << 4;
        s = 0xffff;
    }
    return (far_s){o, (uint16_t)s};
}

#define _P(T1) std::is_pointer<T1>::value
#define _C(T1) std::is_const<T1>::value
#define _RP(T1) typename std::remove_pointer<T1>::type
#define _RC(T1) typename std::remove_const<T1>::type
template<typename> class SymWrp;
template<typename> class SymWrp2;

template<typename T>
class WrpTypeS {
public:
    using type = typename std::conditional<std::is_class<T>::value,
        SymWrp<T>, SymWrp2<T>>::type;
    using ref_type = typename std::add_lvalue_reference<type>::type;
};

template<typename T>
using WrpType = typename WrpTypeS<T>::type;

#define ALLOW_CNV(T0, T1) (( \
        std::is_void<T0>::value || \
        std::is_void<T1>::value || \
        std::is_same<_RC(T0), char>::value || \
        std::is_same<_RC(T1), char>::value || \
        std::is_same<_RC(T0), unsigned char>::value || \
        std::is_same<_RC(T1), unsigned char>::value || \
        std::is_same<_RC(T1), T0>::value) && \
        !std::is_same<T0, T1>::value && \
        (_C(T1) || !_C(T0)))

template<typename T>
class FarPtr;

template<typename T>
class FarPtrBase {
protected:
    far_s ptr;

    using wrp_type = typename WrpTypeS<T>::type;
    using wrp_type_s = wrp_type;
    using wrp_type_a = wrp_type;
    using TheBase = FarPtrBase<T>;

    void do_adjust_far() {
        if (ptr.seg == 0xffff)
            return;
        ptr.seg += ptr.off >> 4;
        ptr.off &= 0xf;
    }
public:
    FarPtrBase() = default;
    FarPtrBase(uint32_t s, uint16_t o) : ptr(_MK_S(s, o)) {}
    FarPtrBase(std::nullptr_t) : ptr(_MK_S(0, 0)) {}
    explicit FarPtrBase(const far_s& f) : ptr(f) {}

    template<typename T0, typename T1 = T,
        typename std::enable_if<ALLOW_CNV(T0, T1)>::type* = nullptr>
    FarPtrBase(const FarPtrBase<T0>& f) : ptr(_MK_S(f.seg(), f.off())) {}

    FarPtr<T> operator ->() const {
        return FarPtr<T>(ptr);
    }
    operator T*() const {
        static_assert(std::is_standard_layout<T>::value ||
                std::is_void<T>::value, "need std layout");
        if (!ptr.seg && !ptr.off)
            return NULL;
        return (T*)resolve_segoff_fd(ptr);
    }
    template<typename T0>
    explicit operator T0*() const {
        static_assert(std::is_standard_layout<T0>::value ||
                std::is_void<T0>::value, "need std layout");
        if (!ptr.seg && !ptr.off)
            return NULL;
        return (T0*)resolve_segoff_fd(ptr);
    }

    wrp_type_s get_wrp() {
        return wrp_type_s(get_far());
    }
    wrp_type_a operator [](int idx) {
        TheBase f = TheBase(*this + idx);
        return wrp_type_a(f.get_far());
    }

    TheBase operator ++(int) {
        TheBase f = *this;
        ptr.off += sizeof(T);
        return f;
    }
    TheBase operator --(int) {
        TheBase f = *this;
        ptr.off -= sizeof(T);
        return f;
    }
    TheBase& operator ++() {
        ptr.off += sizeof(T);
        return *this;
    }
    TheBase& operator --() {
        ptr.off -= sizeof(T);
        return *this;
    }
    TheBase& operator +=(int inc) { ptr.off += inc * sizeof(T); return *this; }
    TheBase& operator -=(int dec) { ptr.off -= dec * sizeof(T); return *this; }
    TheBase operator +(int inc) const { return TheBase(ptr.seg, ptr.off + inc * sizeof(T)); }
    TheBase operator -(int dec) const { return TheBase(ptr.seg, ptr.off - dec * sizeof(T)); }
    bool operator == (std::nullptr_t) const { return (!ptr.seg && !ptr.off); }
    bool operator != (std::nullptr_t) const { return (ptr.seg || ptr.off); }
    uint16_t seg() const { return ptr.seg; }
    uint16_t off() const { return ptr.off; }
    uint32_t get_fp32() const { return ((ptr.seg << 16) | ptr.off); }
    far_s get_far() const { return ptr; }
    far_s *get_ref() { return &ptr; }
    T* get_ptr() const { return (T*)resolve_segoff(ptr); }
    explicit operator uint32_t () const { return get_fp32(); }
} NONPOD_PACKED;

class ObjIf {
public:
    virtual far_s get_obj() = 0;
    virtual void ref(const void *owner) = 0;
    virtual bool is_dupe(const ObjIf *o) = 0;
    virtual bool is_alias(const ObjIf *o) = 0;
    virtual void re_read() = 0;
    virtual ~ObjIf() = default;
};

template <typename T> class FarObj;
#define __Sx(x) #x
#define _Sx(x) __Sx(x)
#define NM __FILE__ ":" _Sx(__LINE__)
#define _RR(t) typename std::remove_reference<t>::type
#define _MK_FAR(o) std::make_shared<FarObj<_RR(decltype(o))>>(o, NM)
#define _MK_FAR_S(o) std::make_shared<FarObj<char>>(o, strlen(o) + 1, false, NM)
#define _MK_FAR_CS(o) std::make_shared<FarObj<const char>>(o, strlen(o) + 1, true, NM)

template<typename T>
class FarPtr : public FarPtrBase<T>
{
    typedef std::shared_ptr<ObjIf> sh_obj;
    sh_obj obj;
    bool nonnull = false;

public:
    /* first, tell a few secret phrases to the compiler :) */
    template <typename> friend class FarPtr;
    using TheBase = typename FarPtrBase<T>::FarPtrBase::TheBase;
    using FarPtrBase<T>::FarPtrBase;
    using wrp_type_m = SymWrp<T>;
    using wrp_type_s = typename WrpTypeS<T>::type;
    using wrp_type_mp = std::unique_ptr<wrp_type_m>;
    FarPtr() = default;

    FarPtr(const TheBase& f) : TheBase(f) {}
    explicit FarPtr(uint32_t f) : TheBase(f >> 16, f & 0xffff) {}
    explicit FarPtr(const sh_obj& o) :
            TheBase(o->get_obj()), obj(o) {}
    FarPtr(const FarPtr<T>& f) = default;
    FarPtr(uint32_t s, uint16_t o, bool nnull) :
            TheBase(_MK_S(s, o)), nonnull(nnull) {}

    template<typename T0, typename T1 = T,
        typename std::enable_if<ALLOW_CNV(T0, T1)>::type* = nullptr>
    FarPtr(const FarPtrBase<T0>& f) : FarPtrBase<T1>(f.seg(), f.off()) {}
    template<typename T0, typename T1 = T,
        typename std::enable_if<ALLOW_CNV(T0, T1)>::type* = nullptr>
    FarPtr(const FarPtr<T0>& f) : FarPtrBase<T1>(f._seg_(), f._off_()),
        obj(f.obj), nonnull(f.nonnull) {}

    template<typename T0, typename T1 = T,
        typename std::enable_if<!ALLOW_CNV(T0, T1)>::type* = nullptr>
    explicit FarPtr(const FarPtrBase<T0>& f) : FarPtrBase<T1>(f.seg(), f.off()) {}
    template<typename T0, typename T1 = T,
        typename std::enable_if<!ALLOW_CNV(T0, T1)>::type* = nullptr>
    explicit FarPtr(const FarPtr<T0>& f) : FarPtrBase<T1>(f._seg_(), f._off_()),
        obj(f.obj), nonnull(f.nonnull) {}

    template<typename T0, typename T1 = T,
        typename std::enable_if<ALLOW_CNV(T1, T0) &&
        _C(T0) == _C(T1)>::type* = nullptr>
    operator FarPtrBase<T0>() const & {
        return FarPtrBase<T0>(this->_seg_(), this->_off_());
    }
    template<typename T0, typename T1 = T,
        typename std::enable_if<ALLOW_CNV(T1, T0) &&
        _C(T0) == _C(T1)>::type* = nullptr>
    operator FarPtrBase<T0>() && {
        ___assert(!obj);
        return FarPtrBase<T0>(this->seg(), this->off());
    }

    template<typename T0, typename T1 = T,
        typename std::enable_if<ALLOW_CNV(T1, T0) && !_C(T0)>::type* = nullptr>
    operator T0*() { return (T0*)resolve_segoff_fd(this->ptr); }
    template<typename T0, typename T1 = T,
        typename std::enable_if<ALLOW_CNV(T1, T0) && _C(T0)>::type* = nullptr>
    operator T0*() const { return (T0*)resolve_segoff_fd(this->ptr); }
    template<typename T0, typename T1 = T,
        typename std::enable_if<!ALLOW_CNV(T1, T0)>::type* = nullptr>
    explicit operator T0*() { return (T0*)resolve_segoff_fd(this->ptr); }

    using TheBase::operator ==;
    template <typename T0, typename T1 = T,
        typename std::enable_if<!std::is_void<T0>::value>::type* = nullptr,
        typename std::enable_if<std::is_void<T1>::value>::type* = nullptr>
    bool operator == (const FarPtrBase<T0>& f) const {
        return ((f.seg() == this->ptr.seg) && (f.off() == this->ptr.off));
    }

    FarPtr<T>& operator = (const FarPtr<T>& f) {
        bool clash = false;
        bool dupe = false;
        if (obj && f.obj) {
            clash = obj->is_alias(f.obj.get());
            dupe = obj->is_dupe(f.obj.get());
        }
        /* just ignore dupe */
        if (!dupe && (obj || f.obj))
            obj = f.obj;  // this implicitly does write (aka copy-back)
        if (clash) {
            ___assert(!dupe);
            /* clash is tricky as it makes the order of reads and writes
             * important. And at that point read was done before write
             * (write is just done few line above). We can retry read to
             * recover from that madness. */
            obj->re_read();
        }

        /* copy rest by hands, sigh */
        nonnull = f.nonnull;
        this->ptr = f.ptr;
        return *this;
    }

    /* below is a bit tricky. If we have a dosobj here, then we
     * should use the "owner" form to link the objects together.
     * But if the object is const, we don't have to link it to
     * the owner, because the fwd copy is already done and bwd
     * copy is not needed. So the simple form can be used then. */
    uint16_t seg() const {
        ___assert(!obj || std::is_const<T>::value);
        return this->ptr.seg;
    }
    uint16_t off() const {
        ___assert(!obj || std::is_const<T>::value);
        return this->ptr.off;
    }
    uint16_t seg(void *owner) const {
        ___assert(obj);
        obj->ref(owner);
        return this->ptr.seg;
    }
    uint16_t off(void *owner) const {
        ___assert(obj);
        obj->ref(owner);
        return this->ptr.off;
    }
    uint16_t _seg_() const { return this->ptr.seg; }
    uint16_t _off_() const { return this->ptr.off; }

    operator T*() {
        static_assert(std::is_standard_layout<T>::value ||
                std::is_void<T>::value, "need std layout");
        if (!nonnull && !this->ptr.seg && !this->ptr.off)
            return NULL;
        return (T*)resolve_segoff_fd(this->ptr);
    }

    wrp_type_s operator *() { return wrp_type_s(this->get_far(), nonnull); }

    wrp_type_mp operator ->() const {
        static_assert(std::is_standard_layout<T>::value ||
                std::is_void<T>::value, "need std layout");
        if (!nonnull && !this->ptr.seg && !this->ptr.off)
            return NULL;
        return wrp_type_mp(new wrp_type_m(this->ptr));
    }

    FarPtr<T>& adjust_far() { this->do_adjust_far(); return *this; }
};

template<typename, int, typename P, auto, bool> class ArMemb;

template<typename T>
class XFarPtr : public FarPtr<T>
{
public:

    using FarPtr<T>::FarPtr;
    XFarPtr() = delete;
    XFarPtr(FarPtr<T>& f) : FarPtr<T>(f) {}

    template<typename T1 = T, int max_len, typename P, auto M, bool V>
    XFarPtr(const ArMemb<T1, max_len, P, M, V> &p) : FarPtr<T>(p.get_far()) {}

    /* The below ctors are safe wrt implicit conversions as they take
     * the lvalue reference to the pointer, not just the pointer itself.
     * This is the reason why they exist at all.
     * So don't remove refs and don't add consts, as consts will disable
     * copy-back. */
    template<typename T1 = T,
        typename std::enable_if<
          !std::is_same<T1, char>::value &&
          !std::is_same<T1, const char>::value
        >::type* = nullptr>
    XFarPtr(T1 *&p) : FarPtr<T>(_MK_FAR(*p)) {}
    template<typename T1 = T,
        typename std::enable_if<
          std::is_same<T1, const char>::value ||
          std::is_same<T1, const void>::value
        >::type* = nullptr>
    XFarPtr(char *&s) : FarPtr<T>(_MK_FAR_S(s)) {}
    template<typename T1 = T,
        typename std::enable_if<
          std::is_same<T1, char>::value ||
          std::is_same<T1, const char>::value ||
          std::is_void<T1>::value
        >::type* = nullptr>
    XFarPtr(const char *&s) : FarPtr<T>(_MK_FAR_CS(s)) {}
    template<typename T1 = T, int N,
        typename std::enable_if<!std::is_void<T1>::value>::type* = nullptr>
    XFarPtr(T1 (&p)[N]) : FarPtr<T>(_MK_FAR(p)) {}
};

#define _MK_F(f, s) ({ ___assert(s.seg || s.off); (f)s; })

template<typename T>
class SymWrp : public T {
    FarPtr<T> fptr;
    T backup;
    /* Magic is needed to check in a parent lookup if the wrapper
     * was actually emitted. It is checked in get_far() method.
     */
    static constexpr const char magic_val[] = "voodoo magic 123";
    char magic[sizeof(magic_val)];

    static void copy_mods(far_t fp, const T *src, const T *ref) {
        if (std::memcmp(src, ref, sizeof(T))) {
            objlock_lock(fp);
            std::memcpy(resolve_segoff(fp), src, sizeof(T));
        }
    }

    void dtor_common(bool mods) {
        bool ok = check_magic();
        if (ok) {
            if (mods)
                copy_mods(fptr.get_far(), this, &backup);
            objlock_unref(fptr.get_far());
        }
    }

    template <typename T1 = T,
        typename std::enable_if<!_C(T1)>::type* = nullptr>
    void dtor() { dtor_common(fptr.get_ptr() ? true : false); }
    template <typename T1 = T,
        typename std::enable_if<_C(T1)>::type* = nullptr>
    void dtor() { dtor_common(false); }

    bool check_magic() const {
        bool ok = std::strcmp(magic, magic_val) == 0;
        if (!ok)
            fdloudprintf("magic bytes corrupted\n");
        return ok;
    }
public:
    SymWrp(far_s f, bool nonnull = false) :
            fptr(f), backup(*(T *)resolve_segoff(f)) {
        ___assert(f.seg || f.off || nonnull);
        objlock_ref(f);
        *(_RC(T) *)this = backup;
        std::strcpy(magic, magic_val);
    }
    ~SymWrp() { dtor(); }
    SymWrp(const SymWrp&) = delete;
    SymWrp(SymWrp&& s) : fptr(s.fptr), backup(s.backup) {
        std::strcpy(magic, s.magic);
        ___assert(check_magic());
        *(T *)this = *(T *)&s;
        s.clear();
    }
    SymWrp<T>& operator =(T& f) { *(T *)this = f; return *this; }
    SymWrp<T>& operator =(const SymWrp<T>& f) {
        *(T *)this = f;
        return *this;
    }
    FarPtr<T> operator &() const { return _MK_F(FarPtr<T>, fptr.get_far()); }
    far_t get_far() const { ___assert(check_magic()); return fptr.get_far(); }
    void clear() { fptr = NULL; }
};

template<typename T>
class SymWrp2 {
    /* remove const or default ctor will be deleted */
    _RC(T) sym;
    FarPtr<T> fptr;
    T backup;

    template <typename T1 = T,
        typename std::enable_if<!_C(T1)>::type* = nullptr>
    void dtor() {
        if (fptr.get_ptr() && sym != backup)
            *(decltype(sym) *)resolve_segoff(fptr.get_far()) = sym;
    }
    template <typename T1 = T,
        typename std::enable_if<_C(T1)>::type* = nullptr>
    void dtor() {}
public:
    SymWrp2(far_s f, bool nonnull = false) :
            sym(*(T *)resolve_segoff(f)), fptr(f), backup(sym) {
        ___assert(f.seg || f.off || nonnull);
    }
    ~SymWrp2() { dtor(); }
    SymWrp2(const SymWrp2&) = delete;
    SymWrp2(SymWrp2&& s) : sym(s.sym), fptr(s.fptr), backup(s.backup) {
        s.clear();
    }
    SymWrp2<T>& operator =(const T& f) { sym = f; return *this; }
    SymWrp2<T>& operator =(const SymWrp2<T>& f) {
        sym = f.sym;
        return *this;
    }
    FarPtr<T> operator &() const { return _MK_F(FarPtr<T>, fptr.get_far()); }
    operator T &() { return sym; }
    /* for fmemcpy() etc that need const void* */
    template <typename T1 = T,
        typename std::enable_if<_P(T1) &&
        !std::is_void<_RP(T1)>::value>::type* = nullptr>
    operator FarPtr<const void> () const {
        return _MK_F(FarPtr<const void>, fptr.get_far());
    }
    const T get_sym() const { return sym; }
    void clear() { fptr = NULL; }
};

template<typename T>
class AsmRef {
    FarPtr<T> *sym;

public:
    using wrp_type_m = SymWrp<T>;
    using wrp_type_mp = std::unique_ptr<wrp_type_m>;
    AsmRef(FarPtr<T> *s) : sym(s) {}
    wrp_type_mp operator ->() {
        return wrp_type_mp(new wrp_type_m(sym->get_far()));
    }
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
    using sym_type = typename WrpTypeS<T>::type;
    sym_type get_sym() { return sym.get_wrp(); }
    AsmRef<T> get_addr() { return AsmRef<T>(&sym); }

    /* everyone with get_ref() method should have no copy ctor */
    AsmSym() = default;
    AsmSym(const AsmSym<T> &) = delete;
    far_s* get_ref() { return sym.get_ref(); }
};

template<typename T>
class AsmFSym {
    FarPtr<T> sym;

public:
    using sym_type = decltype(sym);
    sym_type get_sym() { return sym; }

    AsmFSym() = default;
    AsmFSym(const AsmFSym<T> &) = delete;
    far_s* get_ref() { return sym.get_ref(); }
};

class CallSym {
    FarPtrBase<void> ptr;

public:
    CallSym(const FarPtrBase<void>& f) { ptr = f; }
    void operator()() { thunk_call_void_noret(ptr.get_far()); }
};

class AsmCSym {
    FarPtrBase<void> ptr;

public:
    AsmCSym(const FarPtrBase<void>& f) : ptr(f) {}
    CallSym operator *() { return CallSym(ptr); }
    void operator()() { thunk_call_void(ptr.get_far()); }
};

template<typename T, uint16_t (*SEG)(void)>
class NearPtr {
    uint16_t _off;

public:
    explicit NearPtr(uint16_t o) : _off(o) {}    // for farobj only
    NearPtr(std::nullptr_t) : _off(0) {}
    explicit operator uint16_t () const { return _off; }
    operator T *() const { return FarPtr<T>(SEG(), _off).get_ptr(); }
    int operator - (const NearPtr<T, SEG>& n) const {
        return _off - n.off();
    }
    NearPtr<T, SEG> operator +=(const int inc) {
        _off += inc * sizeof(T);
        return *this;
    }
    NearPtr<T, SEG> operator -=(const int dec) {
        _off -= dec * sizeof(T);
        return *this;
    }
    bool operator == (std::nullptr_t) const { return !_off; }
    bool operator != (std::nullptr_t) const { return !!_off; }
    uint16_t seg() const { return SEG(); }
    uint16_t off() const { return _off; }
    uint32_t get_fp32() const { return ((SEG() << 16) | _off); }

    NearPtr() = default;
};

template<typename T, int max_len = 0>
class ArSymBase {
protected:
    FarPtrBase<T> sym;

    using wrp_type = WrpType<T>;
public:
    wrp_type& operator [](int idx) {
        ___assert(!max_len || idx < max_len);
        return sym[idx];
    }

    far_s* get_ref() { return sym.get_ref(); }
};

template<typename T, typename P, auto M, int O = 0>
class MembBase {
protected:
    FarPtr<T> lookup_sym() const {
        using wrp_type = typename WrpTypeS<P>::type;
        FarPtr<T> fp;
        constexpr int off = M() + O;
        /* find parent first */
        const wrp_type *ptr = (wrp_type *)((const uint8_t *)this - off);
        far_s f = ptr->get_far();
        ___assert(f.seg || f.off);
        fp = _MK_F(FarPtr<uint8_t>, f) + off;
        return fp;
    }
};

template<typename T, int max_len, typename P, auto M, bool V = false>
class ArMemb : public MembBase<T, P, M> {
    T sym[max_len];
    static_assert(max_len > 0, "0-length array");

    using wrp_type = typename WrpTypeS<T>::type;
public:
    using type = T;
    static constexpr decltype(max_len) len = max_len;

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
        ___assert(f.seg() == SEG());
        return NearPtr<T, SEG>(f.off());
    }
    operator T *() { return sym; }
    FarPtr<T> operator +(int inc) { return this->lookup_sym() + inc; }
    FarPtr<T> operator -(int dec) { return this->lookup_sym() - dec; }
    far_s get_far() const { return this->lookup_sym().get_far(); }
    T *get_ptr() { return sym; }

    wrp_type operator [](int idx) {
        ___assert(V || idx < max_len);
        FarPtr<T> f = this->lookup_sym();
        return f[idx];
    }

    ArMemb() = default;
};

template<typename T, uint16_t (*SEG)(void), int max_len = 0>
class AsmArNSym : public ArSymBase<T, max_len> {
public:
    NearPtr<T, SEG> get_sym() {
        ___assert(this->sym.seg() == SEG());
        return NearPtr<T, SEG>(this->sym.off());
    }

    AsmArNSym() = default;
    AsmArNSym(const AsmArNSym<T, SEG, max_len> &) = delete;
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
class AsmFarPtr {
    FarPtr<FarPtrBase<T>> ptr;

public:
    using sym_type = FarPtrBase<T>&;
    sym_type get_sym() { return *ptr.get_ptr(); }

    AsmFarPtr() = default;
    AsmFarPtr(const AsmFarPtr<T> &) = delete;
    far_s* get_ref() { return ptr.get_ref(); }
};

template<typename T, uint16_t (*SEG)(void)>
class AsmNearPtr {
    FarPtr<NearPtr<T, SEG>> ptr;

public:
    using sym_type = NearPtr<T, SEG>&;
    sym_type get_sym() { return *ptr.get_ptr(); }

    AsmNearPtr() = default;
    AsmNearPtr(const AsmNearPtr<T, SEG> &) = delete;
    far_s* get_ref() { return ptr.get_ref(); }
};

template<typename T, typename P, auto M, int O = 0>
class SymMemb : public T, public MembBase<T, P, M, O> {
public:
    using wrp_type_s = SymWrp<T>;
    SymMemb() = default;
    T& operator =(const T& f) { *(T *)this = f; return *this; }
    FarPtr<T> operator &() const { return this->lookup_sym(); }
    /* This dot_barrier is needed because operator dot can't save
     * the lhs fptr. We add manual fixups whereever it crashes... */
    wrp_type_s dot_barrier() const {
        FarPtr<T> sym = this->lookup_sym();
        return wrp_type_s(sym.get_far());
    }
} NONPOD_PACKED;

/* Safety wrapper, in case the dot barriers are added by some stupid script,
 * rather than manually, as now. */
class DotBarrierWrap {
public:
    template<typename T0, typename T1, auto M, int O,
        typename TN = SymMemb<T0, T1, M, O> >
    static constexpr typename TN::wrp_type_s
            dot_barrier(SymMemb<T0, T1, M, O>& s) {
        return s.dot_barrier();
    }
    template<typename T1>
    static constexpr T1& dot_barrier(T1& s) { return s; }
};

template<typename T, typename P, auto M, int O = 0>
class SymMembT : public MembBase<T, P, M, O> {
    T sym;

public:
    SymMembT() = default;
    T& operator =(const T& f) { sym = f; return sym; }
    FarPtr<T> operator &() const { return this->lookup_sym(); }
    operator T &() { return sym; }
} NONPOD_PACKED;

#undef _P
#undef _C
#undef _RP
#undef _RC

#define __ASMSYM(t) AsmSym<t>
#define __ASMFSYM(t) AsmFSym<t>
#define __ASMARSYM(t, l) AsmArFSym<t, l>
#define __ASMARISYM(t, s) AsmArNSym<t, s>
#define __ASMARIFSYM(t) AsmArFSym<t>
#define __FAR(t) FarPtr<t>
#define __XFAR(t) XFarPtr<t>
#define __ASMFAR(t) AsmFarPtr<t>
#define __ASMNEAR(t, s) AsmNearPtr<t, s>
#define __ASMREF(f) f.get_ref()
#define __ASMADDR(v) __##v.get_addr()
#define __ASMCALL(f) AsmCSym f
#define __ASYM(x) __##x.get_sym()
#define __ASYM_L(x) __##x.get_sym()
#define ASMREF(t) AsmRef<t>
#if defined(__clang__)
#ifndef CLANG_VER
#error CLANG_VER not defined
#endif
#if CLANG_VER < 14
#define OLD_CLANG 1
#else
#define OLD_CLANG 0
#endif
#else
#define OLD_CLANG 0
#endif
#if OLD_CLANG
#define DUMMY_MARK(p, n) \
    static constexpr int off_##n(void) { return offsetof(p, n); }
#define OFF_M(p, n) p::off_##n
#else
#define DUMMY_MARK(p, n)
#define OFF_M(p, n) []<typename T = p>() constexpr { return offsetof(T, n); }
#endif
#define AR_MEMB(p, t, n, l) \
    DUMMY_MARK(p, n); \
    ArMemb<t, l, p, OFF_M(p, n)> n
#define AR_MEMB_V(p, t, n) \
    DUMMY_MARK(p, n); \
    ArMemb<t, 1, p, OFF_M(p, n), true> n
#define SYM_MEMB(p, t, n) \
    DUMMY_MARK(p, n); \
    SymMemb<t, p, OFF_M(p, n)> n
#define SYM_MEMB2(p, m, o, t, n) \
    SymMemb<t, p, OFF_M(p, m), o> n
#define SYM_MEMB_T(p, t, n) \
    DUMMY_MARK(p, n); \
    SymMembT<t, p, OFF_M(p, n)> n
#define FP_SEG(fp) ((fp).seg())
#define FP_OFF(fp) ((fp).off())
#define FP_SEG_OBJ(o, fp) ((fp).seg(o))
#define FP_OFF_OBJ(o, fp) ((fp).off(o))
#define MK_FP(seg, ofs) ({ \
    uint32_t _s = seg; \
    uint16_t _o = ofs; \
    __FAR(void)(_s, _o); \
})
#define MK_FP_N(seg, ofs) (__FAR(void)(seg, ofs, true))
#define __DOSFAR(t) FarPtrBase<t>
#define __DOSFAR2(t, c) \
    static_assert(!c || std::is_const<t>::value, "should be const"); \
    FarPtrBase<t>
#define _MK_DOS_FP(t, s, o) __FAR(t)(MK_FP(s, o))
#define GET_FP32(f) (f).get_fp32()
#define GET_FAR(f) (f).get_far()
#define GET_PTR(f) (f).get_ptr()
#define ADJUST_FAR(f) (f).adjust_far()
#define _DOS_FP(p) DotBarrierWrap::dot_barrier(p)
#define GET_SYM(w) (w).get_sym()
#define ____R(x) (decltype(x)&)x

#undef NULL
#define NULL           nullptr

#endif
