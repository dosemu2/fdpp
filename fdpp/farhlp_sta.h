#ifndef FARHLP_STA
#define FARHLP_STA

#ifdef __cplusplus
extern "C" {
#endif

enum { SYM_STORE, ARROW_STORE, ARR_STORE, STORE_MAX };
struct fh1 {
    const void *ptr;
    far_t f;
};
extern struct fh1 store[STORE_MAX];

#ifdef __cplusplus
}
#endif

#endif
