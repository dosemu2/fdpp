# XCPU=86
# the only difference between 186 and 386 is that for 386
# the high word of 32bit registers will be saved/restored.
# fdpp does not need this because all 32/64bit code is executed
# in protected mode and does not affect real-mode registers.
XCPU=186
# XCPU=386

# XFAT=16
XFAT=32

PREFIX ?= /usr/local
LIBDIR ?= $(PREFIX)/lib
INCLUDEDIR ?= $(PREFIX)/include
PKGCONFIGDIR ?= $(LIBDIR)/pkgconfig

DIRSEP = /
RM = rm -f
SHELL = /usr/bin/env bash
