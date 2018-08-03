#
# CLANG.MAK - kernel copiler options for clang
#

DIRSEP=/
RM=rm -f
CP=cp
CC=clang++ -std=c++11
CL=clang++

TARGETOPT=-c -fno-unwind-tables -fno-asynchronous-unwind-tables \
    -fno-exceptions -fno-threadsafe-statics -fno-rtti -Wno-inline-new-delete
ifneq ($(XCPU),386)
$(error unsupported CPU 186)
endif

TARGET=fdppkrnl

EXTRA_DEBUG = 0
ALLCFLAGS += -iquote $(srcdir)/../hdr $(TARGETOPT) -Wall -fpic \
    -fno-strict-aliasing -Wno-format-invalid-specifier
ifeq ($(EXTRA_DEBUG),1)
ALLCFLAGS += -ggdb3 -fdebug-macro -O0
else
ALLCFLAGS += -ggdb3 -O2
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
