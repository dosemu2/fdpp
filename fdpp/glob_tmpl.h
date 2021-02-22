#define __ASM(t, v) _E __ASMSYM(t) __##v
#define __ASM_FAR(t, v) _E __ASMFAR(t) __##v
#define __ASM_NEAR(t, v) _E __ASMNEAR(t, data_seg) __##v
#define __ASM_ARR(t, v, l) _E __ASMARSYM(t, l) __##v
#define __ASM_ARRI(t, v) _E __ASMARISYM(t, data_seg) __##v
#define __ASM_ARRI_F(t, v) _E __ASMARIFSYM(t) __##v
#define __ASM_FUNC(v) _E __ASMFSYM(void) __##v
#define SEMIC ;
#include <glob_asm.h>
#undef __ASM
#undef __ASM_FAR
#undef __ASM_NEAR
#undef __ASM_ARR
#undef __ASM_ARRI
#undef __ASM_ARRI_F
#undef __ASM_FUNC
#undef SEMIC
