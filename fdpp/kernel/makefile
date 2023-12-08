T = ..
-include config.mak
-include $(T)/local.mak
srcdir ?= $(CURDIR)

include $(srcdir)/$(T)/defs.mak
include $(srcdir)/$(T)/clang.mak

TOP = $(abspath $(srcdir)/$(T)/..)
HDR = $(TOP)/hdr
SRC = $(TOP)/kernel
PPINC = $(TOP)/include/fdpp
AVER = $(shell echo FDPP_API_VER | cpp -include $(PPINC)/thunks.h - | tail -n 1)
BVER = $(shell echo BPRM_VER | cpp -include $(PPINC)/bprm.h - | tail -n 1)
TARGET=fdppkrnl.$(AVER).$(BVER)
VERSION = 1.7
CPPFLAGS += -I$(TOP)/fdpp

NASMFLAGS += -i$(SRC) -i$(HDR) -f elf32

ASMS = kernel.asm entry.asm io.asm console.asm serial.asm printer.asm \
    execrh.asm nlssupt.asm procsupt.asm dosidle.asm int2f.asm \
    nls_hc.asm intr.asm irqstack.asm \
    cpu.asm
DASMS = floppy.asm rdpcclk.asm wrpcclk.asm wratclk.asm
OBJS = $(ASMS:.asm=.o)
DOBJS = $(DASMS:.asm=.o)
PPASMS = plt.S cdata.S
PPOBJS = $(PPASMS:.S=.o)

#               *Explicit Rules*

ALL = $(T)/$(TARGET).elf $(T)/$(TARGET).map

all: $(ALL)

want_loadaddr = 0
ifeq ($(want_loadaddr),1)
LA = --defsym=loadaddr=0xfa100
else
LA = --emit-relocs
endif
want_pie = 0
ifeq ($(want_pie),1)
PF = -pie
endif

# New makes have a way to avoid parallel invocation with the use of &:
# On old make you would write "%.elf %.map :" which is a bit silly
# given the names are fixed. Also $(TARGET).% won't work that way at all:
# you need to explicitly list all files that are created simultaneously,
# and yet use pattern in all of them.
DEPS_OF_ELF = $(srcdir)/kernel.ld $(OBJS) $(PPOBJS) $(DOBJS)
need = 4.3
ifneq ($(filter $(need),$(firstword $(sort $(MAKE_VERSION) $(need)))),)
$(T)/$(TARGET).elf $(T)/$(TARGET).map &: $(DEPS_OF_ELF)
else
$(T)/%.elf $(T)/%.map : $(DEPS_OF_ELF)
endif
	$(CROSS_LD) -melf_i386 -static -Map=$(T)/$(TARGET).map -o $(T)/$(TARGET).elf \
		$(LA) $(PF) $(^:%.ld=-T%.ld)
	chmod -x $(T)/$(TARGET).elf

clean:
	-$(RM) .tstamp *.inc *.o *.tmp $(GEN_ASMS)

#               *Individual File Dependencies*
#apisupt.o:	$(SRC)/apisupt.asm $(SRC)/segs.inc
console.o:	$(SRC)/console.asm $(SRC)/io.inc
cpu.o:	$(SRC)/cpu.asm  $(SRC)/segs.inc
dosidle.o:	$(SRC)/dosidle.asm $(SRC)/segs.inc
entry.o:	$(SRC)/entry.asm   $(SRC)/segs.inc $(HDR)/stacks.inc
execrh.o:	$(SRC)/execrh.asm  $(SRC)/segs.inc
int2f.o:	$(SRC)/int2f.asm   $(SRC)/segs.inc $(HDR)/stacks.inc
intr.o:	$(SRC)/intr.asm    $(SRC)/segs.inc
io.o:		$(SRC)/io.asm	    $(SRC)/segs.inc $(HDR)/stacks.inc
irqstack.o:   $(SRC)/irqstack.asm $(SRC)/segs.inc
kernel.o:	$(SRC)/kernel.asm  $(SRC)/segs.inc
nls_hc.o:	$(SRC)/nls_hc.asm  $(SRC)/segs.inc
nlssupt.o:	$(SRC)/nlssupt.asm $(SRC)/segs.inc $(HDR)/stacks.inc
printer.o:	$(SRC)/printer.asm $(SRC)/io.inc
procsupt.o:	$(SRC)/procsupt.asm $(SRC)/segs.inc $(HDR)/stacks.inc
serial.o:	$(SRC)/serial.asm  $(SRC)/io.inc

GEN_ASMS = plt.asm cdata.asm
GEN_TMP = thunk_calls.tmp plt.inc

$(OBJS): %.o: $(SRC)/%.asm makefile
	$(NASM) $(NASMFLAGS) -o $@ $<

$(DOBJS): %.o: $(SRC)/drivers/%.asm makefile
	$(NASM) $(NASMFLAGS) -o $@ $<

$(PPOBJS): %.o: %.asm makefile
	$(NASM) $(NASMFLAGS) -o $@ $<

plt.o: plt.asm plt.inc $(SRC)/segs.inc

plt.asm: $(srcdir)/plt.S $(SRC)/glob_asm.h $(TOP)/include/fdpp/bprm.h \
   $(srcdir)/makefile
	$(CPP) $(CPPFLAGS) -iquote $(PPINC) -P $< >$@ || ($(RM) $@ ; false)

GIT_DESCRIBE := $(shell git describe)

cdata.asm: $(srcdir)/cdata.S $(HDR)/version.h $(GIT_REV) $(srcdir)/makefile
	$(CPP) -iquote $(HDR) -DKERNEL_VERSION="$(VERSION) [GIT: $(GIT_DESCRIBE)]" \
	-DCMA=, -P $< >$@

thunk_calls.tmp: $(SRC)/proto.h
	srcdir=$(srcdir)/$(T)/parsers CPP="$(CPP)" \
	$(srcdir)/$(T)/parsers/parse_decls.sh 1 $< >$@ || \
	($(RM) $@ ; false)
plt.inc: thunk_calls.tmp
	$(srcdir)/$(T)/parsers/parse_decls.sh 3 $< >$@ || \
	($(RM) $@ ; false)
