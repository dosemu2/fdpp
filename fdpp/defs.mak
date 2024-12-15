# XCPU=86
# the only difference between 186 and 386 is that for 386
# the high word of 32bit registers will be saved/restored.
# fdpp does not need this because all 32/64bit code is executed
# in protected mode and does not affect real-mode registers.
XCPU=186
# XCPU=386

# XFAT=16
XFAT=32

prefix ?= /usr/local
LIBDIR ?= $(prefix)/lib
DATADIR ?= $(prefix)/share
INCLUDEDIR ?= $(prefix)/include
pkgconfdir ?= $(LIBDIR)/pkgconfig
export PKG_CONFIG_PATH = $(pkgconfdir):$(DATADIR)/pkgconfig
# for nasm-segelf
export PATH := $(PATH):$(prefix)/bin

DIRSEP = /
RM = rm -f
LN = ln -f
SHELL = /usr/bin/env bash
