project('fdpp', ['cpp', 'c', 'nasm'], version: '0.1', meson_version: '>= 0.64')

li = subproject('libfdpp')
LIBFDPP = li.get_variable('LIBFDPP')
lo = subproject('loader')
LIBLOADER = lo.get_variable('LIBLOADER')
ke = subproject('kernel')
VERSION = ke.get_variable('VERSION')

pkg = import('pkgconfig')
pkg.generate(LIBFDPP,
  libraries: LIBLOADER,
  version: VERSION)

install_subdir('include/fdpp',
  install_dir: 'include')
