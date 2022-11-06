-include config.mak
srcdir ?= $(CURDIR)
include $(srcdir)/../clang.mak

thunk_gen: lex.yy.o thunk_gen.tab.o
	$(CC_FOR_BUILD) -g -o $@ $^

lex.yy.o: lex.yy.c thunk_gen.tab.h

%.o: %.c
	$(CC_FOR_BUILD) $(CLCFLAGS) -Wno-unused-function -o $@ $<

# not sure why GNU make does not set BISON
BISON ?= bison
need = 4.3
ifneq ($(filter $(need),$(firstword $(sort $(MAKE_VERSION) $(need)))),)
thunk_gen.tab.c thunk_gen.tab.h &: $(srcdir)/thunk_gen.y
else
%.tab.c %.tab.h : $(srcdir)/%.y
endif
	$(BISON) -d $^

lex.yy.c: $(srcdir)/thunk_gen.l
	$(LEX) $^

clean:
	$(RM) -f *.tab.* lex.yy.c *.o thunk_gen
