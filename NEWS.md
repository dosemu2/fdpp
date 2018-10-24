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
