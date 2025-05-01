project('libfdpp', ['c', 'cpp'], default_options: ['cpp_std=c++20'],
  version: '0.1', meson_version: '>= 1.3.0')

MF = meson.current_source_dir() / 'parsers/mkfar.sh'
PD = meson.current_source_dir() / 'parsers/parse_decls.sh'
RA = meson.current_source_dir() / 'run_arg.sh'
tg = dependency('thunk_gen')
TGMK = tg.get_variable(pkgconfig: 'makeinc')
TFLAGS = '-a 2 -p 2'
SRC = 'src'
rel_inc = 'include/fdpp'
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

make = find_program('make')

gad = custom_target('glob_asmdefs.h',
  output: 'glob_asmdefs.h',
  input: 'src/glob_asm.h',
  command: [RA, 'glob_asmdefs.h', make, '-f', TGMK,
            'glob_asmdefs.h', 'GLOB_ASM=@INPUT@'],
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

tgenv = environment()
tgenv.set('TFLAGS', TFLAGS)

tc = custom_target('thunk_calls.h',
  output: 'thunk_calls.h',
  input: GEN['thunk_calls.tmp'],
  command: [RA, '-i', 'thunk_calls.tmp', 'thunk_calls.h',
            make, '-f', TGMK, 'thunk_calls.h'],
  env: tgenv,
  feed: true,
  capture: true)

ta = custom_target('thunk_asms.h',
  output: 'thunk_asms.h',
  input: GEN['thunk_asms.tmp'],
  command: [RA, '-i', 'thunk_asms.tmp', 'thunk_asms.h',
            make, '-f', TGMK, 'thunk_asms.h'],
  env: tgenv,
  feed: true,
  capture: true)

FDPP_CFILES = ['smalloc.c']
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
cxx_id = meson.get_compiler('cpp').get_id()
if get_option('buildtype').startswith('debug')
  DEBUG_OPTS = ['-DFDPP_DEBUG', '-DEXTRA_DEBUG']
  if cxx_id == 'clang'
    DEBUG_OPTS += ['-fdebug-macro']
  endif
else
  DEBUG_OPTS = []
endif
CXX_ARGS = ['-DFDPP', '-DDEBUG', '-DWITHFAT32', '-DCLANG_VER=' + CLANG_VER,
      '-fno-threadsafe-statics',
      '-Wno-address-of-packed-member', DEBUG_OPTS]
if cxx_id == 'clang'
  CXX_ARGS += ['-Werror=packed-non-pod', '-Wno-c99-designator',
      '-Wno-format-invalid-specifier', '-Wno-unknown-warning-option']
else
  CXX_ARGS += ['-Wno-format', '-Wno-format-extra-args',
      '-Wno-error=format-security', '-fpermissive']
endif
LIBFDPP = shared_library('fdpp', [ccfiles, ppccf, FDPP_CCFILES, CPPFILES],
  include_directories: incdir2,
  version: FVER,
  sources: [gad, pac, pap, tc, ta],
  cpp_args: CXX_ARGS,
  link_args: ['-Wl,-Bsymbolic'],
  install: true,
  install_dir: get_option('libdir') / 'fdpp')
