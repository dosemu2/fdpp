/*
    DynData.h

    declarations for dynamic NEAR data allocations

    the DynBuffer must initially be large enough to hold
    the PreConfig data.
    after the disksystem has been initialized, the kernel is
    moveable and Dyn.Buffer resizable, but not before
*/

#ifdef __cplusplus
extern "C" {
#endif
far_t DynAlloc(const char *what, unsigned num, unsigned size);
far_t DynLast(void);
void DynFree(void *ptr);
#ifdef __cplusplus
}
#endif
