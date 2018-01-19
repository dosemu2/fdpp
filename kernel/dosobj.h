#ifndef DOSOBJ_H
#define DOSOBJ_H

void dosobj_init(void);
__FAR(void) mk_dosobj(const void *data, UWORD len);
void rm_dosobj(void *data, __FAR(const void) fa, UWORD len);

#endif
