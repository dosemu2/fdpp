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

ALLCFLAGS += -iquote ../hdr $(TARGETOPT) -Wall -fpic -O2 \
    -ggdb3 -fdebug-macro -fno-strict-aliasing -Wno-format-invalid-specifier

CFLAGS=$(ALLCFLAGS)
LDFLAGS=-shared -Wl,-Bsymbolic
CLC = clang

ALLCFLAGS:=$(ALLCFLAGS) -DI386

ifeq ($(XFAT),32)
ALLCFLAGS:=$(ALLCFLAGS) -DWITHFAT32
NASMFLAGS:=$(NASMFLAGS) -DWITHFAT32
endif

NASM=$(XNASM)
NASMFLAGS   := $(NASMFLAGS) -i../hdr/ -DXCPU=$(XCPU)
LOADSEG = 0x60

LINK=$(XLINK)
