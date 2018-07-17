#ifndef OBJHLP_HPP
#define OBJHLP_HPP

class ObjRef {
public:
    virtual void cp() = 0;
    virtual void unref() = 0;
};

bool track_owner(const void *owner, ObjRef *obj);
std::unordered_set<ObjRef *> get_owned_list(const void *owner);

#endif
