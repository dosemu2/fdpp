#
# CLANG.MAK - kernel copiler options for clang
#

CC = clang++
CL = clang++
CLC = clang
NASM = nasm
LINK = ld

TARGETOPT = -std=c++11 -c -fno-threadsafe-statics -fno-rtti -fpic \
    -Wno-format-invalid-specifier
ifneq ($(XCPU),386)
$(error unsupported CPU 186)
endif

TARGET=fdppkrnl

DEBUG_MODE = 1
EXTRA_DEBUG = 0
DEBUG_MSGS = 0
USE_UBSAN = 0
CPPFLAGS = -iquote $(srcdir)/../hdr
WFLAGS = -Wall -Wmissing-prototypes
ALLCFLAGS += $(TARGETOPT) $(CPPFLAGS) $(WFLAGS)
ifeq ($(DEBUG_MODE),1)
ALLCFLAGS += -ggdb3
endif
ifeq ($(EXTRA_DEBUG),1)
ALLCFLAGS += -fdebug-macro -O0
else
ALLCFLAGS += -O2
endif
ifeq ($(DEBUG_MSGS),1)
ALLCFLAGS += -DDEBUG
endif
ifeq ($(USE_UBSAN),1)
ALLCFLAGS += -fsanitize=undefined -fno-sanitize=alignment
endif

CFLAGS = $(ALLCFLAGS)
LDFLAGS = -shared -Wl,-Bsymbolic -Wl,--build-id=sha1
CLCFLAGS = -c -fpic $(CPPFLAGS) $(WFLAGS)

ALLCFLAGS:=$(ALLCFLAGS) -DI386

ifeq ($(XFAT),32)
ALLCFLAGS:=$(ALLCFLAGS) -DWITHFAT32
NASMFLAGS:=$(NASMFLAGS) -DWITHFAT32
endif

NASMFLAGS := $(NASMFLAGS) -i$(srcdir)/../hdr/ -DXCPU=$(XCPU)
LOADSEG = 0x60
