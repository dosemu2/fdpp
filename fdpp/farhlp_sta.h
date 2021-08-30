#ifndef FARHLP_STA
#define FARHLP_STA

enum { SYM_STORE, ARROW_STORE, ASTER_STORE, ARR_STORE, STORE_MAX };
struct fh1 {
    const void *ptr;
    far_t f;
};
extern struct fh1 store[STORE_MAX];

#endif
