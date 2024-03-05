# fdpp

## what is fdpp?
fdpp is a 64-bit DOS core.<br/>
It is based on a FreeDOS kernel ported to modern C++.<br/>
In short, FreeDOS plus-plus.

Can be compiled with clang (not gcc!) and booted under
[dosemu2](https://github.com/dosemu2/dosemu2).

## building and installing
Run `./configure.meson [<build_dir>]`.<br/>
This creates and configures the build dir and prints
the instructions for the further build steps. It should also inform
you about any missing build-dependencies, which you need to install.

## installing from pre-built package
For the ubuntu package please visit
[this PPA](https://code.launchpad.net/~dosemu2/+archive/ubuntu/ppa).
Fedora packages are
[here](https://copr.fedorainfracloud.org/coprs/stsp/dosemu2).
OpenSUSE packages are
[here](https://download.opensuse.org/repositories/home:/stsp2/openSUSE_Tumbleweed).

## running
The simplest way to get it running is to use
[dosemu2](https://github.com/dosemu2/dosemu2).<br/>
Get the pre-built dosemu2 packages from
[ubuntu PPA](https://code.launchpad.net/~dosemu2/+archive/ubuntu/ppa)
or from
[COPR repo](https://copr.fedorainfracloud.org/coprs/stsp/dosemu2)
or from
[OpenSUSE repo](https://download.opensuse.org/repositories/home:/stsp2/openSUSE_Tumbleweed)
or build it from
[sources](https://github.com/dosemu2/dosemu2).

## configuration
fdpp uses fdppconf.sys file at boot. This file has
the standard format of config.sys with some syntax
extensions. It is fully backward-compatible with the
DOSish config.sys, and if fdppconf.sys is missing,
config.sys is attempted instead.

## but what it *actually* is? why dosemu2?
fdpp is a user-space library that, as any DOS, can
run DOS programs. Being a library, it can't act on
its own and needs a host program to operate. This
also means it can't be booted from the bare-metal
PC, as the original freedos could.<br/>
The host program needs to provide a couple of
call-backs for running real-mode code in v86 or
similar environment. See
[this code](https://github.com/stsp/dosemu2/blob/devel/src/plugin/fdpp/fdpp.c)
as an example of providing call-backs to fdpp, and
[this code](https://github.com/stsp/dosemu2/blob/devel/src/plugin/fdpp/boot.c)
as an example of the boot loader.
Sufficiently DOS-oriented kernels like
[freedos-32](http://freedos-32.sourceforge.net/)
or
[pdos/386](http://pdos.sourceforge.net/)
are the good candidates for running fdpp in the future.

## what fdpp technically is?
A meta-compiler that was initially able to compile the
freedos kernel from its almost unmodified sources.
As the project evolves, the ability to compile freedos
from the unmodified sources have lost its value and was
dropped, as our copy of freedos is now heavily modified.
As the result, the compiler is very small and is targeted
only to our coding patterns.<br/>
The main tasks of it are to parse FAR pointers and generate
thunks to real-mode asm code of freedos. As with any other
compiler, fdpp is supplied with a small runtime support
library and a dynamic linker.

## compatibility
Simply put, fdpp runs every DOS program we tried, so the
compatibility level is supposed to be very high. Better
than that of most other known DOS clones, including FreeDOS.<br/>
If you find some compatibility problems, please
[report a bug](https://github.com/dosemu2/fdpp/issues).

## portability
fdpp can work in any environment with STL/C++ runtime & minimal
posix support.
The build requirements are in line with today's posix-compatible
environments: you'll need the full stack of tools like bison,
autoconf, sed etc. Additionally you'll need a clang tool-chain
as clang++ is the only compiler to build fdpp with. That said,
building fdpp under itself may be a challenge. :)

## related projects
### FreeDOS
[FreeDOS kernel](http://www.fdos.org/kernel/) is a
DOS-compatible kernel that is used as a core of fdpp.

### dosemu2
[dosemu2](https://github.com/dosemu2/dosemu2)
is a virtual machine that allows you to run DOS software under linux.
It is a primary host platform for fdpp.

## similar projects
### dosbox
[dosbox](https://www.dosbox.com/) has a good
[built-in DOS](https://sourceforge.net/p/dosbox/code-0/HEAD/tree/dosbox/trunk/src/dos/)
written in C++. It is tightly coupled to dosbox; you can't
easily port it to your project, whereas fdpp is designed to
be plugable. Also it uses host APIs and libraries for
filesystem access, CD-ROM playing and similar things. fdpp
uses no host APIs or libraries, which may be good or bad,
depending on your use-case. dosbox is cleanly written in C++,
whereas fdpp wraps the C-coded freedos kernel into a C++ framework,
resulting in a nuclear C/C++ mix.

### freedos-32 (inactive)
[freedos-32](http://freedos-32.sourceforge.net/) is a
kernel written with DOS compatibility in mind. It has a
rich user-space part with libc. Very good candidate for
running fdpp. Unfortunately,
[the kernel](https://sourceforge.net/p/freedos-32/code/HEAD/tree/trunk/)
was scrapped, and currently
[something else](https://github.com/salvois/kernel)
is being developed.

### NightKernel
[NightKernel](https://github.com/mercury0x000d/NightKernel)
is `A 32-bit drop-in replacement for the FreeDOS kernel`, as
they call themselves. In fact, it is an assembly-written
ring-0 kernel, currently w/o any DOS compatibility at all.
Can't be used to run fdpp because it doesn't support user-space
add-ons.

### PDOS/386
[pdos/386](http://pdos.sourceforge.net/)
is a Public Domain Operating System with a user interface as simple
as MSDOS.
It supports a "theoretical 32-bit MSDOS API" (probably similar to
that of 32-bit DOS extenders) and some of the Win32 API, allowing
a subset of Windows executables to be run. PDOS/386 is good enough to
run gcc, as, and ld to rebuild itself.<br/>
As for running fdpp - pdos/386 is a very promising project, but
its author seems to be reluctant at adding v86 support, preferring
to switch to real mode for bios calls. The lack of v86 leaves very
small room for fdpp and for DOS programs in general unfortunately.
