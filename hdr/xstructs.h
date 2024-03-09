/****************************************************************/
/*                                                              */
/*                        xstructs.h                            */
/*                                                              */
/*                Extended DOS 7.0+ structures                  */
/*                                                              */
/****************************************************************/

#ifdef MAIN
#ifdef VERSION_STRINGS
static BYTE *XStructs_hRcsId =
    "$Id: xstructs.h 1457 2009-06-26 20:00:41Z bartoldeman $";
#endif
#endif

struct xdpbdata {
  UWORD xdd_dpbsize;
  SYM_MEMB(xdpbdata, struct dpb, xdd_dpb);
} PACKED;
ANNOTATE_SIZE(struct xdpbdata, 0x3f);

struct xfreespace {
  UWORD xfs_datasize;           /* size of this structure                */
  union {
    UWORD requested;            /* requested structure version           */
    UWORD actual;               /* actual structure version              */
  } xfs_version;
  ULONG xfs_clussize;           /* number of sectors per cluster         */
  ULONG xfs_secsize;            /* number of bytes per sector            */
  ULONG xfs_freeclusters;       /* number of available clusters          */
  ULONG xfs_totalclusters;      /* total number of clusters on the drive */
  ULONG xfs_freesectors;        /* number of physical sectors available  */
  ULONG xfs_totalsectors;       /* total number of physical sectors      */
  ULONG xfs_freeunits;          /* number of available allocation units  */
  ULONG xfs_totalunits;         /* total allocation units                */
  UBYTE xfs_reserved[8];
};
ANNOTATE_SIZE(struct xfreespace, 0x2c);

struct xdpbforformat {
  UWORD xdff_datasize;          /* size of this structure                */
  union {
    UWORD requested;            /* requested structure version           */
    UWORD actual;               /* actual structure version              */
  } xdff_version;
  UDWORD xdff_function;         /* function number:
                                   00h invalidate DPB counts
                                   01h rebuild DPB from BPB
                                   02h force media change
                                   03h get/set active FAT number and mirroring
                                   04h get/set root directory cluster number
                                 */
  union {
    struct {
      DWORD nfreeclst;          /* # free clusters
                                   (-1 - unknown, 0 - don't change) */
      DWORD cluster;            /* cluster # of first free
                                   (-1 - unknown, 0 - don't change) */
      UDWORD reserved[2];
    } setdpbcounts;

    struct {
      UDWORD unknown;
      __DOSFAR(bpb) bpbp;
      UDWORD reserved[2];
    } rebuilddpb;

    struct {
      DWORD _new;                /* new active FAT/mirroring state, or -1 to get
                                   bits 3-0: the 0-based FAT number of the active FAT
                                   bits 6-4: reserved (0)
                                   bit 7: do not mirror active FAT to inactive FATs
                                   or:
                                   set new root directory cluster, -1 - get current
                                 */
      DWORD old;                 /* previous active FAT/mirroring state (as above)
                                    or
                                    get previous root directory cluster
                                 */
      UDWORD reserved[2];
    } setget;
  } xdff_f;
};
ANNOTATE_SIZE(struct xdpbforformat, 0x18);

struct xgetvolumeinfo {
  UBYTE version;
  UBYTE size;        /* size of this structure */
  UBYTE namelen;     /* length of file system type name */
  UBYTE pad;
  struct {
    UWORD std;       /* 15..0 as per RBIL */
    UWORD ext;       /* 31..16 for extension */
  } PACKED flags;
  UWORD maxfilenamelen;
  UWORD maxpathlen;
  char name[16];     /* file system type name */
} PACKED;
ANNOTATE_SIZE(struct xgetvolumeinfo, 28);

COUNT DosGetExtFree(__FAR(BYTE) DriveString, __FAR(struct xfreespace) xfsp);
