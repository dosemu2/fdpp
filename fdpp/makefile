-include config.mak
srcdir ?= $(CURDIR)
# the below is needed to prevent _our_ srcdir being passed
# down to rpmbuild/debuild, and, subsequently, to their make!
unexport srcdir
MAKEOVERRIDES =

include $(srcdir)/defs.mak
include $(srcdir)/clang.mak

HDR=$(srcdir)/../hdr/
SRC=$(srcdir)/../kernel/
FDPPLIB = libfdpp.so
RELVER = beta8
MAJ = 0.1
VERSION = $(MAJ)$(RELVER)
PKG = fdpp-$(VERSION)
TAR = $(PKG).tar
TGZ = $(TAR).gz
FD_EXT_H = bprm.h memtype.h
EXT_H = $(FD_EXT_H) thunks.h
GEN_EXT = fdpp.pc $(EXT_H)

GIT_REV := $(shell $(srcdir)/git-rev.sh)
.LOW_RESOLUTION_TIME: $(GIT_REV)

CPPFLAGS += -I . -I $(SRC) -I $(srcdir) -DDATADIR=$(DATADIR) \
    -DKRNL_NAME=$(TARGET).sys -DKRNL_MAP_NAME=$(TARGET).map \
    -DKERNEL_VERSION="$(VERSION) [GIT: `git describe`]"
NASMFLAGS += -i$(SRC)

# *List Macros*
# Only 8 files per definition; this is limitation of DOS batch
# files (only 9 directly accessible parameters).

ASMS = kernel.asm entry.asm io.asm console.asm serial.asm printer.asm \
    execrh.asm nlssupt.asm procsupt.asm dosidle.asm int2f.asm \
    nls_hc.asm intr.asm irqstack.asm memdisk.asm \
    cpu.asm
DASMS = floppy.asm rdpcclk.asm wrpcclk.asm wratclk.asm
AASMS = $(ASMS) $(DASMS)
OBJS = $(ASMS:.asm=.o)
DOBJS = $(DASMS:.asm=.o)
INCS = $(AASMS:.asm=_seg.inc) cdata_seg.inc
PPASMS = plt.S cdata.S
PPOBJS = $(PPASMS:.S=.o)

#               *Explicit Rules*

ALL = $(FDPPLIB) $(TARGET).sys $(GEN_EXT)

all: $(ALL)

$(TARGET).sys: $(srcdir)/kernel.ld $(OBJS) $(PPOBJS) $(DOBJS)
	$(LINK) -melf_i386 -static -Map $(TARGET).map -o $@ $(^:%.ld=-T%.ld)
	chmod -x $@

clean:
	+cd parsers && $(MAKE) srcdir=$(abspath $(srcdir))/parsers clean
	-$(RM) .tstamp *.map *.elf *.inc \
		*.o $(GEN_CC) $(FDPPLIB) *.tmp $(GEN_HEADERS) \
		$(GEN_HEADERS_FD) $(GEN_ASMS) \
		$(TARGET).sys kernel.sym *.pc fdpp.spec

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

HDRS = $(wildcard $(HDR)*.h) $(wildcard $(SRC)*.h)
PLPHDRS = farobj.hpp farptr.hpp dispatch.hpp ctors.hpp
_PPHDRS = $(PLPHDRS) dosobj.h farhlp.hpp thunks_priv.h thunks.h smalloc.h
PPHDRS = $(addprefix $(srcdir)/,$(_PPHDRS))
GEN_HEADERS = thunk_calls.h thunk_asms.h
GEN_HEADERS_FD = glob_asmdefs.h
GEN_ASMS = plt.asm cdata.asm
# dont change file order in GEN_TMP as it matches the gen script
GEN_TMP = thunk_calls.tmp thunk_asms.tmp plt.inc plt_asmc.inc plt_asmp.inc
GEN_CC = $(CFILES:.c=.cc)
INITHEADERS = $(SRC)init-mod.h $(SRC)init-dat.h
HEADERS=$(HDRS) $(SRC)globals.h $(SRC)proto.h $(INITHEADERS) $(PPHDRS)

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

FDPP_CFILES = smalloc.c
FDPP_CCFILES = thunks.cc dosobj.cc
CPPFILES = objhlp.cpp ctors.cpp farhlp.cpp objtrace.cpp

OBJECTS = $(CFILES:.c=.o)
FDPP_COBJS = $(FDPP_CFILES:.c=.o)
FDPP_CCOBJS = $(FDPP_CCFILES:.cc=.o)
FDPP_CPPOBJS = $(CPPFILES:.cpp=.o)

$(OBJECTS): %.o: %.cc $(HEADERS) $(GEN_HEADERS_FD)
	$(CC) $(CFLAGS) -o $@ $<

$(FDPP_COBJS): %.o: $(srcdir)/%.c $(PPHDRS) $(srcdir)/makefile
	$(CLC) $(CLCFLAGS) -o $@ $<

thunks.o: $(GEN_HEADERS) $(GIT_REV)
$(FDPP_CCOBJS): %.o: $(srcdir)/%.cc $(PPHDRS) $(srcdir)/makefile
	$(CC) $(CFLAGS) -o $@ $<

$(FDPP_CPPOBJS): %.o: $(srcdir)/%.cpp $(PPHDRS) $(srcdir)/makefile
	$(CC) $(CFLAGS) -o $@ $<

$(OBJS): %.o: $(SRC)%.asm
	$(NASM) $(NASMFLAGS) -f elf -o $@ $<

$(DOBJS): %.o: $(SRC)drivers/%.asm
	$(NASM) $(NASMFLAGS) -f elf -o $@ $<

$(PPOBJS): %.o: %.asm
	$(NASM) $(NASMFLAGS) -f elf -o $@ $<

.cpp.o: $(PLPHDRS)
	$(CC) $(CFLAGS) -o $@ $<

$(GEN_CC): %.cc: $(SRC)%.c makefile
	$(srcdir)/parsers/mkfar.sh $< >$@

$(FDPPLIB): $(OBJECTS) $(FDPP_COBJS) $(FDPP_CCOBJS) $(FDPP_CPPOBJS)
	$(CL) -o $@ $^ $(LDFLAGS)
	@echo "Have `nm -u $@ | grep "U " | wc -l` undefined symbols"

$(INCS): %_seg.inc: %.o
	objdump -t $< | $(srcdir)/parsers/gen_segs.sh $(SRC)segs.inc >$@

plt_seg.inc: $(INCS)
	echo -e $(patsubst %.inc,\%include \"%.inc\"\\n,$^) | sed 's/^ //' >$@

plt.o: plt.asm plt.inc plt_asmc.inc plt_asmp.inc plt_seg.inc $(SRC)segs.inc

plt.asm: $(srcdir)/plt.S $(SRC)/glob_asm.h
	cpp -I $(SRC) -P $< >$@

cdata.asm: $(srcdir)/cdata.S $(HDR)/version.h $(GIT_REV) $(srcdir)/makefile
	cpp -P $(CPPFLAGS) $< | sed 's/" "/","/g' >$@

_pos = $(if $(findstring $1,$2),$(call _pos,$1,\
       $(wordlist 2,$(words $2),$2),x $3),$3)
pos = $(words $(call _pos,$1,$2))
pars = srcdir=$(srcdir)/parsers \
	$(srcdir)/parsers/parse_decls.sh $(call pos,$@,$(GEN_TMP)) $< >$@
$(filter %.tmp,$(GEN_TMP)): $(SRC)proto.h
	$(pars)
plt.inc: thunk_calls.tmp
	$(pars)
plt_asmc.inc plt_asmp.inc: thunk_asms.tmp
	$(pars)

thunk_calls.h: thunk_calls.tmp parsers/thunk_gen
	parsers/thunk_gen <$< >$@ || (rm -f $@ ; false)

thunk_asms.h: thunk_asms.tmp parsers/thunk_gen $(srcdir)/parsers/thunks.m4
	$(srcdir)/parsers/gen_thunks.sh $< $(srcdir)/parsers >$@ || \
		(rm -f $@ ; false)

glob_asmdefs.h: $(SRC)/glob_asm.h
	$(srcdir)/parsers/mkasmdefs.sh $< >$@

parsers/thunk_gen: $(srcdir)/parsers/thunk_gen.y $(srcdir)/parsers/thunk_gen.l
	if [ ! -f parsers/makefile ]; then \
	    ln -s $(abspath $(srcdir))/parsers/makefile parsers/makefile ; \
	fi
	+cd parsers && $(MAKE) srcdir=$(abspath $(srcdir))/parsers

install: $(FDPPLIB) $(GEN_EXT)
	install -D -t $(DESTDIR)$(LIBDIR)/fdpp $(FDPPLIB)
	install -D -t $(DESTDIR)$(PKGCONFIGDIR) -m 0644 fdpp.pc
	install -D -t $(DESTDIR)$(PREFIX)/include/fdpp -m 0644 $(EXT_H)
	install -D -t $(DESTDIR)$(DATADIR) -m 0644 fdppkrnl.sys
	install -D -t $(DESTDIR)$(DATADIR) -m 0644 fdppkrnl.map

$(TGZ): fdpp.spec
	cd $(srcdir)/.. && git archive -o $(CURDIR)/$(TAR) --prefix=$(PKG)/ HEAD
	tar rf $(TAR) --add-file=fdpp.spec
	gzip $(TAR)
.PHONY: $(TGZ)

tar: $(TGZ)

rpm: $(TGZ)
	rpmbuild -tb $(TGZ)

deb:
	debuild -i -us -uc -b

fdpp.spec fdpp.pc: %: $(srcdir)/%.in makefile
	sed \
		-e 's!@PREFIX[@]!$(PREFIX)!g' \
		-e 's!@LIBDIR[@]!$(LIBDIR)!g' \
		-e 's!@VERSION[@]!$(VERSION)!g' \
		$< >$@

$(FD_EXT_H): %: $(HDR)%
	ln -s $< $@
thunks.h: $(srcdir)/thunks.h
	ln -s $< $@
