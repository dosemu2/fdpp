T = ..
-include config.mak
-include $(T)/local.mak
srcdir ?= $(CURDIR)

# if exported from top-level project, then don't include defaults
ifeq ($(PKG_CONFIG),)
include $(srcdir)/$(T)/clang.mak
include $(srcdir)/$(T)/defs.mak
endif

TOP = $(abspath $(srcdir)/$(T)/..)
HDR = $(TOP)/hdr
PPINC = $(TOP)/include/fdpp
AVER = $(shell echo FDPP_API_VER | cpp -include $(PPINC)/thunks.h - | tail -n 1)
BVER = $(shell echo BPRM_VER | cpp -include $(PPINC)/bprm.h - | tail -n 1)
TARGET=fdppkrnl.$(AVER).$(BVER)
FDPPDEVL = libfdldr.so
FDPPLIB = $(FDPPDEVL).$(AVER).$(BVER)
LDFLAGS += -Wl,-soname=$(FDPPLIB) -L..
SOURCES = elf.c loader.c
HEADERS = elf_priv.h
OBJECTS = $(SOURCES:.c=.o)

CFLAGS += -I . -I $(PPINC) \
    -DFDPPKRNLDIR=$(datadir)/fdpp \
    -DKRNL_ELFNAME=$(TARGET).elf -DKRNL_MAP_NAME=$(TARGET).map \
    -fpic
ifeq ($(filter clean install,$(MAKECMDGOALS)),)
LIBELF := $(shell $(PKG_CONFIG) --libs libelf)
ifeq ($(LIBELF),)
$(error libelf not installed)
endif
LIBS += $(LIBELF)
endif

ALL = $(T)/$(FDPPLIB)

all: $(ALL)

$(T)/$(FDPPLIB): $(OBJECTS)
	$(CC_LD) -o $@ $^ $(LDFLAGS) $(LIBS)

$(OBJECTS): %.o: $(srcdir)/%.c $(addprefix $(srcdir)/,$(HEADERS)) $(srcdir)/makefile
	$(CC) $(CFLAGS) $(shell $(PKG_CONFIG) --cflags libelf) -c -o $@ $<

clean:
	$(RM) *.o
