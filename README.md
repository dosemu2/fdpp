# fdpp

## what is fdpp?
fdpp is a 64-bit DOS.<br/>
It is based on a FreeDOS kernel ported to modern C++.<br/>
In short, FreeDOS plus-plus.

Can be compiled with clang (not gcc!) and booted under
[dosemu2](https://github.com/stsp/dosemu2)

## building and installing
Just run `make`.<br/>
For an out-of-tree build, run `<path_to_fdpp>/configure`
before running `make`.<br/>
After compiling, run `sudo make install` to install.

## installing from pre-built package
For the ubuntu package please visit
[this PPA](https://code.launchpad.net/~dosemu2/+archive/ubuntu/ppa)

## running
The simplest way to get it running is to use
[dosemu2](https://github.com/stsp/dosemu2).<br/>
After installing fdpp, you can (re-)build dosemu2.
It will then find fdpp during build, and enable its support.<br/>
You can also get the pre-built dosemu2 packages with
fdpp support enabled from the aforementioned ubuntu PPA.

## but what it *actually* is? why dosemu2?
fdpp is a user-space library that needs a couple of
call-backs to be provided to it by the host program.
This means it can't be booted from the bare-metal PC,
as the original freedos could. But all it needs is a
few call-backs for running real-mode code. So if you
want to run it on something other than dosemu2, please
use [this code](https://github.com/stsp/dosemu2/blob/devel/src/plugin/fdpp/fdpp.c)
as a reference implementation. Sufficiently DOS-oriented
kernels like [freedos-32](http://freedos-32.sourceforge.net/)
are good candidates for running fdpp.
