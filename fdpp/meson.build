project('libfdpp', ['c', 'cpp'], default_options: ['cpp_std=c++20'],
  version: '0.1')
MF = meson.current_source_dir() / 'parsers/mkfar.sh'
MA = meson.current_source_dir() / 'parsers/mkasmdefs.sh'
PD = meson.current_source_dir() / 'parsers/parse_decls.sh'
M4 = meson.current_source_dir() / 'parsers/thunks.m4'
tg = dependency('thunk_gen')
TG = tg.get_pkgconfig_variable('bindir') / 'thunk_gen'
TGS = tg.get_pkgconfig_variable('bindir') / 'tg_m4'
TGM4 = tg.get_pkgconfig_variable('m4script')
TFLAGS = ['-a', '2', '-p', '2']
SRC = 'src'
rel_inc = 'include/fdpp'
abs_inc = meson.current_source_dir() / rel_inc
incdir = include_directories(rel_inc)
cpp = meson.get_compiler('c')
AVER = cpp.get_define('FDPP_API_VER',
  include_directories: incdir,
  prefix: '#include "thunks.h"')
BVER = cpp.get_define('BPRM_VER',
  include_directories: incdir,
  prefix: '#include "bprm.h"')
FVER = AVER + '.' + BVER
TARGET = 'fdppkrnl.' + FVER

CFILES = [
    SRC / 'blockio.c',
    SRC / 'break.c',
    SRC / 'chario.c',
    SRC / 'dosfns.c',
    SRC / 'dsk.c',
    SRC / 'error.c',
    SRC / 'fatdir.c',
    SRC / 'fatfs.c',
    SRC / 'fattab.c',
    SRC / 'fcbfns.c',
    SRC / 'hmamgr.c',
    SRC / 'inthndlr.c',
    SRC / 'ioctl.c',
    SRC / 'memmgr.c',
    SRC / 'misc.c',
    SRC / 'newstuff.c',
    SRC / 'network.c',
    SRC / 'nls.c',
    SRC / 'strings.c',
    SRC / 'sysclk.c',
    SRC / 'systime.c',
    SRC / 'task.c',
    SRC / 'config.c',
    SRC / 'initoem.c',
    SRC / 'main.c',
    SRC / 'inithma.c',
    SRC / 'dyninit.c',
    SRC / 'initdisk.c',
    SRC / 'initclk.c',
    SRC / 'prf.c',
    SRC / 'share.c']

ccgen = generator(find_program(MF),
  output: '@BASENAME@.cc',
  arguments: ['@INPUT@'],
  capture: true)
ccfiles = ccgen.process(CFILES)

gad = custom_target('glob_asmdefs.h',
  output: 'glob_asmdefs.h',
  input: 'src/glob_asm.h',
  command: [MA, '@INPUT@'],
  capture: true)

GEN_TMP = { '1':'thunk_calls.tmp', '2':'thunk_asms.tmp' }
GEN_H = { 'plt_asmc.h':'4', 'plt_asmp.h':'5' }

env = environment()
env.set('CPP', 'cpp')
env.set('srcdir', meson.current_source_dir() / 'parsers')
gtgen = generator(find_program(PD),
  arguments: ['@EXTRA_ARGS@', '@INPUT@'],
  output: '@BASENAME@.tmp',
  capture: true)
gt = []
foreach n: GEN_TMP.keys()
  gt += gtgen.process(SRC / 'proto.h',
      extra_args: n,
      env: ['CPP=cpp', 'srcdir=' + meson.current_source_dir() / 'parsers'])
endforeach
GEN = { GEN_TMP['1']:gt[0], GEN_TMP['2']:gt[1] }
pac = custom_target('plt_asmc.h',
  output: 'plt_asmc.h',
  input: GEN['thunk_asms.tmp'],
  command: [PD, GEN_H['plt_asmc.h'], '@INPUT@'],
  env: env,
  capture: true)
pap = custom_target('plt_asmp.h',
  output: 'plt_asmp.h',
  input: GEN['thunk_asms.tmp'],
  command: [PD, GEN_H['plt_asmp.h'], '@INPUT@'],
  env: env,
  capture: true)

tc = custom_target('thunk_calls.h',
  output: 'thunk_calls.h',
  input: GEN['thunk_calls.tmp'],
  command: [TG, TFLAGS],
  feed: true,
  capture: true)

ta1 = custom_target('thunk_asms1.h',
  output: 'thunk_asms1.h',
  input: GEN['thunk_asms.tmp'],
  command: [TG, TFLAGS, '1'],
  feed: true,
  capture: true)

ta2 = custom_target('thunk_asms2.h',
  output: 'thunk_asms2.h',
  input: GEN['thunk_asms.tmp'],
  command: [TG, TFLAGS, '2'],
  feed: true,
  capture: true)

ta = custom_target('thunk_asms.h',
  output: 'thunk_asms.h',
  input: [ta1, ta2],
  command: ['sh', '-c', 'cat @INPUT0@ | ' + TGS + ' ' + TGM4 +
            ' | cat - @INPUT1@'],
  capture: true)

FDPP_CFILES = ['smalloc.c', 'farhlp_sta.c']
ccgen = generator(find_program('echo'),
  arguments: ['#include', '"@BASENAME@.c"'],
  output: '@BASENAME@.cc',
  capture: true)
ppccf = ccgen.process(FDPP_CFILES)

FDPP_CCFILES = ['thunks.cc', 'thunks_c.cc', 'thunks_a.cc', 'thunks_p.cc',
  'dosobj.cc']
CPPFILES = ['objhlp.cpp', 'ctors.cpp', 'farhlp.cpp', 'objtrace.cpp']
hdr = ['src', 'hdr', '.', rel_inc]
incdir2 = include_directories(hdr)
CLANG_VER = meson.get_compiler('cpp').version().split('.')[0]
LIBFDPP = shared_library('fdpp', [ccfiles, ppccf, FDPP_CCFILES, CPPFILES],
  include_directories: incdir2,
  version: FVER,
  sources: [gad, pac, pap, tc, ta],
  cpp_args: ['-DFDPP', '-DDEBUG', '-DWITHFAT32', '-DCLANG_VER=' + CLANG_VER,
      '-fno-threadsafe-statics',
      '-Werror=packed-non-pod', '-Wno-unknown-warning-option',
      '-Wno-format-invalid-specifier', '-Wno-c99-designator'
  ],
  link_args: ['-Wl,-Bsymbolic'],
  install: true,
  install_dir: get_option('libdir') / 'fdpp')
