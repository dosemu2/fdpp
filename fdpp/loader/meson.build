project('loader', 'c', version: '0.1')
TOP = '../..'
rel_inc = TOP / 'include/fdpp'
abs_inc = meson.current_source_dir() / rel_inc
lelf = dependency('libelf')
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
shared_library('fdldr', 'elf.c', 'loader.c',
  dependencies: lelf, include_directories: incdir,
  version: FVER,
  c_args: [
    '-DFDPPKRNLDIR=' + get_option('prefix') / get_option('datadir') / 'fdpp',
    '-DKRNL_ELFNAME=' + TARGET + '.elf',
    '-DKRNL_MAP_NAME=' + TARGET + '.map'
  ],
  install: true,
  install_dir: get_option('libdir') / 'fdpp')
