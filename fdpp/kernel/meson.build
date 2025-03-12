project('kernel', ['nasm', 'c'], version: '1.7', meson_version: '>= 1.3.2')

PD = meson.current_source_dir() / 'parsers/parse_decls.sh'
SRC = 'src'
DSRC = 'src/drivers'
kernel_ld = meson.current_source_dir() / 'kernel.ld'
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
GIT_DESCRIBE = run_command('git', 'describe', check: false).stdout().strip()
hdr = ['src', 'hdr', rel_inc]
incdir2 = include_directories(hdr)
sfiles = cpp.preprocess(['cdata.S', 'plt.S'],
  output: '@BASENAME@.asm',
  include_directories: incdir2,
  compile_args: [
      '-D__ASSEMBLER__',
      '-DKERNEL_VERSION=' + meson.project_version() +
        ' [GIT: ' + GIT_DESCRIBE + ']',
      '-DCMA=,', '-P'])
env = environment()
env.set('CPP', 'cpp')
env.set('srcdir', meson.current_source_dir() / '../parsers')
tct = custom_target('thunk_calls.tmp',
  output: 'thunk_calls.tmp',
  input: SRC / 'proto.h',
  env: env,
  command: [PD, '1', '@INPUT@'],
  capture: true)
plt_inc = custom_target('plt.inc',
  output: 'plt.inc',
  input: tct,
  command: [PD, '3', '@INPUT@'],
  capture: true)

SRCS = [
    SRC/'console.asm',
    SRC/'cpu.asm',
    SRC/'dosidle.asm',
    SRC/'entry.asm',
    SRC/'execrh.asm',
    SRC/'int2f.asm',
    SRC/'intr.asm',
    SRC/'io.asm',
    SRC/'irqstack.asm',
    SRC/'kernel.asm',
    SRC/'nls_hc.asm',
    SRC/'nlssupt.asm',
    SRC/'printer.asm',
    SRC/'procsupt.asm',
    SRC/'serial.asm',

    DSRC/'floppy.asm',
    DSRC/'rdpcclk.asm',
    DSRC/'wrpcclk.asm',
    DSRC/'wratclk.asm',
]

nargs = ['-DXCPU=186', '-DWITHFAT32', '-DFDPP']

executable(TARGET + '.elf',
  [SRCS, sfiles, plt_inc],
  include_directories: incdir2,
  nasm_args: nargs,
  link_args: ['-Wl,-melf_i386', '-Wl,-static',
    '-Wl,-Map=' + TARGET + '.map', '-nostdlib',
    '-Wl,--build-id=none',
    '-Wl,--emit-relocs', '-Wl,-T' + kernel_ld],
  install: true,
  install_dir: get_option('datadir') / 'fdpp',
  install_mode: 'rw-r--r--')

VERSION = meson.project_version()
