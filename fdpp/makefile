-include config.mak
srcdir ?= $(CURDIR)
lib_path ?= ../lib
utils_path ?= ../utils
include $(srcdir)/defs.mak
XLINK = $(srcdir)/contrib/jwlink
include $(srcdir)/clang.mak

LIBS=$(lib_path)/device.lib
HDR=$(srcdir)/../hdr/
SRC=$(srcdir)/../kernel/
FDPPLIB = libfdpp.so
PKG = fdpp-0.1.tar.gz

ALLCFLAGS += -I . -I $(SRC) -I $(srcdir) -DDATADIR=$(DATADIR) -DKRNL_NAME=$(TARGET).sys
NASMFLAGS += -i$(SRC)

# *List Macros*
# Only 8 files per definition; this is limitation of DOS batch
# files (only 9 directly accessible parameters).

OBJS=kernel.obj entry.obj io.obj console.obj serial.obj printer.obj \
    execrh.obj nlssupt.obj procsupt.obj dosidle.obj int2f.obj \
    nls_hc.obj intr.obj irqstack.obj memdisk.obj \
    cpu.obj
PPOBJS = plt.obj cdata.obj

#               *Explicit Rules*

ALL = $(FDPPLIB) $(TARGET).sys fdpp.pc

all: $(ALL)

production:     $(ALL) ../bin/country.sys
clobber:        clean

# -S to avoid showing expected relocations
# 0x10 & 0x78 or 0x79 depending on compilation options
$(TARGET).sys:	kernel.exe $(utils_path)/exeflat.exe
		$(utils_path)/exeflat.exe kernel.exe $(TARGET).sys $(LOADSEG) -S0x10 -S0x78 -S0x79

kernel.exe:	$(OBJS) $(PPOBJS) $(LIBS)
		$(LINK) debug all op symfile format dos option map,statics,verbose F { $(OBJS) $(PPOBJS) } L $(LIBS) N $@

../bin/country.sys: $(SRC)country.asm
	if [ ! -d ../bin ]; then \
	    mkdir ../bin ; \
	fi
	$(NASM) -o $@ $<

clean:
	cd parsers && make srcdir=$(abspath $(srcdir))/parsers clean
	-$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.cod *.err \
		*.lnk *.o $(GEN_CC) $(FDPPLIB) *.tmp $(GEN_HEADERS) $(GEN_ASMS) \
		plt*.inc kernel.exe $(TARGET).sys kernel.sym *.pc

#               *Individual File Dependencies*
#apisupt.obj:	$(SRC)apisupt.asm $(SRC)segs.inc
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
PLPHDRS = farobj.hpp farptr.hpp
_PPHDRS = $(PLPHDRS) dosobj.h farhlp.h thunks_priv.h thunks.h smalloc.h
PPHDRS = $(addprefix $(srcdir)/,$(_PPHDRS))
GEN_HEADERS=thunk_calls.h thunk_asms.h glob_asmdefs.h
GEN_ASMS=plt.asm cdata.asm
# dont change file order as it matches the gen script
GEN_TMP = thunk_calls.tmp thunk_asms.tmp plt.inc plt_asmc.inc plt_asmp.inc
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
FDPP_CPPOBJS = $(CPPFILES:.cpp=.o)

$(OBJECTS): %.o: %.cc $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

$(FDPP_COBJS): %.o: $(srcdir)/%.c $(PPHDRS)
	$(CLC) $(CFLAGS) -o $@ $<

$(FDPP_CCOBJS): %.o: $(srcdir)/%.cc $(PPHDRS) $(GEN_HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

$(FDPP_CPPOBJS): %.o: $(srcdir)/%.cpp
	$(CC) $(CFLAGS) -o $@ $<

$(OBJS): %.obj: $(SRC)%.asm
	$(NASM) $(NASMFLAGS) -f obj -o $@ $<

$(PPOBJS): %.obj: %.asm
	$(NASM) $(NASMFLAGS) -f obj -o $@ $<

.cpp.o: $(PLPHDRS)
	$(CC) $(CFLAGS) -o $@ $<

$(GEN_CC): %.cc: $(SRC)%.c makefile
	$(srcdir)/parsers/mkfar.sh $< >$@

$(FDPPLIB): $(OBJECTS) $(FDPP_COBJS) $(FDPP_CCOBJS) $(FDPP_CPPOBJS)
	$(CL) $(LDFLAGS) -o $@ $^
	@echo "Have `nm -u $@ | grep "U " | wc -l` undefined symbols"

plt.obj: plt.asm plt.inc plt_asmc.inc plt_asmp.inc $(SRC)segs.inc

plt.asm: $(srcdir)/plt.S $(srcdir)/glob_asm.h
	cpp -P $< >$@

cdata.asm: $(srcdir)/cdata.S
	cpp -P $^ | sed 's/" "//g' >$@

_pos = $(if $(findstring $1,$2),$(call _pos,$1,\
       $(wordlist 2,$(words $2),$2),x $3),$3)
pos = $(words $(call _pos,$1,$2))
pars =	srcdir=$(srcdir)/parsers \
	$(srcdir)/parsers/parse_decls.sh $(call pos,$@,$(GEN_TMP)) $< >$@
$(filter %.tmp,$(GEN_TMP)): $(SRC)proto.h
	$(pars)
plt.inc: thunk_calls.tmp
	$(pars)
plt_asmc.inc plt_asmp.inc: thunk_asms.tmp
	$(pars)

thunk_calls.h: thunk_calls.tmp parsers/thunk_gen
	parsers/thunk_gen <$< >$@ || (rm -f $@ ; false)

thunk_asms.h: thunk_asms.tmp parsers/thunk_gen
	parsers/thunk_gen 1 <$< >$@ || (rm -f $@ ; false)

glob_asmdefs.h: $(srcdir)/glob_asm.h
	$(srcdir)/parsers/mkasmdefs.sh $< >$@

parsers/thunk_gen:
	if [ ! -f parsers/makefile ]; then \
	    ln -s $(abspath $(srcdir))/parsers/makefile parsers/makefile ; \
	fi
	+cd parsers && make srcdir=$(abspath $(srcdir))/parsers

install: $(FDPPLIB) fdpp.pc
	install -D -t $(DESTDIR)$(LIBDIR)/fdpp $(FDPPLIB)
	install -D -t $(DESTDIR)$(PKGCONFIGDIR) -m 0644 fdpp.pc
	install -D -t $(DESTDIR)$(PREFIX)/include/fdpp -m 0644 $(srcdir)/thunks.h
	install -D -t $(DESTDIR)$(DATADIR) -m 0644 fdppkrnl.sys

$(PKG):
	cd .. && git archive -o fdpp/fdpp-0.1.tar.gz --prefix=fdpp-0.1/ HEAD
.PHONY: $(PKG)

tar: $(PKG)

rpm: $(PKG)
	rpmbuild -tb $(PKG)

deb:
	debuild -i -us -uc -b

fdpp.pc: $(srcdir)/fdpp.pc.in makefile
	sed \
		-e 's!@PREFIX[@]!$(PREFIX)!g' \
		-e 's!@LIBDIR[@]!$(LIBDIR)!g' \
		$< >$@
