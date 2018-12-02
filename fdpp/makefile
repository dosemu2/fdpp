-include config.mak
srcdir ?= $(CURDIR)
lib_path ?= ../lib
utils_path ?= ../utils
include $(srcdir)/defs.mak
include $(srcdir)/clang.mak

LIBS=$(lib_path)/device.lib
HDR=$(srcdir)/../hdr/
SRC=$(srcdir)/../kernel/
FDPPLIB = libfdpp.so
RELVER = beta2
PKG = fdpp-0.1$(RELVER)
TGZ = $(PKG).tar.gz

ALLCFLAGS += -I . -I $(SRC) -I $(srcdir) -DDATADIR=$(DATADIR) \
    -DKRNL_NAME=$(TARGET).sys
NASMFLAGS += -i$(SRC)

# *List Macros*
# Only 8 files per definition; this is limitation of DOS batch
# files (only 9 directly accessible parameters).

ASMS = kernel.asm entry.asm io.asm console.asm serial.asm printer.asm \
    execrh.asm nlssupt.asm procsupt.asm dosidle.asm int2f.asm \
    nls_hc.asm intr.asm irqstack.asm memdisk.asm \
    cpu.asm
DASMS = floppy.asm rdpcclk.asm wrpcclk.asm wratclk.asm
AASMS = $(ASMS) $(DASMS)
OBJS = $(ASMS:.asm=.o)
DOBJS = $(DASMS:.asm=.o)
INCS = $(AASMS:.asm=_seg.inc) cdata_seg.inc
PPASMS = plt.S cdata.S
PPOBJS = $(PPASMS:.S=.o)

#               *Explicit Rules*

ALL = $(FDPPLIB) $(TARGET).sys fdpp.pc

all: $(ALL)

production:     $(ALL) ../bin/country.sys
clobber:        clean

$(TARGET).sys: $(OBJS) $(PPOBJS) $(DOBJS)
	$(LINK) -T$(srcdir)/kernel.ld -melf_i386 -static -Map kernel.map -o $@ $^
	chmod -x $@

../bin/country.sys: $(SRC)country.asm
	if [ ! -d ../bin ]; then \
	    mkdir ../bin ; \
	fi
	$(NASM) -o $@ $<

clean:
	+cd parsers && make srcdir=$(abspath $(srcdir))/parsers clean
	-$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.cod *.err *.elf *.inc \
		*.lnk *.o $(GEN_CC) $(FDPPLIB) *.tmp $(GEN_HEADERS) $(GEN_ASMS) \
		$(TARGET).sys kernel.sym *.pc

#               *Individual File Dependencies*
#apisupt.obj:	$(SRC)apisupt.asm $(SRC)segs.inc
console.obj:	$(SRC)console.asm $(SRC)io.inc
cpu.obj:	$(SRC)cpu.asm  $(SRC)segs.inc
dosidle.obj:	$(SRC)dosidle.asm $(SRC)segs.inc
entry.obj:	$(SRC)entry.asm   $(SRC)segs.inc $(HDR)stacks.inc
execrh.obj:	$(SRC)execrh.asm  $(SRC)segs.inc
int2f.obj:	$(SRC)int2f.asm   $(SRC)segs.inc $(HDR)stacks.inc
intr.obj:	$(SRC)intr.asm    $(SRC)segs.inc
io.obj:		$(SRC)io.asm	    $(SRC)segs.inc $(HDR)stacks.inc
irqstack.obj:   $(SRC)irqstack.asm $(SRC)segs.inc
kernel.obj:	$(SRC)kernel.asm  $(SRC)segs.inc $(SRC)ludivmul.inc
memdisk.obj:	$(SRC)memdisk.asm  $(SRC)segs.inc
nls_hc.obj:	$(SRC)nls_hc.asm  $(SRC)segs.inc
nlssupt.obj:	$(SRC)nlssupt.asm $(SRC)segs.inc $(HDR)stacks.inc
printer.obj:	$(SRC)printer.asm $(SRC)io.inc
procsupt.obj:	$(SRC)procsupt.asm $(SRC)segs.inc $(HDR)stacks.inc
serial.obj:	$(SRC)serial.asm  $(SRC)io.inc

HDRS = $(wildcard $(HDR)*.h)
PLPHDRS = farobj.hpp farptr.hpp dispatch.hpp
_PPHDRS = $(PLPHDRS) dosobj.h farhlp.h thunks_priv.h thunks.h smalloc.h
PPHDRS = $(addprefix $(srcdir)/,$(_PPHDRS))
GEN_HEADERS=thunk_calls.h thunk_asms.h glob_asmdefs.h
GEN_ASMS=plt.asm cdata.asm
# dont change file order as it matches the gen script
GEN_TMP = thunk_calls.tmp thunk_asms.tmp plt.inc plt_asmc.inc plt_asmp.inc
GEN_CC = $(CFILES:.c=.cc)
HEADERS=$(HDRS) $(SRC)globals.h $(SRC)proto.h $(GEN_HEADERS) $(PPHDRS)
INITHEADERS=$(HDRS) init-mod.h init-dat.h

CFILES = blockio.c \
    break.c \
    chario.c \
    dosfns.c \
    dsk.c \
    error.c \
    fatdir.c \
    fatfs.c \
    fattab.c \
    fcbfns.c \
    inthndlr.c \
    ioctl.c \
    memmgr.c \
    newstuff.c \
    network.c \
    nls.c \
    strings.c \
    sysclk.c \
    systime.c \
    task.c \
    config.c \
    initoem.c \
    main.c \
    inithma.c \
    dyninit.c \
    initdisk.c \
    initclk.c \
    prf.c

FDPP_CFILES = \
    smalloc.c \
    dosobj.c \
    farhlp.c
FDPP_CCFILES = \
    thunks.cc

CPPFILES = objhlp.cpp

OBJECTS = $(CFILES:.c=.o)
FDPP_COBJS = $(FDPP_CFILES:.c=.o)
FDPP_CCOBJS = $(FDPP_CCFILES:.cc=.o)
FDPP_CPPOBJS = $(CPPFILES:.cpp=.o)

$(OBJECTS): %.o: %.cc $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

$(FDPP_COBJS): %.o: $(srcdir)/%.c $(PPHDRS)
	$(CLC) $(CLCFLAGS) -o $@ $<

$(FDPP_CCOBJS): %.o: $(srcdir)/%.cc $(PPHDRS) $(GEN_HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

$(FDPP_CPPOBJS): %.o: $(srcdir)/%.cpp
	$(CC) $(CFLAGS) -o $@ $<

$(OBJS): %.o: $(SRC)%.asm
	$(NASM) $(NASMFLAGS) -f elf -o $@ $<

$(DOBJS): %.o: $(SRC)../drivers/%.asm
	$(NASM) $(NASMFLAGS) -f elf -o $@ $<

$(PPOBJS): %.o: %.asm
	$(NASM) $(NASMFLAGS) -f elf -o $@ $<

.cpp.o: $(PLPHDRS)
	$(CC) $(CFLAGS) -o $@ $<

$(GEN_CC): %.cc: $(SRC)%.c makefile
	$(srcdir)/parsers/mkfar.sh $< >$@

$(FDPPLIB): $(OBJECTS) $(FDPP_COBJS) $(FDPP_CCOBJS) $(FDPP_CPPOBJS)
	$(CL) -o $@ $^ $(LDFLAGS)
	@echo "Have `nm -u $@ | grep "U " | wc -l` undefined symbols"

$(INCS): %_seg.inc: %.o
	objdump -t $< | $(srcdir)/parsers/gen_segs.sh $(SRC)segs.inc >$@

plt_seg.inc: $(INCS)
	echo $(patsubst %.inc,\%include \"%.inc\",$(INCS)) | sed 's/.inc" /.inc"\n/g' >$@

plt.o: plt.asm plt.inc plt_asmc.inc plt_asmp.inc plt_seg.inc $(SRC)segs.inc

plt.asm: $(srcdir)/plt.S $(SRC)/glob_asm.h
	cpp -I $(SRC) -P $< >$@

cdata.asm: $(srcdir)/cdata.S
	cpp -P $^ | sed 's/" "//g' >$@

_pos = $(if $(findstring $1,$2),$(call _pos,$1,\
       $(wordlist 2,$(words $2),$2),x $3),$3)
pos = $(words $(call _pos,$1,$2))
pars = srcdir=$(srcdir)/parsers \
	$(srcdir)/parsers/parse_decls.sh $(call pos,$@,$(GEN_TMP)) $< >$@
$(filter %.tmp,$(GEN_TMP)): $(SRC)proto.h
	$(pars)
plt.inc: thunk_calls.tmp
	$(pars)
plt_asmc.inc plt_asmp.inc: thunk_asms.tmp
	$(pars)

thunk_calls.h: thunk_calls.tmp parsers/thunk_gen
	parsers/thunk_gen <$< >$@ || (rm -f $@ ; false)

thunk_asms.h: thunk_asms.tmp parsers/thunk_gen
	parsers/thunk_gen 1 <$< >$@ || (rm -f $@ ; false)

glob_asmdefs.h: $(SRC)/glob_asm.h
	$(srcdir)/parsers/mkasmdefs.sh $< >$@

parsers/thunk_gen:
	if [ ! -f parsers/makefile ]; then \
	    ln -s $(abspath $(srcdir))/parsers/makefile parsers/makefile ; \
	fi
	+cd parsers && make srcdir=$(abspath $(srcdir))/parsers

install: $(FDPPLIB) fdpp.pc
	install -D -t $(DESTDIR)$(LIBDIR)/fdpp $(FDPPLIB)
	install -D -t $(DESTDIR)$(PKGCONFIGDIR) -m 0644 fdpp.pc
	install -D -t $(DESTDIR)$(PREFIX)/include/fdpp -m 0644 $(srcdir)/thunks.h
	install -D -t $(DESTDIR)$(DATADIR) -m 0644 fdppkrnl.sys

$(TGZ):
	cd $(srcdir)/.. && git archive -o $(CURDIR)/$(TGZ) --prefix=$(PKG)/ HEAD
.PHONY: $(TGZ)

tar: $(TGZ)

rpm: $(TGZ)
	rpmbuild -tb $(TGZ)

deb:
	debuild -i -us -uc -b

fdpp.pc: $(srcdir)/fdpp.pc.in makefile
	sed \
		-e 's!@PREFIX[@]!$(PREFIX)!g' \
		-e 's!@LIBDIR[@]!$(LIBDIR)!g' \
		$< >$@
