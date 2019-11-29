# fdpp

## what is fdpp?
fdpp is a 64-bit DOS core.<br/>
It is based on a FreeDOS kernel ported to modern C++.<br/>
In short, FreeDOS plus-plus.

Can be compiled with clang (not gcc!) and booted under
[dosemu2](https://github.com/stsp/dosemu2).

## building and installing
Just run `make`.<br/>
For an out-of-tree build, run `<path_to_fdpp>/configure`
before running `make`.<br/>
After compiling, run `sudo make install` to install.

## installing from pre-built package
For the ubuntu package please visit
[this PPA](https://code.launchpad.net/~dosemu2/+archive/ubuntu/ppa).

## running
The simplest way to get it running is to use
[dosemu2](https://github.com/stsp/dosemu2).<br/>
After installing fdpp, you can (re-)build dosemu2.
It will then find fdpp during build, and enable its support.<br/>
You can also get the pre-built dosemu2 packages with
fdpp support enabled from the aforementioned
[ubuntu PPA](https://code.launchpad.net/~dosemu2/+archive/ubuntu/ppa).

## configuration
fdpp uses fdppconf.sys file at boot. This file has
the standard format of config.sys with some syntax
extensions. It is fully backward-compatible with the
DOSish config.sys, and if fdppconf.sys is missing,
config.sys is attempted instead.

## but what it *actually* is? why dosemu2?
fdpp is a user-space library that needs a couple of
call-backs to be provided to it by the host program.
This means it can't be booted from the bare-metal PC,
as the original freedos could. But all it needs is a
few call-backs for running real-mode code. So if you
want to run it on something other than dosemu2, please
use
[this code](https://github.com/stsp/dosemu2/blob/devel/src/plugin/fdpp/fdpp.c)
as a reference implementation. Sufficiently DOS-oriented
kernels like [freedos-32](http://freedos-32.sourceforge.net/)
are good candidates for running fdpp.

## what fdpp technically is?
A meta-compiler that was initially able to compile the
freedos kernel from its almost unmodified sources.
As the project advances, the ability to compile freedos
from the unmodified sources have lost its value, as our
copy of freedos is now heavily modified. The main tasks
of our meta-compiler is to parse FAR pointers and generate
thunks to real-mode asm code of freedos.

## compatibility
fdpp removes some compatibility hacks that FreeDOS had.
For example it doesn't play tricks with disabling A20 when
returning to user code, so you may see "Packed file is corrupt"
message on some old exe files. Using "loadfix" command avoids
that problem.
On the other hand fdpp supports much more games than does freedos.
If you find some compatibility problem, please report a bug.

## portability
fdpp can work in any environment with STL/C++ runtime support.
Requirements to the standard libraries are very small, far
within the ISO standards. No posix or OS-specific APIs are used.
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
[dosemu2](https://github.com/stsp/dosemu2)
is a virtual machine that allows you to run DOS software under linux.
It is a primary host platform for fdpp.

### comcom32
[comcom32](https://github.com/stsp/comcom32)
is a 32bit command.com that is supposed to be used with fdpp
while we don't have a 64bit one.

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

### freedos-32
[freedos-32](http://freedos-32.sourceforge.net/) is a
kernel written with DOS compatibility in mind. It has a
rich user-space part with libc. Very good candidate for
running fdpp. Unfortunately,
[the kernel](https://sourceforge.net/p/freedos-32/code/HEAD/tree/trunk/)
was scrapped, and currently
[something else](https://github.com/salvois/kernel)
is being developed.

### freedos-64
[freedos-64](https://sourceforge.net/projects/dos64/)
seems to be a project
[planning](http://freedos.10956.n7.nabble.com/DOS-Development-Idea-td16159.html)
to implement a 64bit kernel around the real-mode
freedos kernel.

### NightKernel
[NightKernel](https://github.com/mercury0x000d/NightKernel)
is `A 32-bit drop-in replacement for the FreeDOS kernel`, as
they call themselves. In fact, it is an assembly-written
ring-0 kernel, currently w/o any DOS compatibility at all.
Can't be used to run fdpp because it doesn't support user-space
add-ons.
