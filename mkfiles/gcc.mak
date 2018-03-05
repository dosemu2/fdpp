#
# GCC.MAK - kernel copiler options for gcc
#

DIRSEP=/
RM=rm -f
CP=cp
ECHOTO=../utils/echoto
CC=clang++
CL=clang

TARGETOPT=-c -std=c++11 -fno-unwind-tables -fno-asynchronous-unwind-tables \
    -fno-exceptions -fno-threadsafe-statics -fno-rtti -Wno-inline-new-delete
ifneq ($(XCPU),386)
$(error unsupported CPU 186)
endif

TARGET=KMS

ALLCFLAGS += -iquote ../hdr $(TARGETOPT) -Wall -fpic -O2 \
    -ggdb3 -fno-strict-aliasing -Wno-format-invalid-specifier

INITCFLAGS=$(ALLCFLAGS)
CFLAGS=$(ALLCFLAGS)
LDFLAGS=-shared -Wl,-Bsymbolic
CLDEF = 1
CLC = clang
CLT = clang
INITPATCH=@echo > /dev/null
