/* Copyright stsp, FDPP project */

#ifndef BPRM_H
#define BPRM_H

#include <stdint.h>

struct _bprm {
  /* keep plt at the beginning to not add offsets in plt.S (aka asm_offsets) */
  far_t Plt;
  uint16_t InitEnvSeg;          /* initial env seg                      */
  uint8_t ShellDrive;           /* drive num to start shell from        */
  uint8_t DeviceDrive;          /* drive num to load DEVICE= from       */
  uint8_t CfgDrive;             /* drive num to load fdppconf.sys from  */
  uint32_t DriveMask;           /* letters of masked drive are skipped  */
};

#define BPRM_VER 3

#endif
