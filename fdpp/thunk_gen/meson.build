project('thunk_gen', 'c', version: '0.1')

flex = find_program('flex', required: true)
bison = find_program('bison', required: true)

lgen = generator(flex,
  output : '@PLAINNAME@.yy.c',
  arguments : ['-o', '@OUTPUT@', '@INPUT@'])
lfiles = lgen.process('thunk_gen.l')

pgen = generator(bison,
  output : ['@BASENAME@.tab.c', '@BASENAME@.tab.h'],
  arguments : ['@INPUT@', '--defines=@OUTPUT1@', '--output=@OUTPUT0@'])
pfiles = pgen.process('thunk_gen.y')

TG = executable('thunk_gen', lfiles, pfiles)
