#ifndef DOSOBJ_H
#define DOSOBJ_H

void dosobj_init(void);
__DOSFAR(uint8_t) mk_dosobj(const void *data, UWORD len);
void pr_dosobj(__DOSFAR(uint8_t) fa, const void *data, UWORD len);
void cp_dosobj(void *data, __DOSFAR(uint8_t) fa, UWORD len);
void rm_dosobj(__DOSFAR(uint8_t) fa);
__DOSFAR(uint8_t) mk_dosobj_st(const void *data, UWORD len);

#endif
