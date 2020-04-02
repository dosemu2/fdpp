/* Copyright stsp, FDPP project */

#ifndef MEMTYPE_H
#define MEMTYPE_H

/* TODO: add allocated, freed etc types, dont hesitate */
enum FdMemType {
    FD_MEM_NORMAL,
    FD_MEM_NOACCESS,
    FD_MEM_READONLY,
    FD_MEM_READEXEC,
    FD_MEM_UNINITIALIZED,
};

#endif
