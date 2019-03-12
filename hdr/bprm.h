/* Copyright stsp, FDPP project */

#ifndef BPRM_H
#define BPRM_H

struct _bprm {
  unsigned short InitEnvSeg;  /* initial env seg                      */
  unsigned char ShellDrive;   /* drive num to start shell from        */
  unsigned char DeviceDrive;  /* drive num to load DEVICE= from       */
  unsigned char CfgDrive;     /* drive num to load fdppconf.sys from  */
} PACKED;

extern struct _bprm *bprm;

#define BPRM_VER 1

#endif
