#
# Makefile for kernel.sys  (originally for Borland C/C++ 3.1)
#

COMPILER ?= gcc
XNASM ?= nasm
XCPU ?= 386
include ../mkfiles/generic.mak

LIBS=..$(DIRSEP)lib$(DIRSEP)device.lib
HDR=../hdr/

# *List Macros*
# Only 8 files per definition; this is limitation of DOS batch
# files (only 9 directly accessible parameters).

OBJS=kernel.obj entry.obj io.obj console.obj serial.obj printer.obj \
    asmsupt.obj execrh.obj nlssupt.obj procsupt.obj dosidle.obj int2f.obj \
    nls_hc.obj apisupt.obj intr.obj irqstack.obj iasmsupt.obj memdisk.obj \
    cpu.obj

#               *Explicit Rules*

production:     ../bin/libfdkernel.so ../bin/$(TARGET).sys ../bin/country.sys

../bin/$(TARGET).sys: kernel.sys
		$(CP) kernel.sys ..$(DIRSEP)bin
		$(CP) kernel.sys ..$(DIRSEP)bin$(DIRSEP)$(TARGET).sys
		$(CP) kernel.map ..$(DIRSEP)bin$(DIRSEP)$(TARGET).map

# -S to avoid showing expected relocations
# 0x10 & 0x78 or 0x79 depending on compilation options
kernel.sys:	kernel.exe ../utils/exeflat.exe
		..$(DIRSEP)utils$(DIRSEP)exeflat.exe kernel.exe kernel.sys $(LOADSEG) -S0x10 -S0x78 -S0x79 $(UPXOPT) $(XUPX)

kernel.exe:	$(TARGET).lnk $(OBJS) $(LIBS)
		$(LINK) @$(TARGET).lnk;

../bin/country.sys: country.asm
		$(NASM) -o $*.sys country.asm

clobber:        clean
		-$(RM) kernel.exe kernel.sys status.me

clean:
		-$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.cod *.err *.lnk *.o *.so

# XXX: This is a very ugly way of linking the kernel, forced upon us by the
# inability of Turbo `make' 2.0 to perform command line redirection. -- ror4

$(TARGET).lnk: turboc.cfg makefile ../mkfiles/generic.mak ../mkfiles/$(COMPILER).mak
		-$(RM) *.lnk
		$(ECHOTO) $(TARGET).lnk $(OBJS)
		$(ECHOTO) $(TARGET).lnk kernel.exe
		$(ECHOTO) $(TARGET).lnk kernel.map
		$(ECHOTO) $(TARGET).lnk $(LIBS)

#               *Individual File Dependencies*
apisupt.obj:	apisupt.asm segs.inc			$(TARGET).lnk
asmsupt.obj:	asmsupt.asm segs.inc			$(TARGET).lnk
console.obj:	console.asm io.inc			$(TARGET).lnk
cpu.obj:	cpu.asm  segs.inc $(TARGET).lnk
dosidle.obj:	dosidle.asm segs.inc			$(TARGET).lnk
entry.obj:	entry.asm   segs.inc $(HDR)stacks.inc	$(TARGET).lnk
execrh.obj:	execrh.asm  segs.inc 			$(TARGET).lnk
int2f.obj:	int2f.asm   segs.inc $(HDR)stacks.inc	$(TARGET).lnk
intr.obj:	intr.asm    segs.inc			$(TARGET).lnk
io.obj:		io.asm	    segs.inc $(HDR)stacks.inc   $(TARGET).lnk
irqstack.obj:   irqstack.asm segs.inc			$(TARGET).lnk
kernel.obj:	kernel.asm  segs.inc ludivmul.inc 	$(TARGET).lnk
memdisk.obj:	memdisk.asm  segs.inc $(TARGET).lnk
nls_hc.obj:	nls_hc.asm  segs.inc			$(TARGET).lnk
nlssupt.obj:	nlssupt.asm segs.inc $(HDR)stacks.inc   $(TARGET).lnk
printer.obj:	printer.asm io.inc			$(TARGET).lnk
procsupt.obj:	procsupt.asm segs.inc $(HDR)stacks.inc  $(TARGET).lnk
serial.obj:	serial.asm  io.inc			$(TARGET).lnk

HDRS=\
    $(HDR)portab.h $(HDR)device.h $(HDR)mcb.h $(HDR)pcb.h \
    $(HDR)fat.h $(HDR)fcb.h $(HDR)tail.h $(HDR)time.h $(HDR)process.h \
    $(HDR)dcb.h $(HDR)sft.h $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h     \
    $(HDR)dirmatch.h $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h  \
    $(HDR)version.h dyndata.h
HEADERS=$(HDRS) globals.h proto.h
INITHEADERS=$(HDRS) init-mod.h init-dat.h

#the string functions for INIT_TEXT
iasmsupt.obj: asmsupt.asm $(TARGET).lnk
	$(NASM) -D$(COMPILER) -D_INIT $(NASMFLAGS) -f obj -o iasmsupt.obj asmsupt.asm

CFILES = blockio.c \
    break.c \
    chario.c \
    dosfns.c \
    dsk.c \
    error.c \
    fatdir.c \
    fatfs.c \
    fattab.c \
    fcbfns.c \
    inthndlr.c \
    ioctl.c \
    memmgr.c \
    misc.c \
    lfnapi.c \
    newstuff.c \
    network.c \
    nls.c \
    prf.c \
    strings.c \
    sysclk.c \
    syspack.c \
    systime.c \
    task.c \
    config.c \
    initoem.c \
    main.c \
    inithma.c \
    dyninit.c \
    initdisk.c \
    initclk.c \
    iprf.c \
    prf.c

OBJECTS = $(CFILES:.c=.o)

$(CFILES): $(HEADERS)

libfdkernel.so: $(OBJECTS)
	$(CL) -shared -o $@ $^

../bin/libfdkernel.so: libfdkernel.so
		$(CP) $^ ..$(DIRSEP)bin
