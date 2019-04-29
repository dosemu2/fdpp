/*
    DynData.h

    declarations for dynamic NEAR data allocations

    the DynBuffer must initially be large enough to hold
    the PreConfig data.
    after the disksystem has been initialized, the kernel is
    moveable and Dyn.Buffer resizable, but not before
*/

#ifndef DYN_H
#define DYN_H

struct DynS {
  AR_MEMB(DynS, char, Buffer, 2048);
};

#endif
