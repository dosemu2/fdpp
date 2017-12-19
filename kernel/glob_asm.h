/* grep "ASM " globals.h | grep "extern" | grep ";" | sed 's/extern \(.\+\) ASM \(.\+\);/__ASM(\1, \2) SEMIC/' */

__ASM(UWORD, NetBios) SEMIC
__ASM(BYTE *, net_name) SEMIC
__ASM(BYTE, net_set_count) SEMIC
__ASM(char *, inputptr) SEMIC         /* pointer to unread CON input          */
__ASM(sfttbl FAR *, sfthead) SEMIC    /* System File Table head               */
__ASM(WORD, maxsecsize) SEMIC
__ASM(unsigned char, bufloc) SEMIC    /* 0=conv, 1=HMA                        */
__ASM(void far *, deblock_buf) SEMIC  /* pointer to workspace buffer      */
__ASM(struct cds FAR *, CDSp) SEMIC   /* Current Directory Structure          */
__ASM(LONG, current_filepos) SEMIC    /* current file position                */
__ASM(sfttbl FAR *, FCBp) SEMIC       /* FCB table pointer                    */
__ASM(WORD, nprotfcb) SEMIC           /* number of protected fcbs             */
__ASM(UWORD, LoL_nbuffers) SEMIC      /* Number of buffers                    */
__ASM(UBYTE, mem_access_mode) SEMIC   /* memory allocation scheme             */
__ASM(UWORD, Int21AX) SEMIC
__ASM(COUNT, CritErrCode) SEMIC
__ASM(BYTE FAR *, CritErrDev) SEMIC
__ASM(UWORD, wAttr) SEMIC
__ASM(BYTE, default_drive) SEMIC      /* default drive for dos                */
__ASM(dmatch, sda_tmp_dm) SEMIC       /* Temporary directory match buffer     */
__ASM(dmatch, sda_tmp_dm_ren) SEMIC   /* 2nd Temporary directory match buffer */
__ASM(void FAR *, dta) SEMIC
__ASM(seg, cu_psp) SEMIC              /* current psp segment                  */
__ASM(iregs FAR *, user_r) SEMIC      /* User registers for int 21h call      */
__ASM(fcb FAR *, sda_lpFcb) SEMIC     /* Pointer to users fcb                 */
__ASM(sft FAR *, lpCurSft) SEMIC
__ASM(UWORD, return_code) SEMIC       /* Process termination rets             */
__ASM(keyboard, kb_buf) SEMIC
__ASM_ARR(char, local_buffer, LINEBUFSIZE0A) SEMIC
