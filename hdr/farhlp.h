#ifndef FARHLP_H
#define FARHLP_H

struct f_m;

struct farhlp {
    struct f_m *far_map;
    int f_m_size;
    int f_m_len;
};

extern struct farhlp g_farhlp;

void farhlp_init(struct farhlp *ctx);
void store_far(struct farhlp *ctx, const void *ptr, far_s fptr);
far_s lookup_far(struct farhlp *ctx, const void *ptr);

#endif
