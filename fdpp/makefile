#
# Makefile for kernel.sys  (originally for Borland C/C++ 3.1)
#

include config.mak
XLINK = contrib/jwlink
include clang.mak

LIBS=..$(DIRSEP)lib$(DIRSEP)device.lib
HDR=../hdr/
SRC=../kernel/
FDPPLIB = libfdpp.so

ALLCFLAGS += -I $(SRC) -I . -DDATADIR=$(DATADIR) -DKRNL_NAME=$(TARGET).sys
NASMFLAGS += -i$(SRC)

# *List Macros*
# Only 8 files per definition; this is limitation of DOS batch
# files (only 9 directly accessible parameters).

OBJS=kernel.obj entry.obj io.obj console.obj serial.obj printer.obj \
    execrh.obj nlssupt.obj procsupt.obj dosidle.obj int2f.obj \
    nls_hc.obj apisupt.obj intr.obj irqstack.obj memdisk.obj \
    cpu.obj
PPOBJS = plt.obj cdata.obj

#               *Explicit Rules*

production:     $(FDPPLIB) $(TARGET).sys ../bin/country.sys

# -S to avoid showing expected relocations
# 0x10 & 0x78 or 0x79 depending on compilation options
$(TARGET).sys:	kernel.exe ../utils/exeflat.exe
		..$(DIRSEP)utils$(DIRSEP)exeflat.exe kernel.exe $(TARGET).sys $(LOADSEG) -S0x10 -S0x78 -S0x79

kernel.exe:	$(OBJS) $(PPOBJS) $(LIBS)
		$(LINK) debug all op symfile format dos option map,statics,verbose F { $(OBJS) $(PPOBJS) } L ../lib/device.lib N $@

../bin/country.sys: $(SRC)country.asm
		$(NASM) -o $@ $<

clean:
	cd utils && make clean
	-$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.cod *.err \
		*.lnk *.o $(GEN_CC) *.so *.tmp $(GEN_HEADERS) $(GEN_ASMS) \
		plt*.inc kernel.exe $(TARGET).sys kernel.sym

#               *Individual File Dependencies*
apisupt.obj:	$(SRC)apisupt.asm $(SRC)segs.inc
console.obj:	$(SRC)console.asm $(SRC)io.inc
cpu.obj:	$(SRC)cpu.asm  $(SRC)segs.inc
dosidle.obj:	$(SRC)dosidle.asm $(SRC)segs.inc
entry.obj:	$(SRC)entry.asm   $(SRC)segs.inc $(HDR)stacks.inc
execrh.obj:	$(SRC)execrh.asm  $(SRC)segs.inc
int2f.obj:	$(SRC)int2f.asm   $(SRC)segs.inc $(HDR)stacks.inc
intr.obj:	$(SRC)intr.asm    $(SRC)segs.inc
io.obj:		$(SRC)io.asm	    $(SRC)segs.inc $(HDR)stacks.inc
irqstack.obj:   $(SRC)irqstack.asm $(SRC)segs.inc
kernel.obj:	$(SRC)kernel.asm  $(SRC)segs.inc $(SRC)ludivmul.inc
memdisk.obj:	$(SRC)memdisk.asm  $(SRC)segs.inc
nls_hc.obj:	$(SRC)nls_hc.asm  $(SRC)segs.inc
nlssupt.obj:	$(SRC)nlssupt.asm $(SRC)segs.inc $(HDR)stacks.inc
printer.obj:	$(SRC)printer.asm $(SRC)io.inc
procsupt.obj:	$(SRC)procsupt.asm $(SRC)segs.inc $(HDR)stacks.inc
serial.obj:	$(SRC)serial.asm  $(SRC)io.inc

HDRS=\
    $(HDR)portab.h $(HDR)device.h $(HDR)mcb.h $(HDR)pcb.h \
    $(HDR)fat.h $(HDR)fcb.h $(HDR)tail.h $(HDR)time.h $(HDR)process.h \
    $(HDR)dcb.h $(HDR)sft.h $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h     \
    $(HDR)dirmatch.h $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h  \
    $(HDR)version.h $(SRC)dyndata.h
PLPHDRS = $(wildcard $(HDR)*.hpp)
PPHDRS = $(PLPHDRS) dosobj.h farhlp.h thunks_priv.h thunks.h smalloc.h
GEN_HEADERS=thunk_calls.h thunk_asms.h glob_asmdefs.h
GEN_ASMS=plt.asm cdata.asm
GEN_CC = $(CFILES:.c=.cc)
HEADERS=$(HDRS) $(SRC)globals.h $(SRC)proto.h $(GEN_HEADERS) $(PPHDRS)
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
    prf.c

FDPP_CFILES = \
    smalloc.c \
    dosobj.c \
    farhlp.c
FDPP_CCFILES = \
    thunks.cc

CPPFILES = objhlp.cpp

OBJECTS = $(CFILES:.c=.o)
FDPP_COBJS = $(FDPP_CFILES:.c=.o)
FDPP_CCOBJS = $(FDPP_CCFILES:.cc=.o)

$(OBJECTS): %.o: %.cc $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

$(FDPP_COBJS): %.o: %.c $(PPHDRS)
	$(CLC) $(CFLAGS) -o $@ $<

$(FDPP_CCOBJS): %.o: %.cc $(PPHDRS) $(GEN_HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

$(OBJS): %.obj: $(SRC)%.asm
	$(NASM) $(NASMFLAGS) -f obj -o $@ $<

$(PPOBJS): %.obj: %.asm
	$(NASM) $(NASMFLAGS) -f obj -o $@ $<

.cpp.o: $(PLPHDRS)
	$(CC) $(CFLAGS) -o $@ $<

$(CFILES:.c=.cc): %.cc: $(SRC)%.c makefile
	utils/mkfar.sh FAR __FAR $< >$@

$(FDPPLIB): $(OBJECTS) $(FDPP_COBJS) $(FDPP_CCOBJS) $(CPPFILES:.cpp=.o)
	$(CL) $(LDFLAGS) -o $@ $^
	@echo "Have `nm -u $@ | grep "U " | wc -l` undefined symbols"

plt.obj: plt.asm plt.inc plt_asmc.inc plt_asmp.inc $(SRC)segs.inc

plt.asm: plt.S glob_asm.h
	cpp -P $< >$@

cdata.asm: cdata.S
	cpp -P $^ | sed 's/" "//g' >$@

thunk_calls.tmp: $(SRC)proto.h makefile
	grep ASMCFUNC $< | sed -e 's/__FAR(\([^)]\+\))/\1 FAR */g' \
		-e 's/__DOSFAR(\([^)]\+\))/\1 FAR */g' | nl -n ln -v 0 >$@

plt.inc: thunk_calls.tmp
	grep ") FAR " $< | sed 's/\([0-9]\+\).\+ \(SEGM(\(.\+\))\).* \([^ (]\+\) *(.*/asmcfunc_f \4,\t\t\1,\t\3/' >$@
	grep -v ") FAR " $< | sed 's/\([0-9]\+\).\+ \(SEGM(\(.\+\))\).* \([^ (]\+\) *(.*/asmcfunc_n \4,\t\t\1,\t\3/' >>$@

thunk_calls.h: thunk_calls.tmp utils/thunk_gen
	cat $< | utils/thunk_gen >$@ || (rm -f $@ ; false)

thunk_asms.tmp: $(SRC)proto.h makefile
	grep 'ASMFUNC\|ASMPASCAL' $< | grep -v "//" | \
		sed 's/__FAR(\([^)]\+\))/\1 FAR */g' | nl -n ln -v 0 >$@

thunk_asms.h: thunk_asms.tmp utils/thunk_gen
	cat $< | utils/thunk_gen 1 >$@ || (rm -f $@ ; false)

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

utils/thunk_gen:
	cd utils && make

install: $(FDPPLIB)
	install -d $(DESTDIR)$(LIBDIR)
	install -t $(DESTDIR)$(LIBDIR) $(FDPPLIB)
	install -D -m 0644 -t $(DESTDIR)$(PREFIX)/include/fdpp thunks.h
	install -d $(DESTDIR)$(DATADIR)
	install -t $(DESTDIR)$(DATADIR) -m 0644 fdppkrnl.sys

rpm:
	cd .. && git archive -o fdpp/fdpp-0.1.tar.gz --prefix=fdpp-0.1/ HEAD
	rpmbuild -tb fdpp-0.1.tar.gz
