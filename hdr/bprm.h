/* Copyright stsp, FDPP project */

#ifndef BPRM_H
#define BPRM_H

struct _bprm {
  /* keep plt at the beginning to not add offsets in plt.S */
  unsigned short PltOff;
  unsigned short PltSeg;
  unsigned short InitEnvSeg;  /* initial env seg                      */
  unsigned char ShellDrive;   /* drive num to start shell from        */
  unsigned char DeviceDrive;  /* drive num to load DEVICE= from       */
  unsigned char CfgDrive;     /* drive num to load fdppconf.sys from  */
} PACKED;

#define BPRM_VER 2

#endif
