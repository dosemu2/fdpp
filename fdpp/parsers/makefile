-include config.mak
XCPU ?= 386
srcdir ?= $(CURDIR)
include $(srcdir)/../clang.mak

thunk_gen: lex.yy.o thunk_gen.tab.o
	$(CLC) -g -o $@ $^

lex.yy.o: lex.yy.c thunk_gen.tab.h

%.o: %.c
	$(CLC) $(CLCFLAGS) -Wno-unused-function -o $@ $<

need = 4.3
ifneq ($(filter $(need),$(firstword $(sort $(MAKE_VERSION) $(need)))),)
thunk_gen.tab.c thunk_gen.tab.h &: $(srcdir)/thunk_gen.y
else
%.tab.c %.tab.h : $(srcdir)/%.y
endif
	bison -d $^

lex.yy.c: $(srcdir)/thunk_gen.l
	lex $^

clean:
	rm -f *.tab.* lex.yy.c *.o thunk_gen
