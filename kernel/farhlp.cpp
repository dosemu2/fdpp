#include <map>
#include <cassert>
#include "portab.h"
#include "farptr.hpp"

/* hackish helper to store/lookup far pointers - using static
 * object (map) is an ugly hack in an OOP world.
 * Need this to work around some C++ deficiencies, see comments
 * in farptr.hpp */

static std::map<void *, far_s> far_map;

void store_far(void *ptr, far_s fptr)
{
    far_map[ptr] = fptr;
}

far_s lookup_far(void *ptr)
{
    decltype(far_map)::iterator it = far_map.find(ptr);
    assert(it != far_map.end());
    return it->second;
}
