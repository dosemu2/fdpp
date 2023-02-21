uint32_t do_asm_call(int num, uint8_t *sp, uint8_t len, int flags);
uint8_t *clean_stk(size_t len);

#define _TFLG_NONE 0
#define _TFLG_FAR 1
#define _TFLG_NORET 2
#define _TFLG_INIT 4
