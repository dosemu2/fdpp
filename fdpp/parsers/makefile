-include config.mak
XCPU ?= 386
srcdir ?= $(CURDIR)
include $(srcdir)/../../mkfiles/gcc.mak

thunk_gen: lex.yy.c thunk_gen.tab.c
	$(CLC) -g -o $@ $^

thunk_gen.tab.c: $(srcdir)/thunk_gen.y
	bison -d $^

lex.yy.c: $(srcdir)/thunk_gen.l
	lex $^

clean:
	$(RM) *.tab.* lex.yy.c thunk_gen

