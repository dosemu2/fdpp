#
# GCC.MAK - kernel copiler options for gcc
#

DIRSEP=/
RM=rm -f
CP=cp
ECHOTO=../utils/echoto
CC=gcc -c
CL=gcc
LIBUTIL=ar cr

TARGETOPT=
ifneq ($(XCPU),386)
$(error unsupported CPU 186)
endif

TARGET=KMS

ALLCFLAGS:=-I../hdr $(TARGETOPT) $(ALLCFLAGS) -Wall

INITCFLAGS=$(ALLCFLAGS)
CFLAGS=$(ALLCFLAGS)
CLDEF = 1
CLC = gcc
CLT = gcc
INITPATCH=@echo > /dev/null
