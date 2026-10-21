# Test for https://github.com/dosemu2/fdpp/issues/312
#
# A simplified variant of dosemu2/test/func_ds3_share_open_twice.py: it
# opens a file exclusively (write, deny all), then attempts to open it
# again with conflicting sharing modes. For every refused second open it
# prints both the normal (DOS 2 style) error code returned in AX by
# INT 21h/AH=3Dh and AH=6Ch, and the extended error fields reported by
# INT 21h/AH=59h/BX=0000h.
#
# The purpose is to compare how the usual DOS variants report a sharing
# conflict refused by the internal SHARE / redirector: MS-DOS returns
# AX=05h (access denied) from the open call itself and keeps 20h (sharing
# violation) only as the extended error visible through 59h.

import re


def _run_all(self, fstype):
    testdir = self.mkworkdir('d')

    share = self.share_command(fstype)
    name = 'shrerr59'

    # primary opens FOO.DAT exclusively (write, deny all), then spawns
    # itself as secondary which probes with several conflicting modes.
    # Note SH_COMPAT probes are deliberately absent: compatibility-mode
    # opens raise INT 24h on a sharing conflict instead of returning an
    # error code, which would change what 59h reports.
    self.mkfile("testit.bat", f"""\
echo off
set LFN=n
d:
{share}
c:\\{name} primary SH_DENYRW W SH_DENYNO RW DENY
c:\\{name} primary SH_DENYRW W SH_DENYRW R DENY
c:\\{name} primary SH_DENYRW W SH_DENYRD RW DENY
c:\\{name} primary SH_DENYWR W SH_DENYWR W DENY
rem tests complete
rem end
""", newline="\r\n")

    # compile sources
    self.mkexe_with_djgpp(name, r"""
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <share.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define FNAME "FOO.DAT"

/* open flags are in the low byte, sharing mode in bits 8-11 */
#define OPENMODE(omode, smode) ((unsigned short)(omode) | \
                                ((unsigned short)(smode) << 8))

static unsigned short sharemode(const char *s) {
  if (strcmp(s, "SH_COMPAT") == 0)
    return SH_COMPAT;
  if (strcmp(s, "SH_DENYRW") == 0)
    return SH_DENYRW;
  if (strcmp(s, "SH_DENYWR") == 0)
    return SH_DENYWR;
  if (strcmp(s, "SH_DENYRD") == 0)
    return SH_DENYRD;
  if (strcmp(s, "SH_DENYNO") == 0)
    return SH_DENYNO;
  return 0xff;
}

static unsigned short openmode(const char *s) {
  if (strcmp(s, "RW") == 0)
    return O_RDWR;
  if (s[0] == 'R')
    return O_RDONLY;
  if (s[0] == 'W')
    return O_WRONLY;
  return 0xff;
}

struct ext_err {
  uint16_t errcode;      /* extended error code */
  uint8_t  sclass;       /* suggested cause class */
  uint8_t  action;       /* suggested action */
  uint16_t locus;        /* error locus */
  uint8_t  slsrc;        /* suggested source */
  uint8_t  reserved;
};

/* INT 21h/AH=59h,BX=0000h - get extended error information.
   ES must point to the DOS global data area on entry; DJGPP gives us
   the previous value of ES back in the SREGS. */
static void get_ext_err(struct ext_err *ee) {
  union REGS r;
  struct SREGS s;

  memset(&r, 0, sizeof(r));
  memset(&s, 0, sizeof(s));
  r.x.ax = 0x5900;
  r.x.bx = 0x0000;
  int86x(0x21, &r, &r, &s);
  if (r.x.cflag) {
    /* 59h unsupported/refused: mark everything as bogus */
    ee->errcode = 0xffff;
    ee->sclass = 0xff;
    ee->action = 0xff;
    ee->locus = 0xffff;
    ee->slsrc = 0xff;
    return;
  }
  /* ES:BX -> extended error record */
  ee->errcode = peekw(s.es, r.x.bx + offsetof(struct ext_err, errcode));
  ee->sclass  = peekb(s.es, r.x.bx + offsetof(struct ext_err, sclass));
  ee->action  = peekb(s.es, r.x.bx + offsetof(struct ext_err, action));
  ee->locus   = peekw(s.es, r.x.bx + offsetof(struct ext_err, locus));
  ee->slsrc   = peekb(s.es, r.x.bx + offsetof(struct ext_err, slsrc));
}

/* INT 21h/AH=6Ch, AL=02h (open existing file). Returns the AX error
   code on failure, or 0 when the open unexpectedly succeeded (the
   handle is closed again in that case). */
static unsigned short open6c(unsigned short shmode, unsigned short omode) {
  union REGS r;
  struct SREGS s;
  static char fname[] = FNAME;

  memset(&r, 0, sizeof(r));
  memset(&s, 0, sizeof(s));
  r.h.ah = 0x6c;
  r.h.al = 0x02;                       /* open existing only */
  r.x.cx = 0;                          /* normal attributes */
  r.x.dx = 0;                          /* no additional actions */
  r.x.si = FP_OFF(fname);              /* DS:SI -> ASCZ pathname */
  s.ds = FP_SEG(fname);
  /* BH = sharing mode, BL low 2 bits = access mode */
  r.x.bx = (shmode << 8) | (omode & 3);
  int86x(0x21, &r, &r, &s);
  if (!r.x.cflag) {
    _close(r.x.ax);
    return 0;
  }
  return (unsigned short)r.x.ax;
}

int main(int argc, char *argv[]) {
  int primary = -1;
  unsigned short prismode, priomode, secsmode, secomode;
  struct ext_err ee;

  if (argc < 7) {
    printf("FAIL: Missing arguments (primary|secondary) prismode priomode "
           "secsmode secomode expected\n");
    return -2;
  }
  if (strcmp(argv[1], "primary") == 0)
    primary = 1;
  if (strcmp(argv[1], "secondary") == 0)
    primary = 0;
  if (primary < 0) {
    printf("FAIL: Invalid argument (primary|secondary)\n");
    return -2;
  }

  prismode = sharemode(argv[2]);
  priomode = openmode(argv[3]);
  secsmode = sharemode(argv[4]);
  secomode = openmode(argv[5]);
  if (prismode == 0xff || priomode == 0xff ||
      secsmode == 0xff || secomode == 0xff) {
    printf("FAIL: Invalid mode arguments\n");
    return -2;
  }

  if (primary) {
    int handle;
    int ret = _dos_open(FNAME, OPENMODE(priomode, prismode), &handle);
    if (ret != 0) {
      printf("FAIL: primary could not open '%s' (error %d)\n", FNAME, ret);
      return -1;
    }

    /* Now start second copy which attempts the conflicting opens */
    spawnlp(P_WAIT, argv[0], argv[0], "secondary", argv[2], argv[3],
            argv[4], argv[5], argv[6], NULL);

    _dos_close(handle);
  } else {
    int handle;
    unsigned short ax3d, ax6c;
    int ret;

    /* probe with 3Dh first, print its error and the 59h info... */
    ret = _dos_open(FNAME, OPENMODE(secomode, secsmode), &handle);
    ax3d = (ret == 0) ? 0 : (unsigned short)ret;
    if (ret == 0)
      _dos_close(handle);
    get_ext_err(&ee);
    printf("INFO:('%s', '%s', '%s', '%s') 3Dh AX=%04Xh 59h: code=%04Xh "
           "class=%02Xh act=%02Xh locus=%04Xh src=%02Xh%s\n",
           argv[4], argv[5], argv[2], argv[3], ax3d,
           ee.errcode, ee.sclass, ee.action, ee.locus, ee.slsrc,
           ax3d == 0 ? " ALLOWED" : "");

    /* ...then repeat the same probe with 6Ch */
    ax6c = open6c(secsmode, secomode);
    get_ext_err(&ee);
    printf("INFO:('%s', '%s', '%s', '%s') 6Ch AX=%04Xh 59h: code=%04Xh "
           "class=%02Xh act=%02Xh locus=%04Xh src=%02Xh%s\n",
           argv[4], argv[5], argv[2], argv[3], ax6c,
           ee.errcode, ee.sclass, ee.action, ee.locus, ee.slsrc,
           ax6c == 0 ? " ALLOWED" : "");
  }

  return 0;
}
""")

    self.mkfile("FOO.DAT", "some data", dname=testdir)

    config = """$_floppy_a = ""\n"""

    if fstype == "MFS":
        config += """$_hdimage = "dXXXXs/c:hdtype1 dXXXXs/d:hdtype1 +1"\n"""
    else:       # FAT
        name = self.mkimage("12", cwd=testdir)
        config += """$_hdimage = "dXXXXs/c:hdtype1 %s +1"\n""" % name

    return self.runDosemu("testit.bat", config=config, timeout=120)


def ds3_share_open_err59(self, fstype):
    results = _run_all(self, fstype)
    self.assertIn("rem tests complete", results)
    self.assertNotIn("FAIL:", results)
    # print the collected error codes so the CI logs show them for every
    # DOS variant the test matrix runs against
    for line in results.splitlines():
        if line.startswith("INFO:"):
            print(line)
