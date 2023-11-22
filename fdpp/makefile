T = .
-include config.mak
-include $(T)/local.mak
srcdir ?= $(CURDIR)
# the below is needed to prevent _our_ srcdir being passed
# down to rpmbuild/debuild, and, subsequently, to their make!
unexport srcdir
MAKEOVERRIDES =

include $(srcdir)/$(T)/defs.mak
include $(srcdir)/$(T)/clang.mak

TOP = $(abspath $(srcdir)/$(T)/..)
HDR = $(TOP)/hdr
SRC = $(TOP)/kernel
PPINC = $(TOP)/include/fdpp
AVER = $(shell echo FDPP_API_VER | cpp -include $(PPINC)/thunks.h - | tail -n 1)
BVER = $(shell echo BPRM_VER | cpp -include $(PPINC)/bprm.h - | tail -n 1)
FDPPDEVL = libfdpp.so
FDPPLIB = $(FDPPDEVL).$(AVER).$(BVER)
LDFLAGS += -Wl,-soname=$(FDPPLIB)
TARGET=fdppkrnl.$(AVER).$(BVER)
RELVER = 7
MAJ = 1
VERSION = $(MAJ).$(RELVER)
PKG = fdpp-$(VERSION)
TAR = $(PKG).tar
TGZ = $(TAR).gz
FD_EXT_H = $(TOP)/include/fdpp
EXT_H = $(FD_EXT_H)/thunks.h $(FD_EXT_H)/bprm.h $(FD_EXT_H)/memtype.h
GEN_EXT = fdpp.pc
TFLAGS = -a 2 -p 2

GIT_REV := $(shell $(srcdir)/git-rev.sh)
GIT_DESCRIBE := $(shell git describe)
.LOW_RESOLUTION_TIME: $(GIT_REV)

CPPFLAGS += -I . -I $(FD_EXT_H) -I $(SRC) -I $(srcdir) \
    -DFDPPKRNLDIR=$(DATADIR)/fdpp \
    -DKRNL_ELFNAME=$(TARGET).elf -DKRNL_MAP_NAME=$(TARGET).map

#               *Explicit Rules*

ALL = $(FDPPLIB) $(FDPPDEVL) $(TARGET).elf $(TARGET).map $(GEN_EXT)

all: $(ALL)

# New makes have a way to avoid parallel invocation with the use of &:
# On old make you would write "%.elf %.map :" which is a bit silly
# given the names are fixed. Also $(TARGET).% won't work that way at all:
# you need to explicitly list all files that are created simultaneously,
# and yet use pattern in all of them.
need = 4.3
ifneq ($(filter $(need),$(firstword $(sort $(MAKE_VERSION) $(need)))),)
$(TARGET).elf $(TARGET).map &:
else
%.elf %.map:
endif
	$(MAKE) -C kernel

clean:
	$(MAKE) -C kernel clean
	+cd thunk_gen && $(MAKE) srcdir=$(abspath $(srcdir))/thunk_gen clean
	-$(RM) .tstamp *.map $(TARGET).elf *.inc \
		*.o $(GEN_CC) $(FDPPDEVL) $(FDPPLIB) *.tmp $(GEN_HEADERS) \
		$(GEN_HEADERS_FD) $(GEN_ASMS) \
		*.pc *.so *.elf

HDRS = $(wildcard $(HDR)/*.h) $(wildcard $(SRC)/*.h)
PLPHDRS = farobj.hpp farptr.hpp dispatch.hpp ctors.hpp
_PPHDRS = $(PLPHDRS) dosobj.h farhlp.hpp thunks_priv.h smalloc.h \
    farhlp_sta.h
PPHDRS = $(addprefix $(srcdir)/,$(_PPHDRS))
GEN_HEADERS = thunk_calls.h thunk_asms.h plt_asmc.h plt_asmp.h
GEN_HEADERS_FD = glob_asmdefs.h
GEN_ASMS = plt.asm cdata.asm
# dont change file order in GEN_TMP as it matches the gen script
GEN_TMP = thunk_calls.tmp thunk_asms.tmp plt.inc plt_asmc.h plt_asmp.h
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
LIBELF := $(shell $(PKG_CONFIG) --libs libelf)
ifeq ($(LIBELF),)
$(error libelf not installed)
endif
LIBS += $(LIBELF)

OBJECTS = $(CFILES:.c=.o)
FDPP_COBJS = $(FDPP_CFILES:.c=.o)
FDPP_CCOBJS = $(FDPP_CCFILES:.cc=.o)
FDPP_CPPOBJS = $(CPPFILES:.cpp=.o)

$(OBJECTS): %.o: %.cc $(HEADERS) $(GEN_HEADERS_FD) $(EXT_H)
	$(CC) $(CFLAGS) -o $@ $<

$(FDPP_COBJS): %.o: $(srcdir)/%.c $(PPHDRS) $(srcdir)/makefile
	$(CC) $(CLCFLAGS) -I . -o $@ $<

thunks.o: $(EXT_H) $(GIT_REV)
thunks_a.o: $(SRC)/glob_asm.h $(srcdir)/glob_tmpl.h plt_asmc.h plt_asmp.h
$(FDPP_CCOBJS): %.o: $(srcdir)/%.cc $(GEN_HEADERS) $(PPHDRS) $(srcdir)/makefile
	$(CC) $(CFLAGS) -o $@ $<

$(FDPP_CPPOBJS): %.o: $(srcdir)/%.cpp $(PPHDRS) $(srcdir)/makefile
	$(CC) $(CFLAGS) -o $@ $<

$(GEN_CC): %.cc: $(SRC)/%.c makefile
	$(srcdir)/parsers/mkfar.sh $< >$@

$(FDPPLIB): $(OBJECTS) $(FDPP_COBJS) $(FDPP_CCOBJS) $(FDPP_CPPOBJS)
	$(CC_LD) -o $@ $^ $(LDFLAGS) $(LIBS)
	@echo "Have `nm -u $@ | grep "U " | wc -l` undefined symbols"

$(FDPPDEVL): $(FDPPLIB)
	ln -sf $< $@

_pos = $(if $(findstring $1,$2),$(call _pos,$1,\
       $(wordlist 2,$(words $2),$2),x $3),$3)
pos = $(words $(call _pos,$1,$2))
pars = srcdir=$(srcdir)/parsers CPP="$(CPP)" \
	$(srcdir)/parsers/parse_decls.sh $(call pos,$@,$(GEN_TMP)) $< >$@ || \
	($(RM) $@ ; false)
$(filter %.tmp,$(GEN_TMP)): $(SRC)/proto.h
	$(pars)
plt_asmc.h plt_asmp.h: thunk_asms.tmp
	$(pars)

thunk_calls.h: thunk_calls.tmp thunk_gen/thunk_gen
	thunk_gen/thunk_gen $(TFLAGS) <$< >$@ || ($(RM) $@ ; false)

thunk_asms.h: thunk_asms.tmp thunk_gen/thunk_gen $(srcdir)/parsers/thunks.m4
	thunk_gen/thunk_gen $(TFLAGS) 1 <$< | sort | uniq | autom4te -l m4sugar $(srcdir)/parsers/thunks.m4 - >$@_ \
		|| ($(RM) $@_ ; false)
	thunk_gen/thunk_gen $(TFLAGS) 2 <$< >$@__ \
		|| ($(RM) $@__ ; false)
	cat $@_ $@__ >$@
	rm -f $@_ $@__

glob_asmdefs.h: $(SRC)/glob_asm.h
	$(srcdir)/parsers/mkasmdefs.sh $< >$@

thunk_gen/thunk_gen: $(srcdir)/thunk_gen/thunk_gen.y $(srcdir)/thunk_gen/thunk_gen.l
	+cd thunk_gen && $(MAKE) srcdir=$(abspath $(srcdir))/thunk_gen

INSTALL ?= install
install: $(ALL)
	$(INSTALL) -d $(DESTDIR)$(LIBDIR)/fdpp
	$(INSTALL) $(FDPPLIB) $(DESTDIR)$(LIBDIR)/fdpp
	cp -fP $(FDPPDEVL) $(DESTDIR)$(LIBDIR)/fdpp
	$(INSTALL) -d $(DESTDIR)$(DATADIR)/fdpp
	$(INSTALL) -m 0644 $(TARGET).elf $(DESTDIR)$(DATADIR)/fdpp
	$(INSTALL) -m 0644 $(TARGET).map $(DESTDIR)$(DATADIR)/fdpp
	$(INSTALL) -d $(DESTDIR)$(PKGCONFIGDIR)
	$(INSTALL) -m 0644 fdpp.pc $(DESTDIR)$(PKGCONFIGDIR)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/include/fdpp
	$(INSTALL) -m 0644 $(EXT_H) $(DESTDIR)$(PREFIX)/include/fdpp

uninstall:
	$(RM) -r $(DESTDIR)$(LIBDIR)/fdpp
	$(RM) -r $(DESTDIR)$(DATADIR)/fdpp
	$(RM) -r $(DESTDIR)$(PKGCONFIGDIR)/fdpp.pc
	$(RM) -r $(DESTDIR)$(PREFIX)/include/fdpp

$(TGZ):
	cd $(TOP) && git archive -o $(TOP)/$(TAR) --prefix=$(PKG)/ HEAD
	gzip $(TOP)/$(TAR)
.PHONY: $(TGZ)

tar: $(TGZ)

$(GEN_EXT): %: $(srcdir)/%.in makefile
	sed \
		-e 's!@PREFIX[@]!$(PREFIX)!g' \
		-e 's!@INCLUDEDIR[@]!$(INCLUDEDIR)!g' \
		-e 's!@LIBDIR[@]!$(LIBDIR)!g' \
		-e 's!@VERSION[@]!$(VERSION)!g' \
		$< >$@
