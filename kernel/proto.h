/****************************************************************/
/*                                                              */
/*                          proto.h                             */
/*                                                              */
/*                   Global Function Prototypes                 */
/*                                                              */
/*                   Copyright (c) 1995, 1996                   */
/*                      Pasquale J. Villani                     */
/*                      All Rights Reserved                     */
/*                                                              */
/* This file is part of DOS-C.                                  */
/*                                                              */
/* DOS-C is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU General Public License  */
/* as published by the Free Software Foundation; either version */
/* 2, or (at your option) any later version.                    */
/*                                                              */
/* DOS-C is distributed in the hope that it will be useful, but */
/* WITHOUT ANY WARRANTY; without even the implied warranty of   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See    */
/* the GNU General Public License for more details.             */
/*                                                              */
/* You should have received a copy of the GNU General Public    */
/* License along with DOS-C; see the file COPYING.  If not,     */
/* write to the Free Software Foundation, 675 Mass Ave,         */
/* Cambridge, MA 02139, USA.                                    */
/****************************************************************/

#ifdef MAIN
#ifdef VERSION_STRINGS
static BYTE *Proto_hRcsId =
    "$Id$";
#endif
#endif

/* blockio.c */
struct buffer FAR *getblk(ULONG blkno, COUNT dsk, BOOL overwrite);
#define getblock(blkno, dsk) getblk(blkno, dsk, FALSE);
#define getblockOver(blkno, dsk) getblk(blkno, dsk, TRUE);
VOID setinvld(REG COUNT dsk);
BOOL flush_buffers(REG COUNT dsk);
BOOL flush(void);
BOOL fill(REG struct buffer FAR * bp, ULONG blkno, COUNT dsk);
BOOL DeleteBlockInBufferCache(ULONG blknolow, ULONG blknohigh, COUNT dsk, int mode);
/* *** Changed on 9/4/00  BER */
UWORD dskxfer(COUNT dsk, ULONG blkno, VOID FAR * buf, UWORD numblocks,
              COUNT mode);
/* *** End of change */
void AllocateHMASpace (size_t lowbuffer, size_t highbuffer);

/* break.c */
unsigned char ctrl_break_pressed(void);
unsigned char check_handle_break(struct dhdr FAR **pdev);
void handle_break(struct dhdr FAR **pdev, int sft_out);
#ifdef __WATCOMC__
#pragma aux handle_break aborts;
#endif

/* chario.c */
struct dhdr FAR *sft_to_dev(sft FAR *sft);
long BinaryCharIO(struct dhdr FAR **pdev, size_t n, void FAR * bp,
                  unsigned command);
int echo_char(int c, int sft_idx);
int ndread(struct dhdr FAR **pdev);
int StdinBusy(void);
void con_flush(struct dhdr FAR **pdev);
unsigned char read_char(int sft_in, int sft_out, BOOL check_break);
unsigned char read_char_stdin(BOOL check_break);
long cooked_read(struct dhdr FAR **pdev, size_t n, char FAR *bp);
void read_line(int sft_in, int sft_out, keyboard FAR * kp);
size_t read_line_handle(int sft_idx, size_t n, char FAR * bp);
void write_char(int c, int sft_idx);
void write_char_stdout(int c);
void update_scr_pos(unsigned char c, unsigned char count);
long cooked_write(struct dhdr FAR **pdev, size_t n, char FAR *bp);

sft FAR *get_sft(UCOUNT);

/* dosfns.c */
const char FAR *get_root(const char FAR *);
BOOL check_break(void);
UCOUNT GenericReadSft(sft far * sftp, UCOUNT n, void FAR * bp,
                      COUNT * err, BOOL force_binary);
COUNT SftSeek(int sft_idx, LONG new_pos, COUNT mode);
/*COUNT DosRead(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err); */
void BinarySftIO(int sft_idx, void *bp, int mode);
#define BinaryIO(hndl, bp, mode) BinarySftIO(get_sft_idx(hndl), bp, mode)
long DosRWSft(int sft_idx, size_t n, void FAR * bp, int mode);
#define DosRead(hndl, n, bp) DosRWSft(get_sft_idx(hndl), n, bp, XFR_READ)
#define DosWrite(hndl, n, bp) DosRWSft(get_sft_idx(hndl), n, bp, XFR_WRITE)
ULONG DosSeek(unsigned hndl, LONG new_pos, COUNT mode);
long DosOpen(char FAR * fname, unsigned flags, unsigned attrib);
COUNT CloneHandle(unsigned hndl);
long DosDup(unsigned Handle);
COUNT DosForceDup(unsigned OldHandle, unsigned NewHandle);
long DosOpenSft(char FAR * fname, unsigned flags, unsigned attrib);
COUNT DosClose(COUNT hndl);
COUNT DosCloseSft(int sft_idx, BOOL commitonly);
#define DosCommit(hndl) DosCloseSft(get_sft_idx(hndl), TRUE)
BOOL DosGetFree(UBYTE drive, UWORD * spc, UWORD * navc,
                UWORD * bps, UWORD * nc);
COUNT DosGetCuDir(UBYTE drive, BYTE FAR * s);
COUNT DosChangeDir(BYTE FAR * s);
COUNT DosFindFirst(UCOUNT attr, BYTE FAR * name);
COUNT DosFindNext(void);
COUNT DosGetFtime(COUNT hndl, date * dp, time * tp);
COUNT DosSetFtimeSft(int sft_idx, date dp, time tp);
#define DosSetFtime(hndl, dp, tp) DosSetFtimeSft(get_sft_idx(hndl), (dp), (tp))
COUNT DosGetFattr(BYTE FAR * name);
COUNT DosSetFattr(BYTE FAR * name, UWORD attrp);
UBYTE DosSelectDrv(UBYTE drv);
COUNT DosDelete(BYTE FAR * path, int attrib);
COUNT DosRename(BYTE FAR * path1, BYTE FAR * path2);
COUNT DosRenameTrue(BYTE * path1, BYTE * path2, int attrib);
COUNT DosMkdir(const char FAR * dir);
COUNT DosRmdir(const char FAR * dir);
struct dhdr FAR *IsDevice(const char FAR * FileName);
BOOL IsShareInstalled(void);
COUNT DosLockUnlock(COUNT hndl, LONG pos, LONG len, COUNT unlock);
int idx_to_sft_(int SftIndex);
sft FAR *idx_to_sft(int SftIndex);
int get_sft_idx(UCOUNT hndl);
struct cds FAR *get_cds(unsigned dsk);
COUNT DosTruename(const char FAR * src, char FAR * dest);

/*dosidle.asm */
VOID ASMCFUNC DosIdle_int(void);
#ifdef __WATCOMC__
#pragma aux (cdecl) DosIdle_int modify exact []
#endif

/* dosnames.c */
int ParseDosName(const char *, char *, BOOL);

/* error.c */
VOID dump(void);
VOID panic(BYTE * s);
VOID fatal(BYTE * err_msg);

/* fatdir.c */
VOID dir_init_fnode(f_node_ptr fnp, CLUSTER dirstart);
f_node_ptr dir_open(const char *dirname);
COUNT dir_read(REG f_node_ptr fnp);
BOOL dir_write(REG f_node_ptr fnp);
VOID dir_close(REG f_node_ptr fnp);
COUNT dos_findfirst(UCOUNT attr, BYTE * name);
COUNT dos_findnext(void);
void ConvertName83ToNameSZ(BYTE FAR * destSZ, BYTE FAR * srcFCBName);
int FileName83Length(BYTE * filename83);

/* fatfs.c */
struct dpb FAR *get_dpb(COUNT dsk);
ULONG clus2phys(CLUSTER cl_no, struct dpb FAR * dpbp);
long dos_open(char * path, unsigned flag, unsigned attrib);
BOOL fcbmatch(const char *fcbname1, const char *fcbname2);
BOOL fcmp_wild(const char * s1, const char * s2, unsigned n);
VOID touc(BYTE * s, COUNT n);
COUNT dos_close(COUNT fd);
COUNT dos_commit(COUNT fd);
COUNT dos_delete(BYTE * path, int attrib);
COUNT dos_rmdir(BYTE * path);
COUNT dos_rename(BYTE * path1, BYTE * path2, int attrib);
date dos_getdate(void);
time dos_gettime(void);
COUNT dos_getftime(COUNT fd, date FAR * dp, time FAR * tp);
COUNT dos_setftime(COUNT fd, date dp, time tp);
ULONG dos_getfsize(COUNT fd);
BOOL dos_setfsize(COUNT fd, LONG size);
COUNT dos_mkdir(BYTE * dir);
BOOL last_link(f_node_ptr fnp);
COUNT map_cluster(REG f_node_ptr fnp, COUNT mode);
long rwblock(COUNT fd, VOID FAR * buffer, UCOUNT count, int mode);
COUNT dos_read(COUNT fd, VOID FAR * buffer, UCOUNT count);
COUNT dos_write(COUNT fd, const VOID FAR * buffer, UCOUNT count);
LONG dos_lseek(COUNT fd, LONG foffset, COUNT origin);
CLUSTER dos_free(struct dpb FAR * dpbp);
BOOL dir_exists(char * path);

VOID trim_path(BYTE FAR * s);

int dos_cd(char * PathName);

f_node_ptr get_f_node(void);
VOID release_f_node(f_node_ptr fnp);
#define release_near_f_node(fnp) ((fnp)->f_count = 0)
VOID dos_setdta(BYTE FAR * newdta);
COUNT dos_getfattr_fd(COUNT fd);
COUNT dos_getfattr(BYTE * name);
COUNT dos_setfattr(BYTE * name, UWORD attrp);
COUNT media_check(REG struct dpb FAR * dpbp);
f_node_ptr xlt_fd(COUNT fd);
COUNT xlt_fnp(f_node_ptr fnp);
struct dhdr FAR * select_unit(COUNT drive);

/* fattab.c */
void read_fsinfo(struct dpb FAR * dpbp);
void write_fsinfo(struct dpb FAR * dpbp);
UCOUNT link_fat(struct dpb FAR * dpbp, CLUSTER Cluster1,
                REG CLUSTER Cluster2);
CLUSTER next_cluster(struct dpb FAR * dpbp, REG CLUSTER ClusterNum);

/* fcbfns.c */
VOID DosOutputString(BYTE FAR * s);
int DosCharInputEcho(VOID);
int DosCharInput(VOID);
VOID DosDirectConsoleIO(iregs FAR * r);
VOID DosCharOutput(COUNT c);
VOID DosDisplayOutput(COUNT c);
BYTE FAR *FatGetDrvData(UBYTE drive, UWORD * spc, UWORD * bps,
                   UWORD * nc);
UWORD FcbParseFname(int *wTestMode, const BYTE FAR *lpFileName, fcb FAR * lpFcb);
const BYTE FAR *ParseSkipWh(const BYTE FAR * lpFileName);
BOOL TestCmnSeps(BYTE FAR * lpFileName);
BOOL TestFieldSeps(BYTE FAR * lpFileName);
const BYTE FAR *GetNameField(const BYTE FAR * lpFileName, BYTE FAR * lpDestField,
                       COUNT nFieldSize, BOOL * pbWildCard);
UBYTE FcbReadWrite(xfcb FAR *, UCOUNT, int);
UBYTE FcbGetFileSize(xfcb FAR * lpXfcb);
void FcbSetRandom(xfcb FAR * lpXfcb);
UBYTE FcbRandomBlockIO(xfcb FAR * lpXfcb, UWORD *nRecords, int mode);
UBYTE FcbRandomIO(xfcb FAR * lpXfcb, int mode);
UBYTE FcbOpen(xfcb FAR * lpXfcb, unsigned flags);
UBYTE FcbDelete(xfcb FAR * lpXfcb);
UBYTE FcbRename(xfcb FAR * lpXfcb);
UBYTE FcbClose(xfcb FAR * lpXfcb);
void FcbCloseAll(void);
UBYTE FcbFindFirstNext(xfcb FAR * lpXfcb, BOOL First);

/* intr.asm */
COUNT ASMPASCAL res_DosExec(COUNT mode, exec_blk * ep, BYTE * lp);
UCOUNT ASMPASCAL res_read(int fd, void *buf, UCOUNT count);
#ifdef __WATCOMC__
#pragma aux (pascal) res_DosExec modify exact [ax bx dx es]
#pragma aux (pascal) res_read modify exact [ax bx cx dx]
#endif

/* ioctl.c */
COUNT DosDevIOctl(lregs * r);

/* memmgr.c */
seg far2para(VOID FAR * p);
seg long2para(ULONG size);
void FAR *add_far(void FAR * fp, unsigned off);
VOID FAR *adjust_far(const void FAR * fp);
COUNT DosMemAlloc(UWORD size, COUNT mode, seg * para, UWORD * asize);
COUNT DosMemLargest(UWORD * size);
COUNT DosMemFree(UWORD para);
COUNT DosMemChange(UWORD para, UWORD size, UWORD * maxSize);
COUNT DosMemCheck(void);
COUNT FreeProcessMem(UWORD ps);
COUNT DosGetLargestBlock(UWORD * block);
VOID show_chain(void);
VOID DosUmbLink(BYTE n);
VOID mcb_print(mcb FAR * mcbp);

/* lfnapi.c */
COUNT lfn_allocate_inode(VOID);
COUNT lfn_free_inode(COUNT handle);

COUNT lfn_setup_inode(COUNT handle, ULONG dirstart, UWORD diroff);

COUNT lfn_create_entries(COUNT handle, lfn_inode_ptr lip);
COUNT lfn_remove_entries(COUNT handle);

COUNT lfn_dir_read(COUNT handle, lfn_inode_ptr lip);
COUNT lfn_dir_write(COUNT handle);

/* nls.c */
BYTE DosYesNo(unsigned char ch);
#ifndef DosUpMem
VOID DosUpMem(VOID FAR * str, unsigned len);
#endif
unsigned char ASMCFUNC DosUpChar(unsigned char ch);
VOID DosUpString(char FAR * str);
VOID DosUpFMem(VOID FAR * str, unsigned len);
unsigned char DosUpFChar(unsigned char ch);
VOID DosUpFString(char FAR * str);
COUNT DosGetData(int subfct, UWORD cp, UWORD cntry, UWORD bufsize,
                 VOID FAR * buf);
#ifndef DosGetCountryInformation
COUNT DosGetCountryInformation(UWORD cntry, VOID FAR * buf);
#endif
#ifndef DosSetCountry
COUNT DosSetCountry(UWORD cntry);
#endif
COUNT DosGetCodepage(UWORD * actCP, UWORD * sysCP);
COUNT DosSetCodepage(UWORD actCP, UWORD sysCP);
UWORD ASMCFUNC syscall_MUX14(DIRECT_IREGS);

/* prf.c */
#ifdef DEBUG
int VA_CDECL printf(const char * fmt, ...);
int VA_CDECL sprintf(char * buff, const char * fmt, ...);
#endif
VOID hexd(char *title, VOID FAR * p, COUNT numBytes);
void put_unsigned(unsigned n, int base, int width);
void put_string(const char *s);
void put_console(int);

/* strings.c */
size_t /* ASMCFUNC */ ASMPASCAL strlen(const char * s);
size_t /* ASMCFUNC */ ASMPASCAL fstrlen(const char FAR * s);
char FAR * /*ASMCFUNC*/ ASMPASCAL _fstrcpy(char FAR * d, const char FAR * s);
int /*ASMCFUNC*/ ASMPASCAL strcmp(const char * d, const char * s);
int /*ASMCFUNC*/ ASMPASCAL fstrcmp(const char FAR * d, const char FAR * s);
int /*ASMCFUNC*/ ASMPASCAL fstrncmp(const char FAR * d, const char FAR * s, size_t l);
int /*ASMCFUNC*/ ASMPASCAL strncmp(const char * d, const char * s, size_t l);
char * /*ASMCFUNC*/ ASMPASCAL strchr(const char * s, int c);
char FAR * /*ASMCFUNC*/ ASMPASCAL fstrchr(const char FAR * s, int c);
void FAR * /*ASMCFUNC*/ ASMPASCAL fmemchr(const void FAR * s, int c, size_t n);

/* misc.c */
char * /*ASMCFUNC*/ ASMPASCAL strcpy(char * d, const char * s);
void /*ASMCFUNC*/ ASMPASCAL fmemcpyBack(void FAR * d, const void FAR * s, size_t n);
void /*ASMCFUNC*/ ASMPASCAL fmemcpy(void FAR * d, const void FAR * s, size_t n);
void /*ASMCFUNC*/ ASMPASCAL fstrcpy(char FAR * d, const char FAR * s);
void * /*ASMCFUNC*/ ASMPASCAL memcpy(void *d, const void * s, size_t n);
void * /*ASMCFUNC*/ ASMPASCAL fmemset(void FAR * s, int ch, size_t n);
void * /*ASMCFUNC*/ ASMPASCAL memset(void * s, int ch, size_t n);

int /*ASMCFUNC*/ ASMPASCAL memcmp(const void *m1, const void *m2, size_t n);
int /*ASMCFUNC*/ ASMPASCAL fmemcmp(const void FAR *m1, const void FAR *m2, size_t n);

#ifdef __WATCOMC__
/* bx, cx, dx and es not used or clobbered for all asmsupt.asm functions except
   (f)memchr/(f)strchr (which clobber dx) */
#pragma aux (pascal) pascal_ax modify exact [ax]
#pragma aux (pascal_ax) fmemcpy
#pragma aux (pascal_ax) memcpy
#pragma aux (pascal_ax) fmemset
#pragma aux (pascal_ax) memset
#pragma aux (pascal_ax) fmemcmp modify nomemory
#pragma aux (pascal_ax) memcmp modify nomemory
#pragma aux (pascal_ax) fstrcpy
#pragma aux (pascal_ax) strcpy
#pragma aux (pascal_ax) fstrlen modify nomemory
#pragma aux (pascal_ax) strlen modify nomemory
#pragma aux (pascal) memchr modify exact [ax dx] nomemory
#pragma aux (pascal) fmemchr modify exact [ax dx] nomemory
#pragma aux (pascal) strchr modify exact [ax dx] nomemory
#pragma aux (pascal) fstrchr modify exact [ax dx] nomemory
#endif

/* sysclk.c */
COUNT BcdToByte(COUNT x);
COUNT BcdToWord(BYTE * x, UWORD * mon, UWORD * day, UWORD * yr);
COUNT ByteToBcd(COUNT x);
LONG WordToBcd(BYTE * x, UWORD * mon, UWORD * day, UWORD * yr);

/* syspack.c */
#ifdef NONNATIVE
VOID getdirent(UBYTE FAR * vp, struct dirent FAR * dp);
VOID putdirent(struct dirent FAR * dp, UBYTE FAR * vp);
#else
#define getdirent(vp, dp) fmemcpy(dp, vp, sizeof(struct dirent))
#define putdirent(dp, vp) fmemcpy(vp, dp, sizeof(struct dirent))
#endif

/* systime.c */
void DosGetTime(struct dostime *dt);
int DosSetTime(const struct dostime *dt);
unsigned char DosGetDate(struct dosdate *dd);
int DosSetDate(const struct dosdate *dd);

const UWORD *is_leap_year_monthdays(UWORD year);
UWORD DaysFromYearMonthDay(UWORD Year, UWORD Month, UWORD DayOfMonth);

/* task.c */
VOID new_psp(seg para, seg cur_psp);
VOID child_psp(seg para, seg cur_psp, int psize);
VOID return_user(void);
COUNT DosExec(COUNT mode, exec_blk FAR * ep, BYTE FAR * lp);
ULONG SftGetFsize(int sft_idx);
VOID InitPSP(VOID);

/* newstuff.c */
int SetJFTSize(UWORD nHandles);
int DosMkTmp(BYTE FAR * pathname, UWORD attr);
COUNT truename(const char FAR * src, char * dest, COUNT t);

/* network.c */
COUNT ASMPASCAL network_redirector(unsigned cmd);
COUNT ASMCFUNC remote_doredirect(UWORD b, UCOUNT n, UWORD d, VOID FAR * s,
                                 UWORD i, VOID FAR * data);
COUNT ASMCFUNC remote_printset(UWORD b, UCOUNT n, UWORD d, VOID FAR * s,
                               UWORD i, VOID FAR * data);
COUNT ASMCFUNC remote_process_end(VOID);
COUNT ASMCFUNC remote_findfirst(VOID FAR * s);
COUNT ASMCFUNC remote_findnext(VOID FAR * s);
COUNT ASMCFUNC remote_getfree(VOID FAR * s, VOID * d);
COUNT ASMCFUNC remote_open(sft FAR * s, COUNT mode);
int ASMCFUNC remote_extopen(sft FAR * s, unsigned attr);
LONG ASMCFUNC remote_lseek(sft FAR * s, LONG new_pos);
UCOUNT ASMCFUNC remote_read(sft FAR * s, UCOUNT n, COUNT * err);
UCOUNT ASMCFUNC remote_write(sft FAR * s, UCOUNT n, COUNT * err);
COUNT ASMCFUNC remote_creat(sft FAR * s, COUNT attr);
COUNT ASMCFUNC remote_setfattr(COUNT attr);
COUNT ASMCFUNC remote_printredir(UCOUNT dx, UCOUNT ax);
COUNT ASMCFUNC remote_commit(sft FAR * s);
COUNT ASMCFUNC remote_close(sft FAR * s);
COUNT ASMCFUNC QRemote_Fn(char FAR * d, const char FAR * s);
#ifdef __WATCOMC__
/* bx, cx, and es not used or clobbered for all remote functions,
 * except lock_unlock and process_end */
#pragma aux cdecl_axdx "_*" parm caller [] modify exact [ax dx]
#pragma aux (cdecl_axdx) remote_doredirect
#pragma aux (cdecl_axdx) remote_printset
#pragma aux (cdecl_axdx) remote_findfirst
#pragma aux (cdecl_axdx) remote_findnext
#pragma aux (cdecl_axdx) remote_getfree
#pragma aux (cdecl_axdx) remote_open
#pragma aux (cdecl_axdx) remote_extopen
#pragma aux (cdecl_axdx) remote_lseek
#pragma aux (cdecl_axdx) remote_read
#pragma aux (cdecl_axdx) remote_write
#pragma aux (cdecl_axdx) remote_creat
#pragma aux (cdecl_axdx) remote_setfattr
#pragma aux (cdecl_axdx) remote_printredir
#pragma aux (cdecl_axdx) remote_commit
#pragma aux (cdecl_axdx) remote_close
#pragma aux (cdecl_axdx) QRemote_Fn
#pragma aux (pascal) network_redirector modify exact [ax dx]
#endif

UWORD get_machine_name(BYTE FAR * netname);
VOID set_machine_name(BYTE FAR * netname, UWORD name_num);

/* procsupt.asm */
VOID ASMCFUNC exec_user(iregs FAR * irp, int disable_a20);

/* new by TE */

/*
    assert at compile time, that something is true.
    
    use like 
        ASSERT_CONST( SECSIZE == 512) 
        ASSERT_CONST( (BYTE FAR *)x->fcb_ext - (BYTE FAR *)x->fcbname == 8)
*/

#define ASSERT_CONST(x) { typedef struct { char _xx[x ? 1 : -1]; } xx ; }

