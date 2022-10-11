#
# CLANG.MAK - kernel copiler options for clang
#

CCACHE = $(shell which ccache)
CC = $(CCACHE) clang++
# Override builtin CXX.
# The assignment below is ignored if CXX was set via cmd line.
CXX=
ifeq ($(CXX),)
CL = $(CCACHE) clang++
else
CL = $(CXX)
endif
CC_FOR_BUILD = $(CCACHE) clang
NASM ?= nasm
PKG_CONFIG ?= pkg-config

TARGETOPT = -std=c++11 -c -fno-threadsafe-statics -fpic
# _XTRA should go at the end of cmd line
TARGETOPT_XTRA = -Wno-format-invalid-specifier

TARGET=fdppkrnl

DEBUG_MODE ?= 1
EXTRA_DEBUG ?= 0
DEBUG_MSGS ?= 1
USE_ASAN ?= 0
USE_UBSAN ?= 0

IFLAGS = -iquote $(srcdir)/../hdr
CPPFLAGS = $(IFLAGS) -DFDPP
WFLAGS = -Wall
WCFLAGS = $(WFLAGS)
ifeq ($(DEBUG_MODE),1)
DBGFLAGS += -ggdb3
endif
ifeq ($(EXTRA_DEBUG),1)
DBGFLAGS += -O0
CPPFLAGS += -DFDPP_DEBUG -DEXTRA_DEBUG
NASMFLAGS += -DEXTRA_DEBUG
else
DBGFLAGS += -O2
endif
ifeq ($(DEBUG_MSGS),1)
CPPFLAGS += -DDEBUG
endif
ifeq ($(USE_ASAN),1)
DBGFLAGS += -fsanitize=address
endif
ifeq ($(USE_UBSAN),1)
DBGFLAGS += -fsanitize=undefined -fno-sanitize=alignment,function,vptr
endif

CFLAGS = $(TARGETOPT) $(CPPFLAGS) $(WFLAGS) $(DBGFLAGS) $(TARGETOPT_XTRA)
CLCFLAGS = -c -fpic $(WCFLAGS) $(DBGFLAGS) -xc++
LDFLAGS = -shared -Wl,-Bsymbolic -Wl,--build-id=sha1

ifeq ($(XFAT),32)
CPPFLAGS += -DWITHFAT32
NASMFLAGS += -DWITHFAT32
endif

NASMFLAGS := $(NASMFLAGS) -i$(srcdir)/../hdr/ -DXCPU=$(XCPU) -DFDPP
