## 1.8

This release contains ~75 patches, and is mostly targeted to resolving
the long-standing technical debts.

- ELF format is now fully utilized. We now use ELF-based symbol tables
  and relocations, including dynamic relocations (which we simulate
  with --emit-relocs). This completes the ELF relocation work started at
  1.3 and 1.4 releases. Unfortunately, this brought in the new toolchain
  restrictions:
  * we no longer support upstream nasm (can only use our own nasm-segelf
    fork of the nasm for now)
  * do no longer support ld.lld (can only use GNU ld for now)
  * no longer support GNU strip (can only use llvm-strip for now,
    but the fix is already in GNU binutils git)
  So the ELF support went in very painfully. We will keep working with
  the upstream projects (lld mainly) in a hope to restore their use.
  Nasm project have rejected our contributions, so the vanilla nasm
  support is not anticipated any soon.
- Meson build system support. But not removing the old makefiles yet,
  as we need meson-1.3, which is not yet widely available.
- Minor thunk_gen and farptr compiler updates.
- Finally fixed remaining AlphaWaves problems. This game tests for the
  MS-DOS compatibility in a very rough ways.
- Fixed a few regressions of 1.7 (Prehistorik2 again)

## 1.7

A very important release, containing ~200 patches.

- Portability work: ported to aarch64, including MacOS, Android.
- Implemented HMA memory manager, and lots of work done on a more
  aggressive HMA usage. Unfortunately dosemu2 disabled HMA support
  by default in a mean time.
- Lots of SHARE fixes.
- Fixed nasty RTC bug (prehistoric2 had problems because of it)
- Fixed stack switching on int21 calls (vlm now works)
- Fixed stack switching on int24
- Added a config.sys syntax for referencing drives created at run-time:
  CHAIN=%0\userhook.sys can be used if the host platform supplies the
  meaning of %0 (other digits are possible) at the right time.
- Added a config.sys syntax for conditional loading:
  device=?0c:\bin\nansi.sys can be used if the host platform supplies
  the value of ?0 (other digits are possible) via boot protocol.
- Fix to support multiple CHAIN directives
- "Get extended free drive space" function can now use redirector
  extensions to properly report space on much larger partitions.
- Fixes to volume labels support.
- Added handlers for GP and SS exceptions.
- clang-16 support
- Lots of bug fixes.

## 1.6

Resurrected built-in COUNTRY support. You can now do:
COUNTRY=07,866
without specifying the path to country.sys.
dosemu2 is using that feature now, so the new release is due.

## 1.5

Maintenance release. Regression fixes, build changes etc.

- Initial PSP things moved to UMB.
- Fixed regressions with win31 install and others.
- Fixes to SWITCHES= and DOS= parsing.
- Split to normal and dev/devel packages.

## 1.4

Most of work went into making the kernel fully relocatable.
This required writing the new heap manager and updating the boot protocol.

- It is now possible to run fdpp directly from UMB, leaving entire
low mem free for your programs.
- Lots of build infrastructure work was done for buildroot integration.
- LBA support resurrected and enabled by default.

## 1.3

In this release we switched to ELF format. The binary kernel format
was dropped.
Implemented run-time relinking to fix the regressions from switching
to Tiny model that happened in 1.1. Unfortunately many programs expect
the DOS internal areas (like LoL and SDA) at the fixed locations
relative to DOS DS, rather than to query the addresses of the needed
structures. So fully compatible DOS cannot use Tiny model. We found
the way to (partially) convert FreeDOS from Tiny to Compact model at
run-time.
Also the first attempt is done to make the kernel fully relocatable.
So far that was achieved with a horrible hack. In the future that may
be amended, but who cares - the horrible hack just works. :)
Other than that, quite a few developments happened:

- long file seek extension from @ecm-pushbx
- rmdir fix from @andrewbird
- increase amount of file locks from 20 to 1024 in share
- revert mft work-around introduced in 1.2, as our run-time relinking
  solves that regression in a much better way.
- fix reading of large (>512 bytes) config.sys files
- compatibility fixes (Indy3 now works)

## 1.2

Stabilization release. Fixed many regressions and done lots more
work on file region locking. We hope the share/locking support
is now complete. Also worked around the QEMM's mft bug, so it
should work again.

## 1.1

Major development efforts again. Look out for new bugs...

- Switched to TINY model. This required a large redesign, and
  allowed to simplify the build process and reduce the build
  dependencies.
- Switched to lld linked from bfd ld.
- Lot of work on share. Region locking and share modes are now
  supported properly. Also share no longer allocates the precious
  DOS memory.
- Increment early-boot FILES (open file descriptors) from 8
  (actually 5) to 16. Previously all 5 descriptors could be
  exhausted when you use CHAIN, so it was not possible to, say,
  load country.sys from chained config. 3 extra descriptors
  were added too late for country.sys.
- Proper port of the country.sys loader.
- Fix SET command.

## 1.0 release

This is a first stable release of fdpp.<br/>
Not much to say here, because it "just works" - exactly the way
the (first) stable release should. The minimal set of features that
we needed from the first release, is also implemented.<br/>
Note that the main driving force behind this project, is dosemu2.
Which means that we only implement the features needed for dosemu2,
and currently they are all in place. So unless the scope of this
project is widened (like the use with other host kernels), no new
developments are planned, and the project will remain in a bugfixing
mode.

## rc-3

This is hopefully the last RC before 1.0 release.
Added COPR build support, the packages are here:<br/>
https://copr.fedorainfracloud.org/coprs/stsp/dosemu2/package/fdpp/<br/>
Below is the summary of few new additions:

- Confined (most of) freedos DOS memory writes.<br/> This is needed
  for the integration with some JIT engines, like the one in dosemu2.
  That JIT engine needs to be notified about the DOS memory writes.
- Support FILES= up to 256 (freedos only supports 128, then crashes)
- Fix "set volume serial number" fn for drive images/local drives
- Add support for COPR build system

## rc-2

Development is calming down.
We were busy with chasing the memory corruption bug
and also added a few cool new features below.

- Add share/locking support
- Add new config.sys directive INT0DIVZ. Setting it to OFF
  (it is ON by default) allows to avoid the "divizion-by-zero
  on fast CPU" problem in some games (Zool2).
  Note: it won't cure the famous "Runtime error 200" problem,
  even if it is very similar. You need a separate fix for that.
- Fix memory corruption in INSTALL= processing
- Propagate the initial CDS setup. No longer needed to redirect
  drives twice.
- Build improvements, add "make uninstall" target and more.
- Enable and use C++ RTTI - the last of the C++ features I
  wanted to never enable, but couldn't keep it that way.
  This essentially means that the desire of keeping fdpp
  within some "light-weight C++ subset" have completely failed.

## rc-1

The first release candidate.
The code base is believed to be very stable at that point,
and hence not many development is going on here.

- Rework Alpha Waves and Alone in the Dark hacks to be more
  in line with what MS-DOS does (these progs resize PSP to 0
  and then terminate it... among other bad things)
- Boot protocol extension to allow skipping the particular
  letters when assigning drives
- Stop reporting MSDOS-7.1 version to the programs, as FreeDOS
  did. Programs from that MSDOS distribution will now print
  "Incorrect DOS version"
- Fix nasty bug to allow Volkov Commander to work

## beta-9

Desperate stabilization efforts. :) Apart from overall stability,
we also added support for many games that do not work on FreeDOS.
Namely: TestDrive2, Tetris Classic, Elite Frontier, Empire Soccer,
Virtual Chess, Alone in the dark, Alpha Waves and more.
Around 70 commits with usual thanks to Andrew Bird.

- Implement int21/0x71a6 by the use of dosemu2's int2f/0x11a6 extension
- Port to FreeBSD, thanks to @PaddyMac for ssh
- Proper port of the fcb subsystem, with help and testing by @andrewbird
- Many fixes and extensions to compiler front-end
- Many improvements to freedos kernel for supporting games

## beta-8

Massive stabilization efforts. Many bugs and regressions were fixed.
Debug infrastructure improved.

- Fixed many bugs in handling of block devices
- LFN regression fix
- Export map file for debuggers
- Integration with valgrind's memcheck. Uncovered some bugs
- Extensions to boot protocol to get rid of dosemu2-specific calls
- Various crasher bugs fixed (with thanks to @andrewbird)

## beta-7

This marks the end of the short but very intensive development cycle
with lots of changes to freedos. Over 50 commits in 3 weeks.
Lots of regressions were introduced and are hopfully all now fixed,
so the release tagging is a due.

- Lots of work on fdkernel to use far pointers for data access.  [fdkernel]
  This allows to relocate the fdpp memory pool to UMB.
  Unfortunately freedos can't yet re-use the freed space.
- Fixes to relocation code for more source-level compatibility   [fdpp]
  with freedos.
- Fixed long-standing bug that caused the excessive size of the  [fdkernel]
  realmode kernel image. Now realmode part shrunk by 4k (from
  15 to 11k). Certainly that sets a record for the world-smallest
  realmode part of any available DOS.
- Thunk templates are no longer hand-written. They are now       [fdpp]
  generated by the use of m4sugar aka autoconf. We are likely
  the first project to use autoconf not to generate configure
  but rather for meta-programming.
- Set of bug fixes, both reported and found during code review.
  Code review is mostly over, so we are well set towards the
  stable releases hopefully (after a few more betas).

## beta-6

This beta release got a huge amount of work in all directions.
We are steadily heading towards a stable release.

- Completed the work of passing interrupt handling to prev handler [fdkernel]
  This improves the integration with dosemu2 which hooks some int vectors
  before booting DOS.
- Extension to load fdppconf.sys from any drive [boot]
- Extension to config.sys parser to prefix file names with
  "AT" symbol. [fdkernel]
  For example DEVICE=@c:\umb.sys. It checks the file existence before use.
- Improve CHAIN= support [fdkernel]
- Implement SWITCHES=/Y for single-stepping [fdkernel]
- Extension to pass strings via bootparams [boot]
- Extension to refer with #num to the bootparam strings [fdkernel]
  For example you can write SWITCHES=#0 to use string 0 from boot params.
- Increase default value of FILES from 16 to 64. [fdkernel]
  Needed for progs like GEOS.
- Made int21 handler reentrant [fdkernel]
  This appears needed as we now pass unhandled int21 calls to prev handler,
  which, in turn, can call another int21 (that's the life of emulators
  like dosemu2).
- Lots of work on memory management and object tracking [fdpp]
- Resurrect and port INSTALL= directive to fdpp [fdkernel]
- Completely rework thunks dispatching code to get rid of longjumps [fdpp]
- Fix initial environment corruption bug [fdkernel]


## beta-5

- Last bits of UMB saga [fdkernel]
- A lot of work on object tracking [fdpp run-time]
- Extensions to allow passing env vars and boot drives to config.sys [boot]
- Fix problems with drive formatting [fdkernel]
- Add shared headers to most int vectors [fdkernel]
- New config.sys name: fdppconf.sys (fdconfig.sys supported for compatibility)
- Ported int2f/08 handling to fdpp framework
- Started work to pass int vectors back to the prev handler [fdkernel]
- Extend compiler front-end to allow compiling the original freedos
  sources with much fewer manual instrumentation [compile-time templates]
- Reviewed all diffs in fdkernel, compared with the original and reverted
  all changes that are no longer needed. Now our freedos code is much
  closer to the original than it ever was, and our front-end can parse
  it all properly.
- Lots of fixes


## beta-4

All of the bug fixes, including regression fixes and freedos fixes.

- UMB handling in freedos got many fixes. As the result, we can
  now set a record of 332K of UMB space (with UMB at A0, among others).
  This is impossible on the original freedos, at the time of writing this.
- Crash with some versions of ld linker was fixed
- Added reboot support. This was the most difficult and intrusive
  change of this beta, even if sounds simple. :)
- Many fixes to both freedos and fdpp.


## beta-3

A very important milestone as we finally switched to ELF format!

- Switched to ELF tool-chain.
  The binary-supplied jwlib/jwlink are removed from the repo.
  Not everything is done an optimal way yet, the build is very
  complex and slow. We expect the enhancements of nasm in the future.
- Global clean-up of the repository, all FreeDOS remnants removed.
  We now have small and clean repository.
- Long-standing FreeDOS bug fixed that prevented UMB at A0
- Started supporting reboot (not finished yet)
- many fixes (GEOS now works reliably, smaller memory usage)


## beta-2:

We have around 60 commits since previous beta, and @andrewbird joined
the development. :)

- 32bit support resurrected (thanks Andrew for testing)
- better dosemu2 integration (boot sector extension to pass command.com drive)
- DPMI apps are now working reliably, windows-3.1 works
- valgrind and ubsan fixes (ubsan found some real bugs!)
- lots of work on memory management, object tracking and thunk dispatching
  (but not yet complete)
- drop all hops of C++-less run-time: libstdc++ is now and forever a mandatory
- FCB subsystem ported, with additional fixes from Andrew
- many bugfixes
