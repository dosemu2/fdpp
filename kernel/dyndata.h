/*
    DynData.h

    declarations for dynamic NEAR data allocations

    the DynBuffer must initially be large enough to hold
    the PreConfig data.
    after the disksystem has been initialized, the kernel is
    moveable and Dyn.Buffer resizable, but not before
*/

__FAR(void) DynAlloc(const char *what, unsigned num, unsigned size);
__FAR(void) DynLast(void);
void DynFree(void *ptr);

struct DynS {
  unsigned Allocated;
  AR_MEMB(struct DynS, char, Buffer, 1);
};
