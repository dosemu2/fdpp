/* Included by initialisation functions */
#define IN_INIT_MOD

#if 0
#include "version.h"
#include "date.h"
#include "time.h"
#include "mcb.h"
#include "sft.h"
#include "fat.h"
#include "file.h"
#include "cds.h"
#include "device.h"
#include "kbd.h"
#include "error.h"
#include "fcb.h"
#include "tail.h"
#include "process.h"
#include "pcb.h"
#include "buffer.h"
#include "dcb.h"
#endif
#include "lol.h"
#include "nls.h"

#include "init-dat.h"

#include "kconfig.h"

/* MSC places uninitialized data into COMDEF records,
   that end up in DATA segment. this can't be tolerated in INIT code.
   please make sure, that ALL data in INIT is initialized !!

   These guys are marked BSS_INIT to mark that they really should be BSS
   but can't be because of MS
*/
#ifdef _MSC_VER
#define BSS_INIT(x) = x
#else
#define BSS_INIT(x)
#endif

extern struct _KernelConfig InitKernelConfig;

#if 0
/*
 * Functions in `INIT_TEXT' may need to call functions in `_TEXT'. The entry
 * calls for the latter functions therefore need to be wrapped up with far
 * entry points.
 */
#define printf      init_printf
#define sprintf     init_sprintf
#ifndef __WATCOMC__
#define execrh      init_execrh
#define  memcpy     init_memcpy
#define fmemcpy     init_fmemcpy
#define fmemset     init_fmemset
#define fmemcmp     init_fmemcmp
#define  memcmp     init_memcmp
#define  memset     init_memset
#define  strchr     init_strchr
#define  strcpy     init_strcpy
#define  strlen     init_strlen
#define fstrlen     init_fstrlen
#endif
#endif
#define open        init_DosOpen

#undef LINESIZE
#define LINESIZE KBD_MAXLENGTH

/*inithma.c*/
extern BYTE DosLoadedInHMA;
void MoveKernel(UWORD NewKernelSegment);

void setvec(unsigned char intno, intvec vector);
#ifndef __WATCOMC__
//#define getvec init_getvec
#endif
intvec getvec(unsigned char intno);

#define GLOBAL extern
#define NAMEMAX         MAX_CDSPATH     /* Maximum path for CDS         */
#define NFILES          16      /* number of files in table     */
#define NFCBS           16      /* number of fcbs               */
#define NSTACKS         8       /* number of stacks             */
#define STACKSIZE       256     /* default stacksize            */
#define NLAST           5       /* last drive                   */
#define NUMBUFF         20      /* Number of track buffers at INIT time     */
                                        /* -- must be at least 3        */
#define MAX_HARD_DRIVE  8
#define NDEV            26      /* up to Z:                     */

#include "config.h" /* config structure */

/* config.c */
extern struct config Config;
VOID PreConfig(VOID);
VOID PreConfig2(VOID);
VOID DoConfig(int pass);
VOID PostConfig(VOID);
VOID configDone(VOID);
__FAR(VOID) KernelAlloc(size_t nBytes, char type, int mode);
__FAR(void) KernelAllocPara(size_t nPara, char type, char *name, int mode);
//char *strcat(char * d, const char * s);
BYTE * GetStringArg(BYTE * pLine, BYTE * pszString);
void DoInstall(void);
UWORD GetBiosKey(int timeout);

/* diskinit.c */
COUNT dsk_init(VOID);

/* inithma.c */
int MoveKernelToHMA(void);
__FAR(VOID) HMAalloc(COUNT bytesToAllocate);

/* initoem.c */
unsigned init_oem(void);
void movebda(size_t bytes, UWORD new_seg);
unsigned ebdasize(void);

/* main.c */
BOOL init_device(__FAR(struct dhdr) dhp, char * cmdLine,
                      COUNT mode, __FAR(char)*top);
VOID init_fatal(BYTE * err_msg);

#if 0
/* prf.c */
int VA_CDECL init_printf(CONST char * fmt, ...);
int VA_CDECL init_sprintf(char * buff, CONST char * fmt, ...);
#endif

/* initclk.c */
extern void Init_clk_driver(void);

extern UWORD HMAFree;            /* first byte in HMA not yet used      */

extern UWORD CurrentKernelSegment;
extern WORD days[2][13];
extern __FAR(BYTE)lpTop;
extern UWORD ram_top;               /* How much ram in Kbytes               */
extern char singleStep;
extern char SkipAllConfig;
extern char master_env[128];

extern ASMREF(struct lol) LoL;

struct _nlsCountryInfoHardcoded {
  char  ThisIsAConstantOne;
  short TableSize;

  struct CountrySpecificInfo C;
};

/*
    data shared between DSK.C and INITDISK.C
*/

extern UWORD DOSFAR LBA_WRITE_VERIFY;

struct RelocationTable {
  UBYTE jmpFar;
  UWORD jmpOffset;
  UWORD jmpSegment;
  UBYTE callNear;
  UWORD callOffset;
} PACKED;

struct RelocatedEntry {
  UBYTE callNear;
  UWORD callOffset;
  UBYTE jmpFar;
  UWORD jmpOffset;
  UWORD jmpSegment;
} PACKED;

#if defined(WATCOM) && 0
ULONG ASMCFUNC FAR MULULUS(ULONG mul1, UWORD mul2);     /* MULtiply ULong by UShort */
ULONG ASMCFUNC FAR MULULUL(ULONG mul1, ULONG mul2);     /* MULtiply ULong by ULong */
ULONG ASMCFUNC FAR DIVULUS(ULONG mul1, UWORD mul2);     /* DIVide ULong by UShort */
ULONG ASMCFUNC FAR DIVMODULUS(ULONG mul1, UWORD mul2, UWORD * rem);     /* DIVide ULong by UShort */
#endif

