/* grep "ASM " globals.h | grep "extern" | grep ";" | sed 's/extern \(.\+\) ASM \(.\+\);/__ASM(\1, \2);/' */

__ASM(UWORD, NetBios);
__ASM(BYTE *, net_name);
__ASM(BYTE, net_set_count);
__ASM(char *, inputptr);         /* pointer to unread CON input          */
__ASM(sfttbl FAR *, sfthead);    /* System File Table head               */
__ASM(WORD, maxsecsize);         /* largest sector size in use (can use) */
__ASM(unsigned char, bufloc);    /* 0=conv, 1=HMA                        */
__ASM(void far *, deblock_buf);  /* pointer to workspace buffer      */
__ASM(struct cds FAR *, CDSp);   /* Current Directory Structure          */
__ASM(LONG, current_filepos);    /* current file position                */
__ASM(sfttbl FAR *, FCBp);       /* FCB table pointer                    */
__ASM(WORD, nprotfcb);           /* number of protected fcbs             */
__ASM(UWORD, LoL_nbuffers);      /* Number of buffers                    */
__ASM(UBYTE, mem_access_mode);   /* memory allocation scheme             */
__ASM(UWORD, Int21AX);
__ASM(COUNT, CritErrCode);
__ASM(BYTE FAR *, CritErrDev);
__ASM(UWORD, wAttr);
__ASM(BYTE, default_drive);      /* default drive for dos                */
__ASM(dmatch, sda_tmp_dm);       /* Temporary directory match buffer     */
__ASM(dmatch, sda_tmp_dm_ren);   /* 2nd Temporary directory match buffer */
__ASM(void FAR *, dta);          /* Disk transfer area (kludge)          */
__ASM(seg, cu_psp);              /* current psp segment                  */
__ASM(iregs FAR *, user_r);      /* User registers for int 21h call      */
__ASM(fcb FAR *, sda_lpFcb);     /* Pointer to users fcb                 */
__ASM(sft FAR *, lpCurSft);
__ASM(UWORD, return_code);       /* Process termination rets             */
__ASM(keyboard, kb_buf);
__ASM_ARR(char, local_buffer, LINEBUFSIZE0A);
