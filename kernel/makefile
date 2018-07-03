#
# Makefile for kernel.sys  (originally for Borland C/C++ 3.1)
#

COMPILER ?= gcc
XNASM ?= nasm
XCPU ?= 386
XLINK = ../contrib/jwlink
XFAT ?= 32
include ../mkfiles/generic.mak

LIBS=..$(DIRSEP)lib$(DIRSEP)device.lib
HDR=../hdr/
FDPPLIB = libfdpp.so

# *List Macros*
# Only 8 files per definition; this is limitation of DOS batch
# files (only 9 directly accessible parameters).

OBJS=kernel.obj entry.obj io.obj console.obj serial.obj printer.obj \
    execrh.obj nlssupt.obj procsupt.obj dosidle.obj int2f.obj \
    nls_hc.obj apisupt.obj intr.obj irqstack.obj memdisk.obj \
    cpu.obj plt.obj cdata.obj

#               *Explicit Rules*

production:     $(FDPPLIB) ../bin/$(TARGET).sys ../bin/country.sys

../bin/$(TARGET).sys: kernel.sys
		$(CP) kernel.sys ..$(DIRSEP)bin
		$(CP) kernel.sys ..$(DIRSEP)bin$(DIRSEP)$(TARGET).sys
		$(CP) kernel.map ..$(DIRSEP)bin$(DIRSEP)$(TARGET).map

# -S to avoid showing expected relocations
# 0x10 & 0x78 or 0x79 depending on compilation options
kernel.sys:	kernel.exe ../utils/exeflat.exe
		..$(DIRSEP)utils$(DIRSEP)exeflat.exe kernel.exe kernel.sys $(LOADSEG) -S0x10 -S0x78 -S0x79 $(UPXOPT) $(XUPX)

kernel.exe:	$(OBJS) $(LIBS)
		$(LINK) debug all op symfile format dos option map,statics,verbose F { $(OBJS) } L ../lib/device.lib N $@

../bin/country.sys: country.asm
		$(NASM) -o $@ country.asm

clobber:        clean
		-$(RM) kernel.exe kernel.sys status.me

clean:
		-$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.cod *.err \
			*.lnk *.o *.cc *.so *.tmp $(GEN_HEADERS) $(GEN_ASMS)

# XXX: This is a very ugly way of linking the kernel, forced upon us by the
# inability of Turbo `make' 2.0 to perform command line redirection. -- ror4

$(TARGET).lnk: turboc.cfg makefile ../mkfiles/generic.mak ../mkfiles/$(COMPILER).mak
		-$(RM) *.lnk
		$(ECHOTO) $(TARGET).lnk $(OBJS)
		$(ECHOTO) $(TARGET).lnk kernel.exe
		$(ECHOTO) $(TARGET).lnk kernel.map
		$(ECHOTO) $(TARGET).lnk $(LIBS)

#               *Individual File Dependencies*
apisupt.obj:	apisupt.asm segs.inc
console.obj:	console.asm io.inc
cpu.obj:	cpu.asm  segs.inc
dosidle.obj:	dosidle.asm segs.inc
entry.obj:	entry.asm   segs.inc $(HDR)stacks.inc
execrh.obj:	execrh.asm  segs.inc
int2f.obj:	int2f.asm   segs.inc $(HDR)stacks.inc
intr.obj:	intr.asm    segs.inc
io.obj:		io.asm	    segs.inc $(HDR)stacks.inc
irqstack.obj:   irqstack.asm segs.inc
kernel.obj:	kernel.asm  segs.inc ludivmul.inc
memdisk.obj:	memdisk.asm  segs.inc
nls_hc.obj:	nls_hc.asm  segs.inc
nlssupt.obj:	nlssupt.asm segs.inc $(HDR)stacks.inc
printer.obj:	printer.asm io.inc
procsupt.obj:	procsupt.asm segs.inc $(HDR)stacks.inc
serial.obj:	serial.asm  io.inc

HDRS=\
    $(HDR)portab.h $(HDR)device.h $(HDR)mcb.h $(HDR)pcb.h \
    $(HDR)fat.h $(HDR)fcb.h $(HDR)tail.h $(HDR)time.h $(HDR)process.h \
    $(HDR)dcb.h $(HDR)sft.h $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h     \
    $(HDR)dirmatch.h $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h  \
    $(HDR)version.h dyndata.h
GEN_HEADERS=thunk_calls.h thunk_asms.h glob_asmdefs.h
GEN_ASMS=plt.asm cdata.asm
HEADERS=$(HDRS) globals.h proto.h $(GEN_HEADERS)
INITHEADERS=$(HDRS) init-mod.h init-dat.h

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
    lfnapi.c \
    newstuff.c \
    network.c \
    nls.c \
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
    prf.c \
    thunks.c \
    smalloc.c \
    dosobj.c \
    farhlp.c

OBJECTS = $(CFILES:.c=.o)

$(OBJECTS): %.o: %.cc $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

$(CFILES:.c=.cc): makefile
.c.cc:
	../utils/mkfar.sh FAR __FAR $< >$@

$(FDPPLIB): $(OBJECTS)
	$(CL) $(LDFLAGS) -o $@ $^
	@echo "Have `nm -u $@ | grep "U " | wc -l` undefined symbols"

plt.obj: plt.asm plt.inc plt_asmc.inc plt_asmp.inc segs.inc

plt.asm: plt.S glob_asm.h
	cpp -P $< >$@

cdata.asm: cdata.S
	cpp -P $^ | sed 's/" "//g' >$@

thunk_calls.tmp: proto.h makefile
	grep ASMCFUNC $< | sed -e 's/__FAR(\([^)]\+\))/\1 FAR */g' \
		-e 's/__DOSFAR(\([^)]\+\))/\1 FAR */g' | nl -n ln -v 0 >$@

plt.inc: thunk_calls.tmp
	grep ") FAR " $< | sed 's/\([0-9]\+\).\+ \(SEGM(\(.\+\))\).* \([^ (]\+\) *(.*/asmcfunc_f \4,\t\t\1,\t\3/' >$@
	grep -v ") FAR " $< | sed 's/\([0-9]\+\).\+ \(SEGM(\(.\+\))\).* \([^ (]\+\) *(.*/asmcfunc_n \4,\t\t\1,\t\3/' >>$@

thunk_calls.h: thunk_calls.tmp
	cat $^ | ../utils/thunk_gen >$@ || (rm -f $@ ; false)

thunk_asms.tmp: proto.h makefile
	grep 'ASMFUNC\|ASMPASCAL' $< | grep -v "//" | \
		sed 's/__FAR(\([^)]\+\))/\1 FAR */g' | nl -n ln -v 0 >$@

thunk_asms.h: thunk_asms.tmp makefile
	cat $< | ../utils/thunk_gen 1 >$@ || (rm -f $@ ; false)

plt_asmc.inc: thunk_asms.tmp makefile
	grep ASMFUNC $< | sed 's/\([0-9]\+\).\+ \([^ (]\+\) *(.\+/asmcsym \2,\t\t\1/' >$@

plt_asmp.inc: thunk_asms.tmp makefile
	grep ASMPASCAL $< | sed 's/\([0-9]\+\).\+ \([^ (]\+\) *(.\+/asmpsym \U\2,\t\t\1/' >$@

glob_asmdefs.h: glob_asm.h makefile
	grep "__ASM(" $< | grep -v '^/' | \
		sed 's/.\+, \(.\+\)).*/#define \1 __ASYM(__\1)/' >$@
	grep "__ASM_ARR(" $< | grep -v '^/' | \
		sed 's/.\+(.\+, \(.\+\), .\+).*/#define \1 __ASYM(__\1)/' >>$@
	grep "__ASM_ARRI(" $< | grep -v '^/' | \
		sed 's/.\+(.\+, \(.\+\)).*/#define \1 __ASYM(__\1)/' >>$@
	grep "__ASM_ARRI_F(" $< | grep -v '^/' | \
		sed 's/.\+(.\+, \(.\+\)).*/#define \1 __ASYM(__\1)/' >>$@
	grep "__ASM_FAR(" $< | grep -v '^/' | \
		sed 's/.\+, \(.\+\)).*/#define \1 __ASYM(__\1)/' >>$@
	grep "__ASM_NEAR(" $< | grep -v '^/' | \
		sed 's/.\+, \(.\+\)).*/#define \1 __ASYM(__\1)/' >>$@
	grep "__ASM_FUNC(" $< | grep -v '^/' | \
		sed 's/.\+(\(.\+\)).*/#define \1 __ASYM(__\1)/' >>$@

install: $(FDPPLIB)
	install -t /usr/local/lib $(FDPPLIB)
	install -D -m 0644 -t /usr/local/include/fdpp thunks.h
