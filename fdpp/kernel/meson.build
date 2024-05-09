project('kernel', ['nasm', 'c'], version: '1.7', meson_version: '>= 1.3.2')

PD = 'parsers/parse_decls.sh'
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
GIT_DESCRIBE = run_command('git', 'describe', check: true).stdout().strip()
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

lib = static_library('dummy',
  [SRCS, sfiles, plt_inc],
  include_directories: incdir2,
  nasm_args: nargs,
  build_by_default: false)
nasm_ld = find_program(['i686-linux-gnu-ld', 'i386-elf-ld', 'x86_64-linux-gnu-ld', 'ld'])
custom_target(TARGET + '.elf',
  output: [TARGET + '.elf', TARGET + '.map'],
  input: lib.extract_all_objects(recursive: true),
  command: [nasm_ld, '-melf_i386', '-static',
    '-Map=@OUTPUT1@', '-o', '@OUTPUT0@',
    '--emit-relocs', '-T' + kernel_ld, '@INPUT@'],
  install: true,
  install_dir: get_option('datadir') / 'fdpp',
  install_mode: 'rw-r--r--',
  build_by_default: true)

VERSION = meson.project_version()
