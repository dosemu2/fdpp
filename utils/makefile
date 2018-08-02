srcdir ?= $(CURDIR)
COMPILER ?= gcc
XCPU ?= 386
include $(srcdir)/../mkfiles/generic.mak

CFLAGS = -iquote $(srcdir)/..$(DIRSEP)hdr

production: patchobj.com exeflat.exe

patchobj.com: $(srcdir)/patchobj.c
	$(CLT) $(CFLAGS) -o $@ $<

exeflat.exe: $(srcdir)/exeflat.c $(srcdir)/../hdr/exe.h
	$(CLC) $(CFLAGS) -o $@ $<

clobber: clean

clean:
	$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.las *.cod *.err status.me
	$(RM) exeflat.exe patchobj.com
