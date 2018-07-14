/* silly farobj helpers */

#include <unordered_set>
#include <map>
#include "objhlp.hpp"

static std::map<const void *, std::unordered_set<ObjRef *> > omap;

bool track_owner(const void *owner, ObjRef *obj)
{
    std::unordered_set<ObjRef *>& ent = omap[owner];
    return ent.insert(obj).second;
}

std::unordered_set<ObjRef *> get_owned_list(const void *owner)
{
    std::unordered_set<ObjRef *> ret;
    if (omap.find(owner) != omap.end()) {
        ret = omap[owner];
        omap.erase(owner);
    }
    return ret;
}
