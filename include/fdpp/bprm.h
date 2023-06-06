/* Copyright stsp, FDPP project */

#ifndef BPRM_H
#define BPRM_H

#ifndef __ASSEMBLER__

#include <stdint.h>

struct _bprm {
  uint16_t BprmLen;
  uint16_t InitEnvSeg;          /* initial env seg                      */
  uint8_t ShellDrive;           /* drive num to start shell from        */
  uint8_t DeviceDrive;          /* drive num to load DEVICE= from       */
  uint8_t CfgDrive;             /* drive num to load fdppconf.sys from  */
  uint32_t DriveMask;           /* letters of masked drive are skipped  */
  uint16_t HeapSize;            /* heap for initial allocations         */
  uint16_t HeapSeg;             /* heap segment or 0 if after kernel    */
#define FDPP_FL_KERNEL_HIGH 1
#define FDPP_FL_HEAP_HIGH   2
#define FDPP_FL_HEAP_HMA    4
  uint16_t Flags;
  uint16_t BprmVersion;         /* same version as in VER_OFFSET        */
  uint16_t XtraSeg;             /* segment of second bprm part          */
  uint16_t PredMask;            /* predicates for ?x expression         */
};

struct _bprm_xtra {
  uint8_t BootDrvNum;
  char filler[15];
};

#endif

#define BPRM_VER 10
#define BPRM_MIN_VER 10

#define FDPP_BS_SEG 0x1fe0
#define FDPP_BS_OFF 0x7c00

#define FDPP_PLT_OFFSET 0x100
#define FDPP_BPRM_VER_OFFSET 0x110
#define FDPP_BPRM_OFFSET 0x112

#endif
