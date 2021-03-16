/****************************************************************/
/*                                                              */
/*                           main.c                             */
/*                            DOS-C                             */
/*                                                              */
/*                    Main Kernel Functions                     */
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
/* write to the Free Software Foundation, Inc.,                 */
/* 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.     */
/****************************************************************/

#include "portab.h"
#include "globals.h"
#include "init-mod.h"
#include "dyndata.h"
#include "dosobj.h"
#include "debug.h"

#ifdef VERSION_STRINGS
static const char *mainRcsId =
    "$Id: main.c 1699 2012-01-16 20:45:44Z perditionc $";
#endif

static const char *copyright =
    "Written by Stas Sergeev, FDPP project.\n"
    "Based on FreeDOS sources (C) Pasquale J. Villani and The FreeDOS Project.\n\n"
    "This program is free software: you can redistribute it and/or modify\n"
    "it under the terms of the GNU General Public License as published by\n"
    "the Free Software Foundation, either version 3 of the License, or\n"
    "(at your option) any later version.\n\n";

BSSZ(struct _KernelConfig, InitKernelConfig);

STATIC VOID InitIO(void);

STATIC VOID update_dcb(struct dhdr FAR *);
STATIC VOID init_kernel(VOID);
STATIC VOID signon(VOID);
STATIC VOID kernel(VOID);
STATIC VOID FsConfig(BOOL reinit);
STATIC VOID InitPrinters(VOID);
STATIC VOID InitSerialPorts(VOID);
#ifndef FDPP
STATIC void CheckContinueBootFromHarddisk(void);
#endif
STATIC void setup_int_vectors(void);

#ifdef _MSC_VER
BYTE _acrtused = 0;

__segment DosDataSeg = 0;       /* serves for all references to the DOS DATA segment
                                   necessary for MSC+our funny linking model
                                 */
__segment DosTextSeg = 0;

#endif

ASMREF(struct lol) LoL = __ASMADDR(DATASTART);
struct _bprm FAR *bprm;
static UBYTE FAR *bprm_buf;
#define TEXT_SIZE (_InitTextEnd - _HMATextStart)

VOID ASMCFUNC FreeDOSmain(void)
{
  unsigned char drv;
  unsigned char FAR *p;
  UWORD relo;

#ifdef _MSC_VER
  extern FAR prn_dev;
  DosDataSeg = (__segment) & DATASTART;
  DosTextSeg = (__segment) & prn_dev;
#endif
  /* back up kernel code to the top of ram */
  lpTop = MK_FP(_SS - (TEXT_SIZE + 15) / 16, 0);
  fmemcpy(lpTop, _HMATextStart, TEXT_SIZE);
  RelocHook(_CS, _CS + ((lpTop - _HMATextStart) >> 4),
      FP_OFF(_InitTextStart), _InitTextEnd - _InitTextStart);
  /* purge HMAText so that no one uses it on init stage */
  fmemset(_HMATextStart, 0xcc, _HMATextEnd - _HMATextStart);
  /* try to split data segment out of our TINY model */
  ___assert(!(FP_OFF(DataStart) & 0xf));
  relo = FP_OFF(DataStart) >> 4;
  RelocSplitSeg(FP_SEG(&Dyn), FP_SEG(&Dyn) + relo, FP_OFF(&Dyn), 0);
  RelocSplitSeg(FP_SEG(swap_indos), FP_SEG(swap_indos) + relo,
      FP_OFF(swap_indos), 0);
  /* DataStart should be split after others */
  RelocSplitSeg(FP_SEG(DataStart), FP_SEG(DataStart) + relo,
      FP_OFF(DataStart), DataEnd - DataStart);
  setDS(FP_SEG(&DATASTART));
  setES(FP_SEG(&DATASTART));

#ifdef FDPP
  objhlp_reset();
  run_ctors();
#endif

  DynInit(MK_FP(FP_SEG(LoL), FP_OFF(&Dyn)));

#ifdef FDPP
#define DOSOBJ_POOL 512
  far_t fa = DynAlloc("dosobj", 1, DOSOBJ_POOL);
  dosobj_init(fa, DOSOBJ_POOL);
#endif

                        /*  if the kernel has been UPX'ed,
                                CONFIG info is stored at 50:e2 ..fc
                            and the bootdrive (passed from BIOS)
                            at 50:e0
                        */

  drv = LoL->_BootDrive + 1;
  p = (unsigned char FAR *)MK_FP(0, 0x5e0);
  if (fmemcmp(p+2,"CONFIG",6) == 0)      /* UPX */
  {
    fmemcpy_n(&InitKernelConfig, p+2, sizeof(InitKernelConfig));

    drv = *p + 1;
    *(DWORD FAR *)(p+2) = 0;
  }
  else
  {
    *p = drv - 1;
    fmemcpy_n(&InitKernelConfig, &LowKernelConfig, sizeof(InitKernelConfig));
  }

  if (drv >= 0x80)
    drv = (LoL->_BootDrive & ~0x80) + 3; /* HDD */
  LoL->_BootDrive = drv;

  if (LoL->_BootParamVer != BPRM_VER)
    fpanic("Wrong boot param version %i, need %i", LoL->_BootParamVer, BPRM_VER);
  bprm_buf = (UBYTE FAR *)DynAlloc("bprm", 1, sizeof(struct _bprm) + 15);
  bprm = AlignParagraph(bprm_buf);
  fmemcpy_n(bprm, MK_FP(LoL->_BootParamSeg, 0), sizeof(struct _bprm));
  LoL->_BootParamSeg = FP_SEG(bprm);
  ___assert(FP_SEG(bprm) >= FP_SEG(adjust_far(bprm_buf)) && FP_OFF(bprm) == 0);

#ifndef FDPP
  CheckContinueBootFromHarddisk();
#endif
  /* initialize all internal variables, process CONFIG.SYS, load drivers, etc */
  init_kernel();

  /* display copyright info and kernel emulation status */
  signon();

#ifdef DEBUG
  /* Non-portable message kludge alert!   */
  DebugPrintf(("KERNEL: Boot drive = %c\n", 'A' + LoL->_BootDrive - 1));
#endif

  /* purge INIT_TEXT to make sure its not used by mistake */
  memset(_InitTextStart, 0xcc, _InitTextEnd - _InitTextStart);
  PurgeHook(_InitTextStart, _InitTextEnd - _InitTextStart);

  DoInstall();
  configPreBoot();
  kernel();
}

/*
    InitializeAllBPBs()

    or MakeNortonDiskEditorHappy()

    it has been determined, that FDOS's BPB tables are initialized,
    only when used (like DIR H:).
    at least one known utility (norton DE) seems to access them directly.
    ok, so we access for all drives, that the stuff gets build
*/
STATIC void InitializeAllBPBs(VOID)
{
  static char filename[] = "A:-@JUNK@-.TMP";
  int drive, fileno;
  for (drive = 'C'; drive < 'A' + LoL->_nblkdev; drive++)
  {
    filename[0] = drive;
    if ((fileno = open(filename, O_RDONLY)) >= 0)
      close(fileno);
  }
}

STATIC void PSPInit(void)
{
  psp FAR *p = MK_FP(DOS_PSP, 0);

  /* Clear out new psp first                              */
  fmemset(p, 0, sizeof(psp));

  /* initialize all entries and exits                     */
  /* CP/M-like exit point                                 */
  p->ps_exit = PSP_SIG;

  /* CP/M-like entry point - call far to special entry    */
  p->ps_farcall = 0x9a;
  p->ps_reentry = MK_FP(0, 0x30 * 4);
  /* unix style call - 0xcd 0x21 0xcb (int 21, retf)      */
  p->ps_unix[0] = 0xcd;
  p->ps_unix[1] = 0x21;
  p->ps_unix[2] = 0xcb;

  /* Now for parent-child relationships                   */
  /* parent psp segment                                   */
  p->ps_parent = FP_SEG(p);
  /* previous psp pointer                                 */
  p->ps_prevpsp = MK_FP(0xffff,0xffff);

  /* Environment and memory useage parameters             */
  /* memory size in paragraphs                            */
  /*  p->ps_size = 0; clear from above                    */
  /* environment paragraph                                */
  p->ps_environ = 0;
  /* terminate address                                    */
  p->ps_isv22 = getvec(0x22);
  /* break address                                        */
  p->ps_isv23 = getvec(0x23);
  /* critical error address                               */
  p->ps_isv24 = getvec(0x24);

  /* user stack pointer - int 21                          */
  /* p->ps_stack = NULL; clear from above                 */

  /* File System parameters                               */
  /* maximum open files                                   */
  p->ps_maxfiles = 20;
  fmemset(p->ps_files, 0xff, 20);

  /* open file table pointer                              */
  p->ps_filetab = p->ps_files;

  /* default system version for int21/ah=30               */
  p->ps_retdosver = (LoL->_os_setver_minor << 8) + LoL->_os_setver_major;

  /* first command line argument                          */
  /* p->ps_fcb1.fcb_drive = 0; already set                */
  fmemset(_DOS_FP(p->ps_fcb1).fcb_fname, ' ', FNAME_SIZE + FEXT_SIZE);
  /* second command line argument                         */
  /* p->ps_fcb2.fcb_drive = 0; already set                */
  fmemset(_DOS_FP(p->ps_fcb2).fcb_fname, ' ', FNAME_SIZE + FEXT_SIZE);

  /* local command line                                   */
  /* p->ps_cmd.ctCount = 0;     command tail, already set */
  p->ps_cmd.ctBuffer[0] = 0xd; /* command tail            */
}

#ifndef __WATCOMC__
/* for WATCOMC we can use the ones in task.c */
intvec getvec(unsigned char intno)
{
  intvec iv;
  disable();
  iv = *(intvec FAR *)MK_FP_N(0,4 * (intno));
  enable();
  return iv;
}

void setvec(unsigned char intno, intvec vector)
{
  disable();
  *(intvec FAR *)MK_FP_N(0,4 * intno) = vector;
  enable();
}
#endif

STATIC void setup_int_vectors(void)
{
  struct vec
  {
    unsigned char intno;
    UWORD handleroff;
  } vectors[] =
    {
      /* all of these are in the DOS DS */
      { 0x0, FP_OFF(int0_handler) },   /* zero divide */
      { 0x1, FP_OFF(empty_handler) },  /* single step */
      { 0x3, FP_OFF(empty_handler) },  /* debug breakpoint */
      { 0x6, FP_OFF(int6_handler) },   /* invalid opcode */
      { 0x19, FP_OFF(int19_handler) },
      { 0x20, FP_OFF(int20_handler) },
      { 0x21, FP_OFF(int21_handler) },
//      { 0x22, FP_OFF(int22_handler) },
      { 0x24, FP_OFF(int24_handler) },
      { 0x25, FP_OFF(low_int25_handler) },
      { 0x26, FP_OFF(low_int26_handler) },
      { 0x27, FP_OFF(int27_handler) },
//      { 0x28, FP_OFF(int28_handler) },
//      { 0x2a, FP_OFF(int2a_handler) },
      { 0x2f, FP_OFF(int2f_handler) }
    };
  struct vec *pvec;
  struct lowvec FAR *plvec;
  void FAR *old_vec;
  int i;

  for (plvec = intvec_table; plvec < intvec_table + 5; plvec++)
    plvec->isv = getvec(plvec->intno);

  old_vec = getvec(0x21);
  if (old_vec)
    prev_int21_handler = old_vec;
  else
    prev_int21_handler = empty_handler;
  old_vec = getvec(0x2f);
  if (old_vec)
    prev_int2f_handler = old_vec;
  else
    prev_int2f_handler = empty_handler;

  for (i = 0x23; i <= 0x3f; i++) {
    old_vec = getvec(i);
    if (old_vec == NULL)
      setvec(i, empty_handler);
  }
  HaltCpuWhileIdle = 0;
  for (pvec = vectors; pvec < vectors + (sizeof vectors/sizeof *pvec); pvec++)
    setvec(pvec->intno, (intvec)MK_FP(FP_SEG(empty_handler), pvec->handleroff));
  pokeb(0, 0x30 * 4, 0xea);
  pokel(0, 0x30 * 4 + 1, (ULONG)cpm_entry);

  /* these two are in the device driver area LOWTEXT (0x70) */
  setvec(0x1b, got_cbreak);
  setvec(0x29, int29_handler);  /* required for printf! */
}

STATIC void init_kernel(void)
{
  COUNT i;

  /* Init oem hook - returns memory size in KB    */
  ram_top = init_oem();

  /* move kernel to high conventional RAM */
  MoveKernel(FP_SEG(lpTop));
  lpTop = MK_FP(FP_SEG(lpTop) - 0xfff, 0xfff0);

  /* install DOS API and other interrupt service routines, basic kernel functionality works */
  setup_int_vectors();
  /* Initialize IO subsystem                                      */
  InitIO();
  InitPrinters();
  InitSerialPorts();

  init_PSPSet(DOS_PSP);
  set_DTA(MK_FP(DOS_PSP, 0x80));
  PSPInit();

  Init_clk_driver();

  /* Do first initialization of system variable buffers so that   */
  /* we can read config.sys later.  */

  /* use largest possible value for the initial CDS */
  LoL->_lastdrive = 26;

  /*  init_device((struct dhdr FAR *)&blk_dev, NULL, 0, &ram_top); */
  blk_dev.dh_name[0] = dsk_init();

  PreConfig();

  /* Number of units */
  if (blk_dev.dh_name[0] > 0)
    update_dcb(&blk_dev);

  /* Now config the temporary file system */
  FsConfig(0);

  /* Now process CONFIG.SYS     */
  DoConfig(0);
  DoConfig(1);

  /* initialize near data and MCBs */
  PreConfig2();
  /* and process CONFIG.SYS one last time for device drivers */
  DoConfig(2);


  /* Close all (device) files */
  for (i = 0; i < 20; i++)
    close(i);

  /* and do final buffer allocation. */
  PostConfig();

  /* Init the file system one more time     */
  FsConfig(1);

  configDone();

  InitializeAllBPBs();
}

#define safe_open(n, m) \
  if (open(n, m) == -1) \
    init_fatal("unable to open " n)

STATIC VOID FsConfig(BOOL reinit)
{
  struct dpb FAR *dpb = LoL->_DPBp;
  int i;

  /* Initialize the current directory structures    */
  for (i = 0; i < LoL->_lastdrive; i++)
  {
    struct cds FAR *pcds_table = &LoL->_CDSp[i];

    if (reinit && old_CDSp)
    {
      struct cds FAR *old_cds = &old_CDSp[i];
      if (old_cds->cdsFlags)
      {
        fmemcpy(pcds_table, old_cds, sizeof(struct cds));
        if (i < LoL->_nblkdev && (ULONG) dpb != 0xffffffffl)
          dpb = dpb->dpb_next;
        continue;
      }
    }
    memcpy(pcds_table->cdsCurrentPath, "A:\\\0", 4);

    pcds_table->cdsCurrentPath[0] += i;
    pcds_table->cdsDpb = NULL;
    pcds_table->cdsFlags = 0;

    if (i < LoL->_nblkdev && (ULONG) dpb != 0xffffffffl)
    {
      if (!((1 << i) & bprm->DriveMask))
      {
        pcds_table->cdsDpb = dpb;
        pcds_table->cdsFlags = CDSPHYSDRV;
      }
      dpb = dpb->dpb_next;
    }
    pcds_table->cdsStrtClst = 0xffff;
    pcds_table->cdsParam = 0xffff;
    pcds_table->cdsStoreUData = 0xffff;
    pcds_table->cdsJoinOffset = 2;
  }

  /* Log-in the default drive. */
  init_setdrive(LoL->_BootDrive - 1);

  /* The system file tables need special handling and are "hand   */
  /* built. Included is the stdin, stdout, stdaux and stdprn. */
  /* a little bit of shuffling is necessary for compatibility */

  /* sft_idx=0 is /dev/aux                                        */
  safe_open("AUX", O_RDWR);

  /* handle 1, sft_idx=1 is /dev/con (stdout) */
  safe_open("CON", O_RDWR);

  /* 3 is /dev/aux                */
  dup2(STDIN, STDAUX);

  /* 0 is /dev/con (stdin)        */
  dup2(STDOUT, STDIN);

  /* 2 is /dev/con (stdin)        */
  dup2(STDOUT, STDERR);

  /* 4 is /dev/prn                                                */
  safe_open("PRN", O_WRONLY);

  /* Initialize the disk buffer management functions */
  /* init_call_init_buffers(); done from CONFIG.C   */
}

STATIC VOID signon()
{
#ifdef EXTRA_DEBUG
  /* erase debug "123" msg */
  _printf("\r");
#endif
  _printf("%s\n"
         "Kernel compatibility %d.%d - "
#if defined(__BORLANDC__)
  "BORLANDC"
#elif defined(__TURBOC__)
  "TURBOC"
#elif defined(_MSC_VER)
  "MSC"
#elif defined(__WATCOMC__)
  "WATCOMC"
#elif defined(__GNUC__)
  "GNUC"
#else
#error Unknown compiler
  generate some bullshit error here, as the compiler should be known
#endif
#if defined (I386)
    " - 80386 CPU required"
#elif defined (I186)
    " - 80186 CPU required"
#endif

#ifdef WITHFAT32
  " - FAT32 support"
#endif
  "\n\n%s",
         GET_PTR((char FAR *)MK_FP(FP_SEG(LoL), FP_OFF(LoL->_os_release))),
         MAJOR_RELEASE, MINOR_RELEASE, copyright);
}

STATIC void kernel()
{
  CommandTail Cmd;

  /* process 0       */
  /* Execute command.com from the drive we just booted from    */
  memset(Cmd.ctBuffer, 0, sizeof(Cmd.ctBuffer));
  strcpy(Cmd.ctBuffer, Config.cfgInitTail);

  for (Cmd.ctCount = 0; Cmd.ctCount < sizeof(Cmd.ctBuffer); Cmd.ctCount++)
    if (Cmd.ctBuffer[Cmd.ctCount] == '\r')
      break;

  /* if stepping CONFIG.SYS (F5/F8), tell COMMAND.COM about it */

  /* 3 for string + 2 for "\r\n" */
  if (Cmd.ctCount < sizeof(Cmd.ctBuffer) - 5)
  {
    const char *insertString = NULL;

    if (singleStep)
      insertString = " /Y";     /* single step AUTOEXEC */

    if (SkipAllConfig)
      insertString = " /D";     /* disable AUTOEXEC */

    if (insertString)
    {

      /* insert /D, /Y as first argument */
      char *p, *q;

      for (p = Cmd.ctBuffer; p < &Cmd.ctBuffer[Cmd.ctCount]; p++)
      {
        if (*p == ' ' || *p == '\t' || *p == '\r')
        {
          for (q = &Cmd.ctBuffer[Cmd.ctCount + 1]; q >= p; q--)
            q[3] = q[0];
          memcpy(p, insertString, 3);
          break;
        }
      }
      /* save buffer -- on the stack it's fine here */
      MK_NEAR_STR_OBJ(Config, cfgInitTail, Cmd.ctBuffer);
    }
  }
#ifdef FDPP_DEBUG
  dosobj_dump();
#endif
  call_p_0(MK_FAR_SCP(Config)); /* go execute process 0 (the shell) */
}

/* check for a block device and update  device control block    */
STATIC VOID update_dcb(struct dhdr FAR * dhp)
{
  REG COUNT Index;
  COUNT nunits = dhp->dh_name[0];
  struct dpb FAR *dpb;

  if (LoL->_nblkdev == 0)
    dpb = LoL->_DPBp;
  else
  {
    for (dpb = LoL->_DPBp; (ULONG) dpb->dpb_next != 0xffffffffl;
         dpb = dpb->dpb_next)
      ;
    struct dpb FAR *_dpb =
      KernelAlloc(nunits * sizeof(struct dpb), 'E', Config.cfgDosDataUmb);
    dpb->dpb_next = _dpb;
    dpb = dpb->dpb_next;
  }

  for (Index = 0; Index < nunits; Index++)
  {
    dpb->dpb_next = dpb + 1;
    dpb->dpb_unit = LoL->_nblkdev;
    dpb->dpb_subunit = Index;
    dpb->dpb_device = dhp;
    dpb->dpb_flags = M_CHANGED;
    if ((LoL->_CDSp != NULL) && (LoL->_nblkdev < LoL->_lastdrive))
    {
      LoL->_CDSp[LoL->_nblkdev].cdsDpb = dpb;
      LoL->_CDSp[LoL->_nblkdev].cdsFlags = CDSPHYSDRV;
    }
    ++dpb;
    ++LoL->_nblkdev;
  }
  (dpb - 1)->dpb_next = (void FAR *)0xFFFFFFFFl;
}

/* If cmdLine is NULL, this is an internal driver */

BOOL init_device(struct dhdr FAR * dhp, char *cmdLine, COUNT mode,
                 char FAR **r_top)
{
  request rq;
  char name[8];

  if (cmdLine) {
    char *p, *q, ch;
    int i;

    p = q = cmdLine;
    for (;;)
    {
      ch = *p;
      if (ch == '\0' || ch == ' ' || ch == '\t')
        break;
      p++;
      if (ch == '\\' || ch == '/' || ch == ':')
        q = p; /* remember position after path */
    }
    for (i = 0; i < 8; i++) {
      ch = '\0';
      if (p != q && *q != '.')
        ch = *q++;
      /* copy name, without extension */
      name[i] = ch;
    }
  }

  rq.r_unit = 0;
  rq.r_status = 0;
  rq.r_command = C_INIT;
  rq.r_length = sizeof(request);
  rq.r_endaddr = *r_top;
  MK_FAR_STR_OBJ(rq, r_cmdline, cmdLine ? cmdLine : "\n");
  rq.r_firstunit = LoL->_nblkdev;

  execrh(MK_FAR_SCP(rq), dhp);

/*
 *  Added needed Error handle
 */
  if ((rq.r_status & (S_ERROR | S_DONE)) != S_DONE)
    return TRUE;

  if (cmdLine)
  {
    /* Don't link in device drivers which do not take up memory */
    if (rq.r_endaddr == (BYTE FAR *) dhp)
      return TRUE;

    /* Don't link in block device drivers which indicate no units */
    if (!(dhp->dh_attr & ATTR_CHAR) && !rq.r_nunits)
    {
      rq.r_endaddr = (BYTE FAR *) dhp;
      return TRUE;
    }


    /* Fix for multisegmented device drivers:                          */
    /*   If there are multiple device drivers in a single driver file, */
    /*   only the END ADDRESS returned by the last INIT call should be */
    /*   the used.  It is recommended that all the device drivers in   */
    /*   the file return the same address                              */

    if (FP_OFF(dhp->dh_next) == 0xffff)
    {
      KernelAllocPara(FP_SEG(rq.r_endaddr) + (FP_OFF(rq.r_endaddr) + 15)/16
                      - FP_SEG(dhp), 'D', name, mode);
    }

    /* Another fix for multisegmented device drivers:                  */
    /*   To help emulate the functionallity experienced with other DOS */
    /*   operating systems when calling multiple device drivers in a   */
    /*   single driver file, save the end address returned from the    */
    /*   last INIT call which will then be passed as the end address   */
    /*   for the next INIT call.                                       */

    *r_top = (char FAR *)rq.r_endaddr;
  }

  if (!(dhp->dh_attr & ATTR_CHAR) && (rq.r_nunits != 0))
  {
    dhp->dh_name[0] = rq.r_nunits;
    update_dcb(dhp);
  }

  if (dhp->dh_attr & ATTR_CONIN)
    LoL->_syscon = dhp;
  else if (dhp->dh_attr & ATTR_CLOCK)
    LoL->_clock = dhp;

  return FALSE;
}

STATIC void InitIO(void)
{
  struct dhdr FAR *device = &LoL->_nul_dev;

  /* Initialize driver chain                                      */
  do {
    init_device(device, NULL, 0, &lpTop);
    device = device->dh_next;
  }
  while (FP_OFF(device) != 0xffff);
}

/* issue an internal error message                              */
VOID init_fatal(const BYTE * err_msg)
{
  char buf[256];
  _snprintf(buf, sizeof(buf), "\nInternal kernel error - %s\nSystem halted\n", err_msg);
  panic(buf);
}

/*
       Initialize all printers

       this should work. IMHO, this might also be done on first use
       of printer, as I never liked the noise by a resetting printer, and
       I usually much more often reset my system, then I print :-)
 */

STATIC VOID InitPrinters(VOID)
{
  iregs r = {};
  int num_printers, i;

  init_call_intr(0x11, &r);     /* get equipment list */

  num_printers = (r.a.x >> 14) & 3;     /* bits 15-14 */

  for (i = 0; i < num_printers; i++)
  {
    r.a.x = 0x0100;             /* initialize printer */
    r.d.x = i;
    init_call_intr(0x17, &r);
  }
}

STATIC VOID InitSerialPorts(VOID)
{
  iregs r = {};
  int serial_ports, i;

  init_call_intr(0x11, &r);     /* get equipment list */

  serial_ports = (r.a.x >> 9) & 7;      /* bits 11-9 */

  for (i = 0; i < serial_ports; i++)
  {
    r.a.x = 0xA3;               /* initialize serial port to 2400,n,8,1 */
    r.d.x = i;
    init_call_intr(0x14, &r);
  }
}

#ifndef FDPP
/*****************************************************************
        if kernel.config.BootHarddiskSeconds is set,
        the default is to boot from harddisk, because
        the user is assumed to just have forgotten to
        remove the floppy/bootable CD from the drive.

        user has some seconds to hit ANY key to continue
        to boot from floppy/cd, else the system is
        booted from HD
*/
STATIC int EmulatedDriveStatus(int drive,char statusOnly)
{
  iregs r = {};
  UBYTE buffer[0x13];
  UBYTE FAR *buf;
  buffer[0] = 0x13;

  r.a.b.h = 0x4b;               /* bootable CDROM - get status */
  r.a.b.l = statusOnly;
  r.d.b.l = (char)drive;
  buf = MK_FAR(buffer);
  r.ds = FP_SEG(buf);
  r.si = FP_OFF(buf);
  init_call_intr(0x13, &r);

  if (r.flags & 1)
        return FALSE;

  return TRUE;
}

STATIC void CheckContinueBootFromHarddisk(void)
{
  const char *bootedFrom = "Floppy/CD";
  iregs r = {};
  int key;

  if (InitKernelConfig.BootHarddiskSeconds == 0)
    return;

  if (LoL->_BootDrive >= 3)
  {
#if 0
    if (!EmulatedDriveStatus(0x80,1))
#endif
    {
      /* already booted from HD */
      return;
    }
  }
  else {
#if 0
    if (!EmulatedDriveStatus(0x00,1))
#endif
      bootedFrom = "Floppy";
  }

  _printf("\n"
         "\n"
         "\n"
         "     Hit any key within %d seconds to continue boot from %s\n"
         "     Hit 'H' or    wait %d seconds to boot from Harddisk\n",
         InitKernelConfig.BootHarddiskSeconds,
         bootedFrom,
         InitKernelConfig.BootHarddiskSeconds
    );

  key = GetBiosKey(InitKernelConfig.BootHarddiskSeconds);

  if (key != -1 && (key & 0xff) != 'h' && (key & 0xff) != 'H')
  {
    /* user has hit a key, continue to boot from floppy/CD */
    _printf("\n");
    return;
  }

  /* reboot from harddisk */
  EmulatedDriveStatus(0x00,0);
  EmulatedDriveStatus(0x80,0);

  /* now jump and run */
  r.a.x = 0x0201;
  r.c.x = 0x0001;
  r.d.x = 0x0080;
  r.b.x = 0x7c00;
  r.es  = 0;

  init_call_intr(0x13, &r);

  {
    __ASMCALL(reboot) = MK_FP(0x0,0x7c00);

     (*reboot)();
  }
}
#endif
