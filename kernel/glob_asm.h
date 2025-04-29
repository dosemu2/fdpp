/* grep "ASM " globals.h | grep "extern" | grep ";" | sed 's/extern \(.\+\) ASM \(.\+\);/__ASM(\1, \2) SEMIC/' */

__ASM(UWORD, DOS_PSP) SEMIC
__ASM(UWORD, NetBios) SEMIC
__ASM(BYTE *, net_name) SEMIC
__ASM(BYTE, net_set_count) SEMIC
__ASM_NEAR(char, inputptr) SEMIC         /* pointer to unread CON input          */
__ASM_NEAR(void, nul_strtgy) SEMIC
__ASM_NEAR(void, nul_intr) SEMIC
__ASM_FAR(sfttbl, sfthead) SEMIC    /* System File Table head               */
__ASM(WORD, maxsecsize) SEMIC
__ASM(unsigned char, bufloc) SEMIC    /* 0=conv, 1=HMA                        */
__ASM_FAR(void, deblock_buf) SEMIC  /* pointer to workspace buffer      */
__ASM_FAR(struct cds, CDSp) SEMIC   /* Current Directory Structure          */
__ASM_FAR(BYTE, setverPtr) SEMIC
__ASM(LONG, current_filepos) SEMIC    /* current file position                */
__ASM_FAR(sfttbl, FCBp) SEMIC       /* FCB table pointer                    */
__ASM(WORD, nprotfcb) SEMIC           /* number of protected fcbs             */
__ASM(UWORD, LoL_nbuffers) SEMIC      /* Number of buffers                    */
__ASM(UBYTE, mem_access_mode) SEMIC   /* memory allocation scheme             */
__ASM(UWORD, Int21AX) SEMIC
__ASM(COUNT, CritErrCode) SEMIC
__ASM_FAR(BYTE, CritErrDev) SEMIC
__ASM(UWORD, wAttr) SEMIC
__ASM(BYTE, default_drive) SEMIC      /* default drive for dos                */
__ASM(dmatch, sda_tmp_dm) SEMIC       /* Temporary directory match buffer     */
__ASM(dmatch, sda_tmp_dm_ren) SEMIC   /* 2nd Temporary directory match buffer */
__ASM(dmatch, dmatch_ff) SEMIC        /* directory match buffer for fcb */
__ASM_FAR(void, dta) SEMIC
__ASM(seg, cu_psp) SEMIC              /* current psp segment                  */
__ASM_FAR(iregs, user_r) SEMIC      /* User registers for int 21h call      */
__ASM_FAR(fcb, sda_lpFcb) SEMIC     /* Pointer to users fcb                 */
__ASM_FAR(sft, lpCurSft) SEMIC
__ASM(UWORD, return_code) SEMIC       /* Process termination rets             */
__ASM(keyboard, kb_buf) SEMIC
__ASM_ARR(char, local_buffer, LINEBUFSIZE0A) SEMIC
__ASM(BYTE, NetDelay) SEMIC
__ASM(BYTE, NetRetry) SEMIC
__ASM(UWORD, first_mcb) SEMIC         /* Start of user memory                 */
__ASM(UWORD, uppermem_root) SEMIC
__ASM_FAR(struct dhdr, _clock_) SEMIC /* CLOCK$ device                        */
__ASM_FAR(struct dhdr, syscon) SEMIC /* console device                      */
__ASM_FAR(struct buffer, firstbuf) SEMIC /* head of buffers linked list     */
__ASM_FAR(struct cds, current_ldt) SEMIC
__ASM(UBYTE, nblkdev) SEMIC            /* number of block devices              */
__ASM(UBYTE, lastdrive) SEMIC          /* value of last drive                  */
__ASM(UBYTE, uppermem_link) SEMIC      /* UMB Link flag */
__ASM(UBYTE, PrinterEcho) SEMIC        /* Printer Echo Flag                    */
__ASM(struct dhdr, nul_dev) SEMIC
__ASM(BYTE, ErrorMode) SEMIC           /* Critical error flag                  */
__ASM(BYTE, InDOS) SEMIC               /* In DOS critical section              */
__ASM(BYTE, OpenMode) SEMIC            /* File Open Attributes                 */
__ASM(BYTE, SAttr) SEMIC               /* Attrib Mask for Dir Search           */
__ASM(BYTE, dosidle_flag) SEMIC
__ASM(BYTE, Server_Call) SEMIC
__ASM(BYTE, CritErrLocus) SEMIC
__ASM(BYTE, CritErrAction) SEMIC
__ASM(BYTE, CritErrClass) SEMIC
__ASM(BYTE, VgaSet) SEMIC
__ASM(BYTE, njoined) SEMIC             /* number of joined devices             */
__ASM(struct dirent, SearchDir) SEMIC
//__ASM(struct _FcbSearchBuffer, FcbSearchBuffer) SEMIC
__ASM(struct __PriPathBuffer, _PriPathBuffer) SEMIC
__ASM(struct __SecPathBuffer, _SecPathBuffer) SEMIC
__ASM_ARRI(BYTE, internal_data) SEMIC              /* sda areas                            */
__ASM_ARRI(BYTE, swap_always) SEMIC                /*  "    "                              */
__ASM_ARRI(BYTE, swap_indos) SEMIC                 /*  "    "                              */
__ASM(BYTE, tsr) SEMIC                          /* true if program is TSR               */
__ASM(BYTE, break_flg) SEMIC                    /* true if break was detected           */
__ASM(BYTE, break_ena) SEMIC                    /* break enabled flag                   */
__ASM(struct dirent, DirEntBuffer) SEMIC
__ASM(BYTE, verify_ena) SEMIC                   /* verify enabled flag                  */
__ASM(BYTE, switchar) SEMIC                     /* switch char                          */
__ASM(UBYTE, BootDrive) SEMIC                   /* Drive we came up from                */
__ASM(UBYTE, CPULevel) SEMIC                    /* CPU family, 0=8086, 1=186, ...       */
__ASM(UBYTE, scr_pos) SEMIC                     /* screen position for bs, ht, etc      */
__ASM(struct cds, TempCDS) SEMIC
__ASM_FAR(struct dpb, DPBp) SEMIC             /* First drive Parameter Block          */
__ASM(struct dhdr FAR, clk_dev) SEMIC           /* Clock device driver                  */
__ASM(struct dhdr FAR, con_dev) SEMIC           /* Console device driver                */
__ASM(struct dhdr FAR, prn_dev) SEMIC           /* Generic printer device driver        */
__ASM(struct dhdr FAR, aux_dev) SEMIC           /* Generic aux device driver            */
__ASM(struct dhdr FAR, blk_dev) SEMIC
__ASM(struct ClockRecord, ClkRecord) SEMIC
__ASM(BYTE, os_setver_major) SEMIC              /* editable major version number        */
__ASM(BYTE, os_setver_minor) SEMIC              /* editable minor version number        */
__ASM(BYTE, os_major) SEMIC                     /* major version number                 */
__ASM(BYTE, os_minor) SEMIC                     /* minor version number                 */
//__ASM(BYTE, rev_number) SEMIC                   /* minor version number                 */
__ASM(BYTE, version_flags) SEMIC                /* minor version number                 */
__ASM(struct _KernelConfig FAR, LowKernelConfig) SEMIC
__ASM(struct lol FAR, DATASTART) SEMIC
__ASM_ARRI_F(BYTE, DataStart) SEMIC
__ASM_ARRI_F(BYTE, DataEnd) SEMIC
__ASM_ARRI_F(BYTE, MARK0026H) SEMIC
__ASM(BYTE FAR, _HMATextAvailable) SEMIC     /* first byte of available CODE area    */
__ASM_ARRI_F(BYTE, _HMATextStart) SEMIC          /* first byte of HMAable CODE area      */
__ASM_ARRI_F(BYTE, _HMATextEnd) SEMIC
__ASM_ARRI_F(BYTE, _InitTextStart) SEMIC     /* first available byte of ram          */
__ASM_ARRI_F(BYTE, _InitTextEnd) SEMIC
__ASM_ARRI_F(BYTE, InitEnd) SEMIC
//__ASM(BYTE FAR, ReturnAnyDosVersionExpected) SEMIC
__ASM(BYTE FAR, HaltCpuWhileIdle) SEMIC
__ASM(unsigned char FAR, kbdType) SEMIC
__ASM(struct _nlsCountryInfoHardcoded FAR, nlsCountryInfoHardcoded) SEMIC
__ASM_ARRI_F(char, Dyn) SEMIC
__ASM_ARRI_F(struct RelocationTable, _HMARelocationTableStart) SEMIC
__ASM_ARRI_F(struct RelocationTable, _HMARelocationTableEnd) SEMIC
__ASM_FAR(void, XMSDriverAddress) SEMIC
__ASM(request, ClkReqHdr) SEMIC
__ASM(WORD, current_sft_idx) SEMIC
__ASM(UWORD, ext_open_mode) SEMIC
__ASM(UWORD, ext_open_attrib) SEMIC
__ASM(UWORD, ext_open_action) SEMIC
__ASM(struct nlsInfoBlock, nlsInfo) SEMIC
__ASM(struct nlsPackage FAR, nlsPackageHardcoded) SEMIC
        /* These are the "must have" tables within the hard coded NLS pkg */
__ASM(struct nlsFnamTerm FAR, nlsFnameTermHardcoded) SEMIC
__ASM(struct nlsDBCS FAR, nlsDBCSHardcoded) SEMIC
__ASM(struct nlsCharTbl FAR, nlsUpcaseHardcoded) SEMIC
__ASM(struct nlsCharTbl FAR, nlsFUpcaseHardcoded) SEMIC
__ASM(struct nlsCharTbl FAR, nlsCollHardcoded) SEMIC
__ASM(struct nlsExtCntryInfo FAR, nlsCntryInfoHardcoded) SEMIC
__ASM_FAR(BYTE, hcTablesStart) SEMIC
__ASM_FAR(BYTE, hcTablesEnd) SEMIC
__ASM(request, CharReqHdr) SEMIC
__ASM(request, IoReqHdr) SEMIC
__ASM(request, MediaReqHdr) SEMIC
__ASM_FUNC(int0_handler) SEMIC
__ASM_FUNC(int0_handler_suppress) SEMIC
__ASM_FUNC(int6_handler) SEMIC
__ASM_FUNC(int0d_handler) SEMIC
__ASM_FUNC(int19_handler) SEMIC
__ASM_FUNC(empty_handler) SEMIC
__ASM_FUNC(int20_handler) SEMIC
__ASM_FUNC(int21_handler) SEMIC
__ASM_FUNC(int22_handler) SEMIC
__ASM_FUNC(int24_handler) SEMIC
__ASM_FUNC(low_int25_handler) SEMIC
__ASM_FUNC(low_int26_handler) SEMIC
__ASM_FUNC(int27_handler) SEMIC
__ASM_FUNC(int28_handler) SEMIC
__ASM_FUNC(int29_handler) SEMIC
__ASM_FUNC(int2a_handler) SEMIC
__ASM_FUNC(int2f_handler) SEMIC
__ASM_FAR(void, prev_int0d_handler) SEMIC
__ASM_FAR(void, prev_int21_handler) SEMIC
__ASM_FAR(void, prev_int2f_handler) SEMIC
__ASM_FUNC(got_cbreak) SEMIC
__ASM_FUNC(cpm_entry) SEMIC
__ASM_ARRI_F(const char, os_release) SEMIC
__ASM_NEAR(const char, os_rel) SEMIC
__ASM(UWORD, DaysSinceEpoch) SEMIC
__ASM(UBYTE, p0_exec_mode) SEMIC
__ASM_FAR(const char, p0_cmdline_p) SEMIC
__ASM_FAR(exec_blk, p0_execblk_p) SEMIC
