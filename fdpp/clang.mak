#
# CLANG.MAK - kernel copiler options for clang
#

DIRSEP=/
RM=rm -f
CP=cp
CC=clang++ -std=c++11
CL=clang++

TARGETOPT = -c -fno-threadsafe-statics -fno-rtti -fpic \
    -Wno-format-invalid-specifier
ifneq ($(XCPU),386)
$(error unsupported CPU 186)
endif

TARGET=fdppkrnl

DEBUG_MODE = 1
EXTRA_DEBUG = 0
DEBUG_MSGS = 0
USE_UBSAN = 0
ALLCFLAGS += -iquote $(srcdir)/../hdr -Wall $(TARGETOPT) -Wmissing-prototypes
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

CFLAGS=$(ALLCFLAGS)
LDFLAGS=-shared -Wl,-Bsymbolic -Wl,--build-id=sha1 -static-libgcc
CLC = clang

ALLCFLAGS:=$(ALLCFLAGS) -DI386

ifeq ($(XFAT),32)
ALLCFLAGS:=$(ALLCFLAGS) -DWITHFAT32
NASMFLAGS:=$(NASMFLAGS) -DWITHFAT32
endif

NASM=$(XNASM)
NASMFLAGS   := $(NASMFLAGS) -i$(srcdir)/../hdr/ -DXCPU=$(XCPU)
LOADSEG = 0x60

LINK=$(XLINK)
