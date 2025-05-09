T = .
-include config.mak
-include $(T)/local.mak
srcdir ?= $(CURDIR)
# the below is needed to prevent _our_ srcdir being passed
# down to rpmbuild/debuild, and, subsequently, to their make!
unexport srcdir
MAKEOVERRIDES =

include $(srcdir)/$(T)/defs.mak
ifeq ($(filter clean install,$(MAKECMDGOALS)),)
include $(srcdir)/$(T)/clang.mak
endif

TOP = $(abspath $(srcdir)/$(T)/..)
HDR = $(TOP)/hdr
SRC = $(TOP)/kernel
PPINC = $(TOP)/include/fdpp
AVER = $(shell echo FDPP_API_VER | cpp -include $(PPINC)/thunks.h - | tail -n 1)
BVER = $(shell echo BPRM_VER | cpp -include $(PPINC)/bprm.h - | tail -n 1)
FDPPDEVL = libfdpp.so
FDPPLIB = $(FDPPDEVL).$(AVER).$(BVER)
LDRDEVL = libfdldr.so
LDRLIB = $(LDRDEVL).$(AVER).$(BVER)
LDFLAGS += -Wl,-soname=$(FDPPLIB) -Wl,-Bsymbolic
TARGET=fdppkrnl.$(AVER).$(BVER)
MAJ = 1
RELVER = 8
VERSION = $(MAJ).$(RELVER)
PKG = fdpp-$(VERSION)
TAR = $(PKG).tar
TGZ = $(TAR).gz
FD_EXT_H = $(TOP)/include/fdpp
EXT_H = $(FD_EXT_H)/thunks.h $(FD_EXT_H)/bprm.h $(FD_EXT_H)/memtype.h \
     $(FD_EXT_H)/loader.h
GEN_EXT = fdpp.pc

GIT_REV := $(shell $(srcdir)/git-rev.sh)
GIT_DESCRIBE := $(shell git describe)
.LOW_RESOLUTION_TIME: $(GIT_REV)

CPPFLAGS += -I . -I $(FD_EXT_H) -I $(SRC) -I $(srcdir)

#               *Explicit Rules*

ALL = $(FDPPLIB) $(FDPPDEVL) $(LDRLIB) $(LDRDEVL) $(TARGET).elf $(TARGET).map \
    $(GEN_EXT)

all: $(ALL)

export CFLAGS
export LDFLAGS

ifeq ($(filter clean install,$(MAKECMDGOALS)),)
# New makes have a way to avoid parallel invocation with the use of &:
# On old make you would write "%.elf %.map :" which is a bit silly
# given the names are fixed. Also $(TARGET).% won't work that way at all:
# you need to explicitly list all files that are created simultaneously,
# and yet use pattern in all of them.
need = 4.3
ifneq ($(filter $(need),$(firstword $(sort $(MAKE_VERSION) $(need)))),)
$(TARGET).elf $(TARGET).map &:
else
$(TARGET).map: | $(TARGET).elf
$(TARGET).elf:
endif
	$(MAKE) -C kernel

$(LDRLIB):
	$(MAKE) -C loader
endif

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C loader clean
	-$(RM) .tstamp *.map $(TARGET).elf *.inc \
		*.o $(GEN_CC) $(FDPPDEVL) $(FDPPLIB) *.tmp $(GEN_HEADERS) \
		$(LDRDEVL) $(LDRLIB) $(GEN_HEADERS_FD) \
		*.pc *.so *.elf

HDRS = $(wildcard $(HDR)/*.h) $(wildcard $(SRC)/*.h)
PLPHDRS = farobj.hpp farptr.hpp dispatch.hpp ctors.hpp
_PPHDRS = $(PLPHDRS) dosobj.h farhlp.hpp thunks_priv.h smalloc.h
PPHDRS = $(addprefix $(srcdir)/,$(_PPHDRS))
GEN_HEADERS = thunk_calls.h thunk_asms.h plt_asmc.h plt_asmp.h
GEN_HEADERS_FD = glob_asmdefs.h
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

FDPP_CFILES = smalloc.c
FDPP_CCFILES = thunks.cc thunks_c.cc thunks_a.cc thunks_p.cc dosobj.cc
CPPFILES = objhlp.cpp ctors.cpp farhlp.cpp objtrace.cpp

OBJECTS = $(CFILES:.c=.o)
FDPP_COBJS = $(FDPP_CFILES:.c=.o)
FDPP_CCOBJS = $(FDPP_CCFILES:.cc=.o)
FDPP_CPPOBJS = $(CPPFILES:.cpp=.o)

$(OBJECTS): %.o: %.cc $(HEADERS) $(GEN_HEADERS_FD) $(EXT_H)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(FDPP_COBJS): %.o: $(srcdir)/%.c $(PPHDRS) $(srcdir)/makefile
	$(CXX) $(CLCFLAGS) -I . -o $@ $<

thunks.o: $(EXT_H) $(GIT_REV)
thunks_a.o: $(SRC)/glob_asm.h $(srcdir)/glob_tmpl.h plt_asmc.h plt_asmp.h
$(FDPP_CCOBJS): %.o: $(srcdir)/%.cc $(GEN_HEADERS) $(PPHDRS) $(srcdir)/makefile
	$(CXX) $(CXXFLAGS) -o $@ $<

$(FDPP_CPPOBJS): %.o: $(srcdir)/%.cpp $(PPHDRS) $(srcdir)/makefile
	$(CXX) $(CXXFLAGS) -o $@ $<

$(GEN_CC): %.cc: $(SRC)/%.c makefile
	$(srcdir)/parsers/mkfar.sh $< >$@

$(FDPPLIB): $(OBJECTS) $(FDPP_COBJS) $(FDPP_CCOBJS) $(FDPP_CPPOBJS)
	$(CXX_LD) -o $@ $^ $(LDFLAGS) $(LIBS)
	@echo "Have `nm -u $@ | grep "U " | wc -l` undefined symbols"

$(FDPPDEVL): | $(FDPPLIB)
	ln -sf $(FDPPLIB) $@

$(LDRDEVL): | $(LDRLIB)
	ln -sf $(LDRLIB) $@

# we have custom PDS
PDS = srcdir=$(srcdir)/parsers CPP="$(CPP)" $(srcdir)/parsers/parse_decls.sh
TGMK = $(shell pkg-config --variable=makeinc thunk_gen)
ifeq ($(wildcard $(TGMK)),)
ifeq ($(filter clean,$(MAKECMDGOALS)),)
$(error thunk_gen not installed)
endif
else
ifeq ($(filter clean,$(MAKECMDGOALS)),)
$(shell pkg-config --atleast-version=1.2 thunk_gen)
ifneq ($(.SHELLSTATUS),0)
$(error thunk_gen is too old, 1.2 is needed)
endif
endif
TFLAGS = -a 2 -p 2
PDHDR = $(SRC)/proto.h
GLOB_ASM = $(SRC)/glob_asm.h
include $(TGMK)
endif

INSTALL ?= install
install: $(ALL)
	$(INSTALL) -d $(DESTDIR)$(libdir)/fdpp
	$(INSTALL) $(FDPPLIB) $(DESTDIR)$(libdir)/fdpp
	cp -fP $(FDPPDEVL) $(DESTDIR)$(libdir)/fdpp
	$(INSTALL) $(LDRLIB) $(DESTDIR)$(libdir)/fdpp
	cp -fP $(LDRDEVL) $(DESTDIR)$(libdir)/fdpp
	$(INSTALL) -d $(DESTDIR)$(datadir)/fdpp
	$(INSTALL) -m 0644 $(TARGET).elf $(DESTDIR)$(datadir)/fdpp
	$(INSTALL) -m 0644 $(TARGET).map $(DESTDIR)$(datadir)/fdpp
	$(INSTALL) -d $(DESTDIR)$(pkgconfdir)
	$(INSTALL) -m 0644 fdpp.pc $(DESTDIR)$(pkgconfdir)
	$(INSTALL) -d $(DESTDIR)$(prefix)/include/fdpp
	$(INSTALL) -m 0644 $(EXT_H) $(DESTDIR)$(prefix)/include/fdpp

uninstall:
	$(RM) -r $(DESTDIR)$(libdir)/fdpp
	$(RM) -r $(DESTDIR)$(datadir)/fdpp
	$(RM) -r $(DESTDIR)$(pkgconfdir)/fdpp.pc
	$(RM) -r $(DESTDIR)$(prefix)/include/fdpp

$(TGZ):
	cd $(TOP) && git archive -o $(TOP)/$(TAR) --prefix=$(PKG)/ HEAD
	gzip $(TOP)/$(TAR)
.PHONY: $(TGZ) $(TARGET).elf $(LDRLIB)

tar: $(TGZ)

$(GEN_EXT): %: $(srcdir)/%.in makefile
	sed \
		-e 's!@PREFIX[@]!$(prefix)!g' \
		-e 's!@INCLUDEDIR[@]!$(includedir)!g' \
		-e 's!@LIBDIR[@]!$(libdir)!g' \
		-e 's!@VERSION[@]!$(VERSION)!g' \
		$< >$@
