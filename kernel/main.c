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
/* write to the Free Software Foundation, 675 Mass Ave,         */
/* Cambridge, MA 02139, USA.                                    */
/****************************************************************/

#include "portab.h"
#include "init-mod.h"
#include "dyndata.h"
#include "init-dat.h"

char copyright[] =
    "(C) Copyright 1995-2002 Pasquale J. Villani and The FreeDOS Project.\n"
    "All Rights Reserved. This is free software and comes with ABSOLUTELY NO\n"
    "WARRANTY; you can redistribute it and/or modify it under the terms of the\n"
    "GNU General Public License as published by the Free Software Foundation;\n"
    "either version 2, or (at your option) any later version.\n";

/*
  These are the far variables from the DOS data segment that we need here. The
  init procedure uses a different default DS data segment, which is discarded
  after use. I hope to clean this up to use the DOS List of List and Swappable
  Data Area obtained via INT21.

  -- Bart
 */
extern UBYTE DOSFAR ASM nblkdev, DOSFAR ASM lastdrive;  /* value of last drive                  */

GLOBAL BYTE DOSFAR os_major,    /* major version number                 */
  DOSFAR os_minor,              /* minor version number                 */
  DOSFAR ASM default_drive;         /* default drive for dos                */
GLOBAL UBYTE DOSFAR ASM BootDrive;  /* Drive we came up from                */

GLOBAL BYTE DOSFAR os_release[];

extern struct dpb FAR *DOSFAR ASM DPBp;     /* First drive Parameter Block          */
extern struct cds FAR *DOSFAR ASM CDSp; /* Current Directory Structure          */

extern struct dhdr FAR *DOSFAR ASM clock,   /* CLOCK$ device                        */
  FAR * DOSFAR ASM syscon;          /* console device                       */
extern struct dhdr ASM DOSTEXTFAR con_dev,  /* console device drive                 */
  DOSTEXTFAR ASM clk_dev,           /* Clock device driver                  */
  DOSTEXTFAR ASM blk_dev;           /* Block device (Disk) driver           */
extern iregs FAR *DOSFAR ASM user_r;        /* User registers for int 21h call      */
extern BYTE FAR ASM _HMATextEnd[];

extern struct _KernelConfig FAR ASM LowKernelConfig;

#ifdef VERSION_STRINGS
static BYTE *mainRcsId =
    "$Id$";
#endif

struct _KernelConfig InitKernelConfig = { "", 0, 0, 0, 0, 0, 0 };

extern WORD days[2][13];
extern BYTE FAR *lpBase;
extern BYTE FAR *lpOldTop;
extern BYTE FAR *lpTop;
extern BYTE FAR *upBase;
extern BYTE ASM _ib_start[], ASM _ib_end[], ASM _init_end[];
extern UWORD ram_top;               /* How much ram in Kbytes               */

STATIC VOID InitIO(void);

STATIC VOID update_dcb(struct dhdr FAR *);
STATIC VOID init_kernel(VOID);
STATIC VOID signon(VOID);
STATIC VOID kernel(VOID);
STATIC VOID FsConfig(VOID);
STATIC VOID InitPrinters(VOID);

extern void Init_clk_driver(void);

#ifdef _MSC_VER
BYTE _acrtused = 0;
#endif

#ifdef _MSC_VER
__segment DosDataSeg = 0;       /* serves for all references to the DOS DATA segment 
                                   necessary for MSC+our funny linking model
                                 */
__segment DosTextSeg = 0;

#endif

/* little functions - could be ASM but does not really matter in this context */
void memset(void *s, int c, unsigned n)
{
  char *t = s;	
  while(n--) *t++ = c;
}

void fmemset(void far *s, int c, unsigned n)
{
  char far *t = s;
  while(n--) *t++ = c;
}

void strcpy(char *dest, const char *src)
{
  while(*src)
    *dest++ = *src++;
  *dest = '\0';
}

void fmemcpy(void far *dest, const void far *src, unsigned n)
{
  char far *d = dest;
  const char far *s = src;
  while(n--) *d++ = *s++;
}

VOID ASMCFUNC FreeDOSmain(void)
{
#ifdef _MSC_VER
  extern FAR DATASTART;
  extern FAR prn_dev;
  DosDataSeg = (__segment) & DATASTART;
  DosTextSeg = (__segment) & prn_dev;
#endif

                        
                        /*  if the kernel has been UPX'ed,
                                CONFIG info is stored at 50:e2 ..fc
                            and the bootdrive (passed from BIOS)
                            at 50:e0
                        */    
                        
  if (fmemcmp(MK_FP(0x50,0xe0+2),"CONFIG",6) == 0)      /* UPX */
  {
    fmemcpy(&InitKernelConfig, MK_FP(0x50,0xe0+2), sizeof(InitKernelConfig));
    
    BootDrive = *(BYTE FAR *)MK_FP(0x50,0xe0) + 1;
        
    if (BootDrive >= 0x80)
      BootDrive = 3; /* C: */
    
    *(DWORD FAR *)MK_FP(0x50,0xe0+2) = 0; 
  } 
  else
  {
    fmemcpy(&InitKernelConfig, &LowKernelConfig, sizeof(InitKernelConfig));
  }
  
  setvec(0, int0_handler);      /* zero divide */
  setvec(1, empty_handler);     /* single step */
  setvec(3, empty_handler);     /* debug breakpoint */
  setvec(6, int6_handler);      /* invalid opcode */

  /* clear the Init BSS area (what normally the RTL does */
  memset(_ib_start, 0, _ib_end - _ib_start);

  signon();
  init_kernel();

#ifdef DEBUG
  /* Non-portable message kludge alert!   */
  printf("KERNEL: Boot drive = %c\n", 'A' + BootDrive - 1);
#endif

  DoInstall();

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
void InitializeAllBPBs(VOID)
{
  static char filename[] = "A:-@JUNK@-.TMP";
  int drive, fileno;
  for (drive = 'C'; drive < 'A' + nblkdev; drive++)
  {
    filename[0] = drive;
    if ((fileno = open(filename, O_RDONLY)) >= 0)
      close(fileno);
  }
}

STATIC void init_kernel(void)
{
  COUNT i;

  os_major = MAJOR_RELEASE;
  os_minor = MINOR_RELEASE;

  /* Init oem hook - returns memory size in KB    */
  ram_top = init_oem();

  /* move kernel to high conventional RAM, just below the init code */
  lpTop = MK_FP(ram_top * 64 - (FP_OFF(_init_end) + 15) / 16 -
                (FP_OFF(_HMATextEnd) + 15) / 16, 0);

  MoveKernel(FP_SEG(lpTop));
  lpOldTop = lpTop = MK_FP(FP_SEG(lpTop) - 0xfff, 0xfff0);

/* Fake int 21h stack frame */
  user_r = (iregs FAR *) MK_FP(DOS_PSP, 0xD0);

#ifndef KDB
  for (i = 0x20; i <= 0x3f; i++)
    setvec(i, empty_handler);
#endif

  /* Initialize IO subsystem                                      */
  InitIO();
  InitPrinters();

#ifndef KDB
  /* set interrupt vectors                                        */
  setvec(0x1b, got_cbreak);
  setvec(0x20, int20_handler);
  setvec(0x21, int21_handler);
  setvec(0x22, int22_handler);
  setvec(0x23, empty_handler);
  setvec(0x24, int24_handler);
  setvec(0x25, low_int25_handler);
  setvec(0x26, low_int26_handler);
  setvec(0x27, int27_handler);
  setvec(0x28, int28_handler);
  setvec(0x2a, int2a_handler);
  setvec(0x2f, int2f_handler);
#endif

  init_PSPSet(DOS_PSP);
  init_PSPInit(DOS_PSP);

  Init_clk_driver();

  /* Do first initialization of system variable buffers so that   */
  /* we can read config.sys later.  */
  lastdrive = Config.cfgLastdrive;

  /*  init_device((struct dhdr FAR *)&blk_dev, NULL, NULL, ram_top); */
  blk_dev.dh_name[0] = dsk_init();

  PreConfig();

  /* Number of units */
  if (blk_dev.dh_name[0] > 0)
    update_dcb(&blk_dev);

  /* Now config the temporary file system */
  FsConfig();

#ifndef KDB
  /* Now process CONFIG.SYS     */
  DoConfig(0);
  DoConfig(1);

  /* Close all (device) files */
  for (i = 0; i < lastdrive; i++)
    close(i);

  /* and do final buffer allocation. */
  PostConfig();
  nblkdev = 0;
  update_dcb(&blk_dev);

  /* Init the file system one more time     */
  FsConfig();

  /* and process CONFIG.SYS one last time to load device drivers. */
  DoConfig(2);
  configDone();

  /* Close all (device) files */
  for (i = 0; i < lastdrive; i++)
    close(i);

  /* Now config the final file system     */
  FsConfig();

#endif
  InitializeAllBPBs();
}

STATIC VOID FsConfig(VOID)
{
  REG COUNT i;
  struct dpb FAR *dpb;

  /* Log-in the default drive.  */
  /* Get the boot drive from the ipl and use it for default.  */
  default_drive = BootDrive - 1;
  dpb = DPBp;

  /* Initialize the current directory structures    */
  for (i = 0; i < lastdrive; i++)
  {
    struct cds FAR *pcds_table = &CDSp[i];

    fmemcpy(pcds_table->cdsCurrentPath, "A:\\\0", 4);

    pcds_table->cdsCurrentPath[0] += i;

    if (i < nblkdev && (ULONG) dpb != 0xffffffffl)
    {
      pcds_table->cdsDpb = dpb;
      pcds_table->cdsFlags = CDSPHYSDRV;
      dpb = dpb->dpb_next;
    }
    else
    {
      pcds_table->cdsFlags = 0;
    }
    pcds_table->cdsStrtClst = 0xffff;
    pcds_table->cdsParam = 0xffff;
    pcds_table->cdsStoreUData = 0xffff;
    pcds_table->cdsJoinOffset = 2;
  }

  /* The system file tables need special handling and are "hand   */
  /* built. Included is the stdin, stdout, stdaux and stdprn. */
  /* a little bit of shuffling is necessary for compatibility */

  /* sft_idx=0 is /dev/aux                                        */
  open("AUX", O_RDWR);

  /* handle 1, sft_idx=1 is /dev/con (stdout) */
  open("CON", O_RDWR);

  /* 3 is /dev/aux                */
  dup2(STDIN, STDAUX);

  /* 0 is /dev/con (stdin)        */
  dup2(STDOUT, STDIN);

  /* 2 is /dev/con (stdin)        */
  dup2(STDOUT, STDERR);

  /* 4 is /dev/prn                                                */
  open("PRN", O_WRONLY);

  /* Initialize the disk buffer management functions */
  /* init_call_init_buffers(); done from CONFIG.C   */
}

STATIC VOID signon()
{
  printf("\r%S", (void FAR *)os_release);

  printf("Kernel compatibility %d.%d", MAJOR_RELEASE, MINOR_RELEASE);

#if defined(__TURBOC__)
  printf(" - TURBOC");
#elif defined(_MSC_VER)
  printf(" - MSC");
#elif defined(__WATCOMC__)
  printf(" - WATCOMC");
#elif defined(__GNUC__)
  printf(" - GNUC"); /* this is hypothetical only */
#else
  generate some bullshit error here, as the compiler should be known
#endif
#if defined (I386)
    printf(" - 80386 CPU required");
#elif defined (I186)
    printf(" - 80186 CPU required");
#endif

#ifdef WITHFAT32
  printf(" - FAT32 support");
#endif
  printf("\n\n%S", (void FAR *)copyright);
}

STATIC void kernel()
{
  exec_blk exb;
  CommandTail Cmd;
  int rc;

  extern char MenuSelected;
  extern unsigned Menus;

  BYTE master_env[32];
  char *masterenv_ptr = master_env;

  /* build the startup environment */

  memset(master_env,0,sizeof(master_env));

  /* initial path setting. is this useful ?? */
  masterenv_ptr += sprintf(masterenv_ptr, "PATH=.");

  /* export the current selected config  menu */
  if (Menus)
    {
    masterenv_ptr += sprintf(masterenv_ptr, "CONFIG=%c", MenuSelected+'0');
    }

  exb.exec.env_seg = DOS_PSP + 8;
  fmemcpy(MK_FP(exb.exec.env_seg, 0), master_env, sizeof(master_env));

  /* process 0       */
  /* Execute command.com /P from the drive we just booted from    */
  memset(Cmd.ctBuffer, 0, sizeof(Cmd.ctBuffer));
  fmemcpy(Cmd.ctBuffer, Config.cfgInitTail, sizeof(Config.cfgInitTail));

  for (Cmd.ctCount = 0; Cmd.ctCount < sizeof(Cmd.ctBuffer); Cmd.ctCount++)
    if (Cmd.ctBuffer[Cmd.ctCount] == '\r')
      break;

  /* if stepping CONFIG.SYS (F5/F8), tell COMMAND.COM about it */

  if (Cmd.ctCount < sizeof(Cmd.ctBuffer) - 3)
  {
    extern int singleStep;
    extern int SkipAllConfig;
    char *insertString = NULL;

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
          for (q = &Cmd.ctBuffer[Cmd.ctCount - 1]; q >= p; q--)
            q[3] = q[0];

          fmemcpy(p, insertString, 3);

          Cmd.ctCount += 3;
          break;
        }
      }
    }
  }

  exb.exec.cmd_line = (CommandTail FAR *) & Cmd;
  exb.exec.fcb_1 = exb.exec.fcb_2 = (fcb FAR *) 0xfffffffful;
  
#ifdef DEBUG
  printf("Process 0 starting: %s\n\n", Config.cfgInit);
#endif

  while ((rc =
          init_DosExec(Config.cfgP_0_startmode, &exb,
                       Config.cfgInit)) != SUCCESS)
  {
    BYTE *pLine;
    printf("\nBad or missing Command Interpreter: %d - %s\n", rc,
           Cmd.ctBuffer);
    printf
        ("\nPlease enter the correct location (for example C:\\COMMAND.COM):\n");
    rc = read(STDIN, Cmd.ctBuffer, sizeof(Cmd.ctBuffer) - 1);
    Cmd.ctBuffer[rc] = '\0';

    /* Get the string argument that represents the new init pgm     */
    pLine = GetStringArg(Cmd.ctBuffer, Config.cfgInit);

    /* Now take whatever tail is left and add it on as a single     */
    /* string.                                                      */
    strcpy(Cmd.ctBuffer, pLine);

    /* and add a DOS new line just to be safe                       */
    strcat(Cmd.ctBuffer, "\r\n");

    Cmd.ctCount = rc - (pLine - Cmd.ctBuffer);

#ifdef DEBUG
    printf("Process 0 starting: %s\n\n", Config.cfgInit);
#endif
  }
  printf("\nSystem shutdown complete\nReboot now.\n");
  for (;;) ;
}

/* check for a block device and update  device control block    */
STATIC VOID update_dcb(struct dhdr FAR * dhp)
{
  REG COUNT Index;
  COUNT nunits = dhp->dh_name[0];
  struct dpb FAR *dpb;

  if (nblkdev == 0)
    dpb = DPBp;
  else
  {
    for (dpb = DPBp; (ULONG) dpb->dpb_next != 0xffffffffl;
         dpb = dpb->dpb_next)
      ;
    dpb = dpb->dpb_next =
        (struct dpb FAR *)KernelAlloc(nunits * sizeof(struct dpb));
  }

  for (Index = 0; Index < nunits; Index++)
  {
    dpb->dpb_next = dpb + 1;
    dpb->dpb_unit = nblkdev;
    dpb->dpb_subunit = Index;
    dpb->dpb_device = dhp;
    dpb->dpb_flags = M_CHANGED;
    if ((CDSp != 0) && (nblkdev < lastdrive))
    {
      CDSp[nblkdev].cdsDpb = dpb;
      CDSp[nblkdev].cdsFlags = CDSPHYSDRV;
    }
    ++dpb;
    ++nblkdev;
  }
  (dpb - 1)->dpb_next = (void FAR *)0xFFFFFFFFl;
}

/* If cmdLine is NULL, this is an internal driver */

BOOL init_device(struct dhdr FAR * dhp, BYTE FAR * cmdLine, COUNT mode,
                 char FAR *r_top)
{
  request rq;

  rq.r_unit = 0;
  rq.r_status = 0;
  rq.r_command = C_INIT;
  rq.r_length = sizeof(request);
  rq.r_endaddr = r_top;
  rq.r_bpbptr = (void FAR *)(cmdLine ? cmdLine : "\n");
  rq.r_firstunit = nblkdev;

  execrh((request FAR *) & rq, dhp);

/*
 *  Added needed Error handle
 */
  if (rq.r_status & S_ERROR)
    return TRUE;

  if (cmdLine)
  {
    if (mode)
    {
      /* Don't link in device drivers which do not take up memory */
      if (rq.r_endaddr == (BYTE FAR *) dhp)
        return TRUE;
      else
        upBase = rq.r_endaddr;
    }
    else
    {
      if (rq.r_endaddr == (BYTE FAR *) dhp)
        return TRUE;
      else
        lpBase = rq.r_endaddr;
    }
  }

  if (!(dhp->dh_attr & ATTR_CHAR) && (rq.r_nunits != 0))
  {
    dhp->dh_name[0] = rq.r_nunits;
    update_dcb(dhp);
  }

  if (dhp->dh_attr & ATTR_CONIN)
    syscon = dhp;
  else if (dhp->dh_attr & ATTR_CLOCK)
    clock = dhp;

  return FALSE;
}

STATIC void InitIO(void)
{
  /* Initialize driver chain                                      */
  setvec(0x29, int29_handler);  /* Requires Fast Con Driver     */
  init_device(&con_dev, NULL, NULL, lpTop);
  init_device(&clk_dev, NULL, NULL, lpTop);
}

/* issue an internal error message                              */
VOID init_fatal(BYTE * err_msg)
{
  printf("\nInternal kernel error - %s\nSystem halted\n", err_msg);
  for (;;) ;
}

/*
       Initialize all printers
 
       this should work. IMHO, this might also be done on first use
       of printer, as I never liked the noise by a resetting printer, and
       I usually much more often reset my system, then I print :-)
 */

STATIC VOID InitPrinters(VOID)
{
  iregs r;
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

