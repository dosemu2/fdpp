#
# CLANG.MAK - kernel copiler options for clang
#

CROSS_LD ?= x86_64-linux-gnu-ld
ifeq ($(shell $(CROSS_LD) --version 2>/dev/null),)
ifneq ($(filter x86_64 amd64 i686,$(shell uname -m)),)
CROSS_LD = ld.bfd
else
$(error binutils-x86-64-linux-gnu not installed)
endif
endif
LD_VER = $(shell $(CROSS_LD) --version 2>/dev/null)
ifneq ($(filter LLD,$(LD_VER)),)
$(warning lld found, producing unrelocatable build)
want_loadaddr = 0x900
NASM = nasm
NASMFLAGS += -DFDPP_STATIC
endif

# don't use ?= here as that doesn't override make's builtin CC var
CC = clang
CXX = clang++
CLANG_VER := $(shell $(CXX) --version 2>&1 | head -n 1 | grep clang | \
  sed -E 's/.+ version ([^.]+)\.[^.]+\.[^ ]+.*/\1/')
ifeq ($(CLANG_VER),)
GCC_VER := $(shell $(CXX) --version 2>&1 | head -n 1 | grep g++)
ifeq ($(GCC_VER),)
$(error unknown compiler $(CXX) $(shell $(CXX) --version))
endif
endif
FLEX = $(shell which flex 2>/dev/null)
ifneq ($(FLEX),)
LEX = $(FLEX)
endif

CC_FOR_BUILD ?= $(CC)
CPP = $(CC_FOR_BUILD) -E
ifeq ($(CXX_LD),)
CXX_LD = $(CXX)
endif
ifeq ($(CC_LD),)
CC_LD = $(CC)
endif
NASM ?= nasm-segelf
PKG_CONFIG ?= pkg-config

# export vars needed for loader sub-project
export CC
export CC_FOR_BUILD
export CC_LD
export PKG_CONFIG

TARGETOPT = -std=c++20 -c -fno-threadsafe-statics -fpic
# _XTRA should go at the end of cmd line
ifeq ($(GCC_VER),)
TARGETOPT += -DCLANG_VER=$(CLANG_VER)
TARGETOPT_XTRA = -Wno-format-invalid-specifier
else
TARGETOPT_XTRA = -Wno-error=format-security
endif

DEBUG_MODE ?= 1
EXTRA_DEBUG ?= 0
DEBUG_MSGS ?= 1
USE_ASAN ?= 0
USE_UBSAN ?= 0

IFLAGS = -iquote $(srcdir)/../hdr
CPPFLAGS += $(IFLAGS) -DFDPP
WFLAGS := -Wall -Wsign-compare
ifeq ($(GCC_VER),)
WFLAGS += -Werror=packed-non-pod -Wno-unknown-warning-option
ifneq ($(CLANG_VER),16)
WFLAGS += -Wpacked
endif
else
WFLAGS += -Wno-format -Wno-format-extra-args
endif
WFLAGS += -Wno-address-of-packed-member
WCFLAGS = $(WFLAGS)
ifeq ($(DEBUG_MODE),1)
DBGFLAGS += -ggdb3
endif
ifeq ($(EXTRA_DEBUG),1)
DBGFLAGS += -O0
ifeq ($(GCC_VER),)
DBGFLAGS += -fdebug-macro
endif
CPPFLAGS += -DFDPP_DEBUG -DEXTRA_DEBUG
NASMFLAGS += -DEXTRA_DEBUG
else
DBGFLAGS += -O2 -flto=auto
LDFLAGS += -O2 -flto=auto
endif
ifeq ($(DEBUG_MSGS),1)
CPPFLAGS += -DDEBUG
endif
ifeq ($(USE_ASAN),1)
DBGFLAGS += -fsanitize=address
endif
ifeq ($(USE_UBSAN),1)
DBGFLAGS += -fsanitize=undefined -fno-sanitize=alignment,vptr
ifeq ($(GCC_VER),)
DBGFLAGS += -fno-sanitize=function
endif
endif

CXXFLAGS += $(TARGETOPT) $(CPPFLAGS) $(WFLAGS) $(DBGFLAGS) $(TARGETOPT_XTRA)
CFLAGS += -Wall $(DBGFLAGS)
CLCFLAGS = -c -fpic -Wall $(DBGFLAGS) -xc++
LDFLAGS += -shared -Wl,--build-id=sha1

ifeq ($(XFAT),32)
CPPFLAGS += -DWITHFAT32
NASMFLAGS += -DWITHFAT32
endif
ifeq ($(XCPU),386)
CPPFLAGS += -DI386
endif

NASMFLAGS += -i$(srcdir)/../hdr/ -DXCPU=$(XCPU) -DFDPP
