#include <cstdlib>

template <class T>
struct Mallocator {
  typedef T value_type;
  Mallocator() = default;
  template <class U> constexpr Mallocator(const Mallocator<U>&) noexcept {}
  T* allocate(std::size_t n) {
    return (T*)std::malloc(n*sizeof(T));
  }
  void deallocate(T* p, std::size_t) noexcept { std::free(p); }
};
template <class T, class U>
bool operator==(const Mallocator<T>&, const Mallocator<U>&) { return true; }
template <class T, class U>
bool operator!=(const Mallocator<T>&, const Mallocator<U>&) { return false; }

/* providing custom allocator doesn't seem to work right:
 * https://stackoverflow.com/questions/48915888/allocate-shared-with-malloc
 * So also kill operator delete. */
inline void operator delete(void *, unsigned long) noexcept { std::abort(); }
inline void operator delete(void *) noexcept { std::abort(); }
