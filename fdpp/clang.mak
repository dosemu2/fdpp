#
# CLANG.MAK - kernel copiler options for clang
#

LLD ?= $(shell which ld.lld 2>/dev/null)
CCACHE ?= $(shell which ccache 2>/dev/null)
CC = $(CCACHE) clang++
CLANG_VER := $(shell $(CC) -v 2>&1 | head -n 1 | sed -E 's/.+ (.+)\..+\..+/\1/')
# below requires make-4.4, uncomment when it is widely available
#CC += $(intcmp 15,$(CLANG_VER),-fclang-abi-compat=15)
ifeq ($(CLANG_VER),16)
CC += -fclang-abi-compat=15
endif

# Override builtin CXX.
# The assignment below is ignored if CXX was set via cmd line.
CXX=
ifeq ($(CXX),)
CL = $(CC)
else
CL = $(CXX)
endif
CC_FOR_BUILD = $(CCACHE) clang
CPP = $(CC_FOR_BUILD) -E
CC_LD = $(CL)
ifneq ($(LLD),)
# ld.lld can cross-compile while gnu ld not
CROSS_LD ?= $(LLD)
else
CROSS_LD ?= ld
endif
NASM ?= nasm
PKG_CONFIG ?= pkg-config

TARGETOPT = -std=c++11 -c -fno-threadsafe-statics -fpic
# _XTRA should go at the end of cmd line
TARGETOPT_XTRA = -Wno-format-invalid-specifier

DEBUG_MODE ?= 1
EXTRA_DEBUG ?= 0
DEBUG_MSGS ?= 1
USE_ASAN ?= 0
USE_UBSAN ?= 0

IFLAGS = -iquote $(srcdir)/../hdr
CPPFLAGS = $(IFLAGS) -DFDPP
WFLAGS = -Wall -Wpacked -Werror=packed-non-pod -Wno-unknown-warning-option
WCFLAGS = $(WFLAGS)
ifeq ($(DEBUG_MODE),1)
DBGFLAGS += -ggdb3
endif
ifeq ($(EXTRA_DEBUG),1)
DBGFLAGS += -O0 -fdebug-macro
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
