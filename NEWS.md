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
