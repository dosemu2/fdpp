#ifndef THUNKS_PRIV_H
#define THUNKS_PRIV_H

void *resolve_segoff(struct far_s fa);
struct far_s lookup_far(void *ptr);
uint32_t thunk_call_void(struct far_s fa);

#endif
