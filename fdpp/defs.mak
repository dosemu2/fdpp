# XCPU=86
# the only difference between 186 and 386 is that for 386
# the high word of 32bit registers will be saved/restored.
# fdpp does not need this because all 32/64bit code is executed
# in protected mode and does not affect real-mode registers.
XCPU=186
# XCPU=386

# XFAT=16
XFAT=32

# on Termux PREFIX is used
ifneq ($(PREFIX),)
prefix := $(PREFIX)
else
prefix ?= /usr/local
endif
libdir ?= $(prefix)/lib
datadir ?= $(prefix)/share
includedir ?= $(prefix)/include
pkgconfdir ?= $(libdir)/pkgconfig
export PKG_CONFIG_PATH := $(PKG_CONFIG_PATH):$(pkgconfdir):$(datadir)/pkgconfig
# for nasm-segelf
export PATH := $(PATH):$(prefix)/bin
# for loader
export datadir

DIRSEP = /
RM = rm -f
LN = ln -f
SHELL = /usr/bin/env bash
