# These are generic definitions

#**********************************************************************
#* TARGET    : we create a %TARGET%.sys file
#* TARGETOPT : options, handled down to the compiler
#**********************************************************************

TARGETOPT=-1-

ifeq ($(XCPU),186)
TARGETOPT=-1
endif
ifeq ($(XCPU),386)
TARGETOPT=-3
endif

ifeq ($(XFAT),32)
ALLCFLAGS:=$(ALLCFLAGS) -DWITHFAT32
NASMFLAGS:=$(NASMFLAGS) -DWITHFAT32
endif

NASM=$(XNASM)
NASMFLAGS   := $(NASMFLAGS) -i../hdr/ -DXCPU=$(XCPU)

LINK=$(XLINK)

INITPATCH=@rem
DIRSEP=\ #a backslash
RM=..\utils\rmfiles
CP=copy
ECHOTO=..\utils\echoto
CLDEF=0

ifeq ($(LOADSEG)0,0)
LOADSEG=0x60
endif

include ../mkfiles/$(COMPILER).mak

ifeq ($(CLDEF),0)
CLT=$(CL) $(CFLAGST) $(TINY) -I$(INCLUDEPATH)
CLC=$(CL) $(CFLAGSC) -I$(INCLUDEPATH)
endif

TARGET:=$(TARGET)$(XCPU)$(XFAT)

.SUFFIXES: .c .cpp .obj .asm .o
.asm.obj :
	$(NASM) -D$(COMPILER) $(NASMFLAGS) -f obj $<
