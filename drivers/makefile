#
# makefile for device.lib
#
# $Id: makefile 1387 2009-05-19 21:39:29Z bartoldeman $
#


-include config.mak
srcdir ?= $(CURDIR)
vpath %.asm $(srcdir)
COMPILER = gcc
XNASM = nasm
XCPU = 386
LIBUTIL = $(srcdir)/../fdpp/contrib/JWlib
LIBFLAGS = -fo -n
include $(srcdir)/../mkfiles/generic.mak


# MICROSOFT C
# -----------
#MODEL  = s
#CFLAGS = /c /Gs /A$(MODEL)
#AFLAGS = /Mx /Dmem$(MODEL)=1
#TERM   = ;

# BORLAND C
# -----------
#MODEL  = s
#CFLAGS = -c -m$(MODEL)
#AFLAGS = /Mx /Dmem$(MODEL)=1
#LIBFLAGS = /c

OBJS   = floppy.obj rdpcclk.obj wrpcclk.obj wratclk.obj

LIBOBJS= $(OBJS)



# Build the LIBRARY
# -----------------
all:            production

production:     ../lib/device.lib

../lib/device.lib: device.lib
	$(CP) device.lib ..$(DIRSEP)lib

clobber:        clean
	-$(RM) device.lib status.me ..$(DIRSEP)lib$(DIRSEP)device.lib

clean:
	-$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.cod *.err *.lib

device.lib : $(OBJS)
	-$(RM) device.lib
	$(LIBUTIL) $(LIBFLAGS) $@ $(LIBOBJS) $(LIBTERM)
