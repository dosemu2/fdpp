XCPU ?= 386
include ../../mkfiles/gcc.mak

thunk_gen: lex.yy.c thunk_gen.tab.c
	$(CLC) -g -o $@ $^

thunk_gen.tab.c: thunk_gen.y
	bison -d $^

lex.yy.c: thunk_gen.l
	lex $^

clean:
	$(RM) *.tab.* lex.yy.c thunk_gen

