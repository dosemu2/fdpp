COMPILER ?= gcc
XCPU ?= 386
include ../mkfiles/generic.mak

CFLAGS = -I..$(DIRSEP)hdr

production: patchobj.com exeflat.exe thunk_gen

patchobj.com: patchobj.c
	$(CLT) $(CFLAGS) -o $@ patchobj.c

exeflat.exe: exeflat.c ../hdr/exe.h
	$(CLC) $(CFLAGS) -o $@ exeflat.c

thunk_gen: lex.yy.c thunk_gen.tab.c
	$(CLC) -g -o $@ $^

thunk_gen.tab.c: thunk_gen.y
	bison -d $^

lex.yy.c: thunk_gen.l
	lex $^

clobber: clean

clean:
	$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.las *.cod *.err status.me
	$(RM) *.tab.*
	$(RM) exeflat.exe patchobj.com thunk_gen

