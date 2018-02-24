#ifndef CPPSTUBS_HPP
#define CPPSTUBS_HPP

#include <cstdlib>

/* providing custom allocator doesn't seem to work right:
 * https://stackoverflow.com/questions/48915888/allocate-shared-with-malloc
 * So also kill operator delete. */
inline void operator delete(void *, unsigned long) noexcept { std::abort(); }
inline void operator delete(void *) noexcept { std::abort(); }

extern "C" inline void __cxa_pure_virtual () { std::abort(); }

#endif
