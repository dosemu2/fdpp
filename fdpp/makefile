-include config.mak
-include local.mak
srcdir ?= $(CURDIR)
# the below is needed to prevent _our_ srcdir being passed
# down to rpmbuild/debuild, and, subsequently, to their make!
unexport srcdir
MAKEOVERRIDES =

include $(srcdir)/defs.mak
include $(srcdir)/clang.mak

TOP = $(abspath $(srcdir)/..)
HDR=$(TOP)/hdr
SRC=$(TOP)/kernel
FDPPLIB = libfdpp.so
RELVER = 6
MAJ = 1
VERSION = $(MAJ).$(RELVER)
PKG = fdpp-$(VERSION)
TAR = $(PKG).tar
TGZ = $(TAR).gz
FD_EXT_H = $(TOP)/include/fdpp
EXT_H = $(FD_EXT_H)/thunks.h $(FD_EXT_H)/bprm.h $(FD_EXT_H)/memtype.h
GEN_EXT = fdpp.pc

GIT_REV := $(shell $(srcdir)/git-rev.sh)
GIT_DESCRIBE := $(shell git describe)
.LOW_RESOLUTION_TIME: $(GIT_REV)

CPPFLAGS += -I . -I $(FD_EXT_H) -I $(SRC) -I $(srcdir) \
    -DFDPPKRNLDIR=$(DATADIR)/fdpp \
    -DKRNL_ELFNAME=$(TARGET).elf -DKRNL_MAP_NAME=$(TARGET).map \
    -DKERNEL_VERSION="$(VERSION) [GIT: $(GIT_DESCRIBE)]"
NASMFLAGS += -i$(SRC)/ -f elf32

# *List Macros*
# Only 8 files per definition; this is limitation of DOS batch
# files (only 9 directly accessible parameters).

ASMS = kernel.asm entry.asm io.asm console.asm serial.asm printer.asm \
    execrh.asm nlssupt.asm procsupt.asm dosidle.asm int2f.asm \
    nls_hc.asm intr.asm irqstack.asm \
    cpu.asm
DASMS = floppy.asm rdpcclk.asm wrpcclk.asm wratclk.asm
OBJS = $(ASMS:.asm=.o)
DOBJS = $(DASMS:.asm=.o)
PPASMS = plt.S cdata.S
PPOBJS = $(PPASMS:.S=.o)

#               *Explicit Rules*

ALL = $(FDPPLIB) $(TARGET).elf $(TARGET).map $(GEN_EXT)

all: $(ALL)

elf.o: rel.h

# New makes have a way to avoid parallel invocation with the use of &:
# On old make you would write "%.elf %.map :" which is a bit silly
# given the names are fixed. Also $(TARGET).% won't work that way at all:
# you need to explicitly list all files that are created simultaneously,
# and yet use pattern in all of them.
DEPS_OF_ELF = $(srcdir)/kernel.ld $(OBJS) $(PPOBJS) $(DOBJS)
need = 4.3
ifneq ($(filter $(need),$(firstword $(sort $(MAKE_VERSION) $(need)))),)
$(TARGET).elf $(TARGET).map &: $(DEPS_OF_ELF)
else
%.elf %.map : $(DEPS_OF_ELF)
endif
	$(CROSS_LD) -melf_i386 -static --build-id=sha1 -Map=$(TARGET).map -o $@ \
		$(^:%.ld=-T%.ld) --defsym=DOS_PSP=0x60
	chmod -x $@

$(TARGET).elf.tmp: $(srcdir)/kernel.ld $(OBJS) $(PPOBJS) $(DOBJS)
	$(CROSS_LD) -melf_i386 -static --build-id=sha1 -o $@ \
		$(^:%.ld=-T%.ld) --defsym=DOS_PSP=0
	chmod -x $@

rel.h: $(TARGET).elf $(TARGET).elf.tmp
	cmp -l $^ | grep "140   0" | sed 's/^ *//' | cut -d " " -f 1 | \
		sed 's/$$/,/' > $@


clean:
	+cd parsers && $(MAKE) srcdir=$(abspath $(srcdir))/parsers clean
	-$(RM) .tstamp *.map $(TARGET).elf *.inc \
		*.o $(GEN_CC) $(FDPPLIB) *.tmp $(GEN_HEADERS) \
		$(GEN_HEADERS_FD) $(GEN_ASMS) \
		*.pc

#               *Individual File Dependencies*
#apisupt.o:	$(SRC)/apisupt.asm $(SRC)/segs.inc
console.o:	$(SRC)/console.asm $(SRC)/io.inc
cpu.o:	$(SRC)/cpu.asm  $(SRC)/segs.inc
dosidle.o:	$(SRC)/dosidle.asm $(SRC)/segs.inc
entry.o:	$(SRC)/entry.asm   $(SRC)/segs.inc $(HDR)/stacks.inc
execrh.o:	$(SRC)/execrh.asm  $(SRC)/segs.inc
int2f.o:	$(SRC)/int2f.asm   $(SRC)/segs.inc $(HDR)/stacks.inc
intr.o:	$(SRC)/intr.asm    $(SRC)/segs.inc
io.o:		$(SRC)/io.asm	    $(SRC)/segs.inc $(HDR)/stacks.inc
irqstack.o:   $(SRC)/irqstack.asm $(SRC)/segs.inc
kernel.o:	$(SRC)/kernel.asm  $(SRC)/segs.inc
nls_hc.o:	$(SRC)/nls_hc.asm  $(SRC)/segs.inc
nlssupt.o:	$(SRC)/nlssupt.asm $(SRC)/segs.inc $(HDR)/stacks.inc
printer.o:	$(SRC)/printer.asm $(SRC)/io.inc
procsupt.o:	$(SRC)/procsupt.asm $(SRC)/segs.inc $(HDR)/stacks.inc
serial.o:	$(SRC)/serial.asm  $(SRC)/io.inc

HDRS = $(wildcard $(HDR)/*.h) $(wildcard $(SRC)/*.h)
PLPHDRS = farobj.hpp farptr.hpp dispatch.hpp ctors.hpp
_PPHDRS = $(PLPHDRS) dosobj.h farhlp.hpp thunks_priv.h smalloc.h \
    farhlp_sta.h
PPHDRS = $(addprefix $(srcdir)/,$(_PPHDRS))
GEN_HEADERS = thunk_calls.h thunk_asms.h rel.h
GEN_HEADERS_FD = glob_asmdefs.h
GEN_ASMS = plt.asm cdata.asm
# dont change file order in GEN_TMP as it matches the gen script
GEN_TMP = thunk_calls.tmp thunk_asms.tmp plt.inc plt_asmc.inc plt_asmp.inc
GEN_CC = $(CFILES:.c=.cc)
INITHEADERS = $(SRC)/init-mod.h $(SRC)/init-dat.h
HEADERS=$(HDRS) $(SRC)/globals.h $(SRC)/proto.h $(INITHEADERS) $(PPHDRS)

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
    hmamgr.c \
    inthndlr.c \
    ioctl.c \
    memmgr.c \
    misc.c \
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
    prf.c \
    share.c

FDPP_CFILES = smalloc.c farhlp_sta.c elf.c
FDPP_CCFILES = thunks.cc thunks_c.cc thunks_a.cc thunks_p.cc dosobj.cc
CPPFILES = objhlp.cpp ctors.cpp farhlp.cpp objtrace.cpp
LIBS = $(shell $(PKG_CONFIG) --libs libelf)

OBJECTS = $(CFILES:.c=.o)
FDPP_COBJS = $(FDPP_CFILES:.c=.o)
FDPP_CCOBJS = $(FDPP_CCFILES:.cc=.o)
FDPP_CPPOBJS = $(CPPFILES:.cpp=.o)

$(OBJECTS): %.o: %.cc $(HEADERS) $(GEN_HEADERS_FD) $(EXT_H)
	$(CC) $(CFLAGS) -o $@ $<

$(FDPP_COBJS): %.o: $(srcdir)/%.c $(PPHDRS) $(srcdir)/makefile
	$(CC) $(CLCFLAGS) -I . -o $@ $<

thunks.o: $(EXT_H) $(GEN_HEADERS) $(SRC)/glob_asm.h $(srcdir)/glob_tmpl.h $(GIT_REV)
$(FDPP_CCOBJS): %.o: $(srcdir)/%.cc $(PPHDRS) $(srcdir)/makefile
	$(CC) $(CFLAGS) -o $@ $<

$(FDPP_CPPOBJS): %.o: $(srcdir)/%.cpp $(PPHDRS) $(srcdir)/makefile
	$(CC) $(CFLAGS) -o $@ $<

$(OBJS): %.o: $(SRC)/%.asm makefile
	$(NASM) $(NASMFLAGS) -o $@ $<

$(DOBJS): %.o: $(SRC)/drivers/%.asm makefile
	$(NASM) $(NASMFLAGS) -o $@ $<

$(PPOBJS): %.o: %.asm makefile
	$(NASM) $(NASMFLAGS) -o $@ $<

$(GEN_CC): %.cc: $(SRC)/%.c makefile
	$(srcdir)/parsers/mkfar.sh $< >$@

$(FDPPLIB): $(OBJECTS) $(FDPP_COBJS) $(FDPP_CCOBJS) $(FDPP_CPPOBJS)
	$(CC_LD) -o $@ $^ $(LDFLAGS) $(LIBS)
	@echo "Have `nm -u $@ | grep "U " | wc -l` undefined symbols"

plt.o: plt.asm plt.inc plt_asmc.inc plt_asmp.inc $(SRC)/segs.inc

plt.asm: $(srcdir)/plt.S $(SRC)/glob_asm.h $(TOP)/include/fdpp/bprm.h $(srcdir)/makefile
	$(CPP) $(CPPFLAGS) -P $< >$@ || ($(RM) -f $@ ; false)

cdata.asm: $(srcdir)/cdata.S $(HDR)/version.h $(GIT_REV) $(srcdir)/makefile
	$(CPP) $(CPPFLAGS) -P $< | sed 's/" "/","/g' >$@

_pos = $(if $(findstring $1,$2),$(call _pos,$1,\
       $(wordlist 2,$(words $2),$2),x $3),$3)
pos = $(words $(call _pos,$1,$2))
pars = srcdir=$(srcdir)/parsers CPP="$(CPP)" \
	$(srcdir)/parsers/parse_decls.sh $(call pos,$@,$(GEN_TMP)) $< >$@ || \
	($(RM) -f $@ ; false)
$(filter %.tmp,$(GEN_TMP)): $(SRC)/proto.h
	$(pars)
plt.inc: thunk_calls.tmp
	$(pars)
plt_asmc.inc plt_asmp.inc: thunk_asms.tmp
	$(pars)

thunk_calls.h: thunk_calls.tmp parsers/thunk_gen
	parsers/thunk_gen <$< >$@ || ($(RM) -f $@ ; false)

thunk_asms.h: thunk_asms.tmp parsers/thunk_gen $(srcdir)/parsers/thunks.m4
	$(srcdir)/parsers/gen_thunks.sh $< $(srcdir)/parsers >$@ || \
		($(RM) -f $@ ; false)

glob_asmdefs.h: $(SRC)/glob_asm.h
	$(srcdir)/parsers/mkasmdefs.sh $< >$@

parsers/thunk_gen: $(srcdir)/parsers/thunk_gen.y $(srcdir)/parsers/thunk_gen.l
	if [ ! -f parsers/makefile ]; then \
	    ln -s $(abspath $(srcdir))/parsers/makefile parsers/makefile ; \
	fi
	+cd parsers && $(MAKE) srcdir=$(abspath $(srcdir))/parsers

INSTALL ?= install
install: $(ALL)
	mkdir -p $(DESTDIR)$(LIBDIR)/fdpp
	$(INSTALL) $(FDPPLIB) $(DESTDIR)$(LIBDIR)/fdpp
	mkdir -p $(DESTDIR)$(DATADIR)/fdpp
	$(INSTALL) -m 0644 $(TARGET).elf $(DESTDIR)$(DATADIR)/fdpp
	$(INSTALL) -m 0644 $(TARGET).map $(DESTDIR)$(DATADIR)/fdpp
	mkdir -p $(DESTDIR)$(PKGCONFIGDIR)
	$(INSTALL) -m 0644 fdpp.pc $(DESTDIR)$(PKGCONFIGDIR)
	mkdir -p $(DESTDIR)$(PREFIX)/include/fdpp
	$(INSTALL) -m 0644 $(EXT_H) $(DESTDIR)$(PREFIX)/include/fdpp

uninstall:
	$(RM) -rf $(DESTDIR)$(LIBDIR)/fdpp
	$(RM) -rf $(DESTDIR)$(DATADIR)/fdpp
	$(RM) -rf $(DESTDIR)$(PKGCONFIGDIR)/fdpp.pc
	$(RM) -rf $(DESTDIR)$(PREFIX)/include/fdpp

$(TGZ):
	cd $(TOP) && git archive -o $(CURDIR)/$(TAR) --prefix=$(PKG)/ HEAD
	gzip $(TAR)
.PHONY: $(TGZ)

tar: $(TGZ)

deb:
	debuild -i -us -uc -b

$(GEN_EXT): %: $(srcdir)/%.in makefile
	sed \
		-e 's!@PREFIX[@]!$(PREFIX)!g' \
		-e 's!@INCLUDEDIR[@]!$(INCLUDEDIR)!g' \
		-e 's!@LIBDIR[@]!$(LIBDIR)!g' \
		-e 's!@VERSION[@]!$(VERSION)!g' \
		$< >$@
