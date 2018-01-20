#ifndef DOSOBJ_H
#define DOSOBJ_H

void dosobj_init(void);
__FAR(void) mk_dosobj(const void *data, UWORD len);
void pr_dosobj(__FAR(void) fa, const void *data, UWORD len);
void cp_dosobj(void *data, __FAR(const void) fa, UWORD len);
void rm_dosobj(__FAR(void) fa);

#endif
