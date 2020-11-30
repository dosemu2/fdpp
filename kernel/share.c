/* fdpp port by stsp. original copyrights below */

/*
	FreeDOS SHARE
	Copyright (c) 2000 Ronald B. Cemer under the GNU GPL
	You know the drill.
	If not, see www.gnu.org for details.  Read it, learn it, BE IT.  :-)
*/

/* #include <stdio.h> */ /* (fprintf removed...) */
/* #include <fcntl.h> */ /* Not used, using defines below... */
//#include <io.h>		/* write (what else?) */
//#include <stdlib.h>	/* _psp, NULL, malloc, free, atol, atoi */
//#include <dos.h>	/* MK_FP, FP_OFF, FP_SEG, int86, intdosx, */
			/* freemem, keep */
#include <string.h>	/* strchr, strlen, memset */

#include "portab.h"
#include "globals.h"
#include "proto.h"
#include "init-mod.h"
#define write(x,y,z) DosWrite(x,z,y)
#define interrupt

/* Changed by Eric Auer 5/2004: Squeezing executable size a bit -> */
/* Replaced fprint(stderr or stdout,...) by write(hand, buf, size) */
/* Keeps stream stuff and printf stuff outside the file and TSR... */

		/* ------------- DEFINES ------------- */
#define MUX_INT_NO 0x2f
#define MULTIPLEX_ID 0x10

	/* Valid values for openmode: */
#define OPEN_READ_ONLY   0
#define OPEN_WRITE_ONLY  1
#define OPEN_READ_WRITE  2

	/* Valid values for sharemode: */
#define SHARE_COMPAT     0
#define SHARE_DENY_ALL   1
#define SHARE_DENY_WRITE 2
#define SHARE_DENY_READ  3
#define SHARE_DENY_NONE  4

		/* ------------- TYPEDEFS ------------- */
	/* This table determines the action to take when attempting to open
	   a file.  The first array index is the sharing mode of a previous
	   open on the same file.  The second array index is the sharing mode
	   of the current open attempt on the same file.  Action codes are
	   defined as follows:
		   0 = open may proceed
		   1 = open fails with error code 05h
		   2 = open fails and an INT 24h is generated
		   3 = open proceeds if the file is read-only; otherwise fails
			   with error code (used only in exception table below)
		   4 = open proceeds if the file is read-only; otherwise fails
			   with INT 24H (used only in exception table below)
	   Exceptions to the rules are handled in the table
	   below, so this table only covers the general rules.
	*/
static unsigned char open_actions[5][5] = {
	{ 0, 1, 1, 1, 1 },
	{ 2, 1, 1, 1, 1 },
	{ 2, 1, 1, 1, 1 },
	{ 2, 1, 1, 1, 1 },
	{ 2, 1, 1, 1, 0 },
};

typedef struct {
	unsigned char first_sharemode;
	unsigned char first_openmode;
	unsigned char current_sharemode;
	unsigned char current_openmode;
	unsigned char action;
} open_action_exception_t;

static open_action_exception_t open_exceptions[] = {
	{ 0, 0, 2, 0, 3 },
	{ 0, 0, 4, 0, 3 },	/* compatibility-read/deny none-read, MED 08/2004 */

	{ 2, 0, 0, 0, 4 },
	{ 2, 0, 2, 0, 0 },
	{ 2, 0, 4, 0, 0 },
	{ SHARE_DENY_WRITE, OPEN_WRITE_ONLY, SHARE_DENY_READ, OPEN_READ_ONLY, 0 },
	{ SHARE_DENY_WRITE, OPEN_WRITE_ONLY, SHARE_DENY_NONE, OPEN_READ_ONLY, 0 },
	{ SHARE_DENY_WRITE, OPEN_READ_WRITE, SHARE_DENY_NONE, OPEN_READ_ONLY, 0 },

	{ 3, 0, 2, 1, 0 },
	{ 3, 0, 4, 1, 0 },
	{ 3, 1, 4, 1, 0 },
	{ 3, 2, 4, 1, 0 },
	{ SHARE_DENY_READ, OPEN_WRITE_ONLY, SHARE_DENY_READ, OPEN_WRITE_ONLY, 0 },

	{ 4, 0, 0, 0, 4 },
	{ 4, 0, 2, 0, 0 },
	{ 4, 0, 2, 1, 0 },
	{ 4, 0, 2, 2, 0 },
	{ 4, 1, 3, 0, 0 },
	{ 4, 1, 3, 1, 0 },
	{ 4, 1, 3, 2, 0 },
};

	/* One of these exists for each instance of an open file. */
typedef struct _file_t {
	char filename[128];		/* fully-qualified filename; "\0" if unused */
	unsigned char openmode;	/* 0=read-only, 1=write-only, 2=read-write */
	unsigned char sharemode;/* SHARE_COMPAT, etc... */
	unsigned char first_openmode;	/* openmode of first open */
	unsigned char first_sharemode;	/* sharemode of first open */
} file_t;

	/* One of these exists for each active lock region. */
typedef struct {
	unsigned char used;		/* Non-zero if this entry is used. */
	unsigned long start;	/* Beginning offset of locked region */
	unsigned long end;		/* Ending offset of locked region */
	unsigned short fileno;	/* file_table entry number */
} lock_t;


		/* ------------- GLOBALS ------------- */
//static char progname[9];
static const int file_table_size = 256;	/* # of file_t we can have */
static file_t file_table[file_table_size];
static const int lock_table_size = 20;	/* # of lock_t we can have */
static lock_t lock_table[lock_table_size];

		/* ------------- HOOK ------------- */
void int2F_10_handler(iregs FAR *iregs_p)
{
#define iregs (*iregs_p)
#define ax a.x
#define bx b.x
#define cx c.x
#define dx d.x

	iregs.flags &= ~FLG_CARRY;
	if (((iregs.ax >> 8) & 0xff) == MULTIPLEX_ID) {
		if ((iregs.ax & 0xff) == 0) {
				/* Installation check.  Return 0xff in AL. */
			iregs.ax |= 0xff;
			return;
		}
	}
	iregs.flags |= FLG_CARRY;

#undef iregs
#undef ax
#undef bx
#undef cx
#undef dx
}

static void remove_all_locks(int fileno) {
	int i;
	lock_t *lptr;

	for (i = 0; i < lock_table_size; i++) {
		lptr = &lock_table[i];
		if (lptr->fileno == fileno) lptr->used = 0;
	}
}

static void free_file_table_entry(int fileno) {
	file_table[fileno].filename[0] = '\0';
}

#define fnmatches(fn1, fn2) (strcmp(fn1, fn2) == 0)

static WORD do_open_check(
	int fileno,		/* file_table entry number */
	BOOL rdonly,
	__FAR(struct dhdr) lpDevice,
	UWORD ax)
{
	file_t *p;
	file_t *fptr = &file_table[fileno];
	int i, j, action = 0, foundexc;
	unsigned char current_sharemode = fptr->sharemode;
	unsigned char current_openmode = fptr->openmode;
	open_action_exception_t *excptr;

	fptr->first_sharemode = fptr->sharemode;
	fptr->first_openmode = fptr->openmode;
	for (i = 0; i < file_table_size; i++) {
		if (i == fileno) continue;
		p = &file_table[i];
		if (p->filename[0] == '\0') continue;
		if (!fnmatches(p->filename, fptr->filename)) continue;
		fptr->first_sharemode = p->first_sharemode;
		fptr->first_openmode = p->first_openmode;
			/* Look for exceptions to the general rules first. */
		foundexc = 0;
		for (j = 0;
			 j < (sizeof(open_exceptions)/sizeof(open_action_exception_t));
			 j++) {
			excptr = &open_exceptions[j];
			if (   (excptr->first_sharemode == fptr->first_sharemode)
				&& (excptr->current_sharemode == current_sharemode)
				&& (excptr->first_openmode == fptr->first_openmode)
				&& (excptr->current_openmode == current_openmode)  ) {
				foundexc = 1;
				action = excptr->action;
				break;
			}
		}
			/* If no exception to rules, use normal rules. */
		if (!foundexc)
			action = open_actions[fptr->first_sharemode][current_sharemode];
			/* Fail appropriately based on action. */
		switch (action) {

		case 0:		/* proceed with open */
			break;

		case 3:		/* succeed if file read-only, else fail with error 05h */
			if (rdonly)
				break;
			/* fall through */
		case 1:		/* fail with error code 05h */
			free_file_table_entry(fileno);
			/* XXX contrary to RBIL, testlock.exe expects error 20h here -stsp*/
			return DE_SHARING;

		case 4:		/* succeed if file read-only, else fail with int 24h */
			if (rdonly)
				break;
			/* fall through */
		case 2:		/* fail with int 24h */
			if (lpDevice) {
				UWORD flags;
				UWORD err;

				flags = 0x0e00;	/* disk I/O; fail allowed; data area */
				err = 0x0d;	/* sharing violation */
				if ( (fptr->filename[0]!='\0') && (fptr->filename[1]==':') )
					flags |= fptr->filename[0]-'A';
				free_file_table_entry(fileno);
				share_criterr(flags, err, lpDevice, ax);
			}
			return DE_SHARING;			/* sharing violation */
		}
		break;
	}
	return fileno;
}

	/* DOS calls this to see if it's okay to open the file.
	   Returns a file_table entry number to use (>= 0) if okay
	   to open.  Otherwise returns < 0 and may generate a critical
	   error.  If < 0 is returned, it is the negated error return
	   code, so DOS simply negates this value and returns it in
	   AX. */
WORD share_open_file
	(const char FAR *filename,/* FAR  pointer to fully qualified filename */
	 WORD openmode,		/* 0=read-only, 1=write-only, 2=read-write */
	 WORD sharemode,	/* SHARE_COMPAT, etc... */
	 BOOL rdonly,
	 __FAR(struct dhdr) lpDevice,
	 UWORD ax)
{

	int i, fileno = -1;
	file_t *fptr;

		/* Whack off unused bits in the share mode
		   in case we were careless elsewhere. */
	sharemode &= 0x07;

		/* Assume compatibility mode if invalid share mode. */
/* ??? IS THIS CORRECT ??? */
	if ( (sharemode < SHARE_COMPAT) || (sharemode > SHARE_DENY_NONE) )
		sharemode = SHARE_COMPAT;

		/* Whack off unused bits in the open mode
		   in case we were careless elsewhere. */
	openmode &= 0x03;

		/* Assume read-only mode if invalid open mode. */
/* ??? IS THIS CORRECT ??? */
	if ( (openmode < OPEN_READ_ONLY) || (openmode > OPEN_READ_WRITE) )
		openmode = OPEN_READ_ONLY;

	for (i = 0; i < file_table_size; i++) {
		if (file_table[i].filename[0] == '\0') {
			fileno = i;
			break;
		}
	}
	if (fileno == -1) return -1;
	fptr = &file_table[fileno];

		/* Copy the filename into ftpr->filename. */
	for (i = 0; i < sizeof(fptr->filename); i++) {
		if ((fptr->filename[i] = filename[i]) == '\0') break;
	}
	fptr->openmode = (unsigned char)openmode;
	fptr->sharemode = (unsigned char)sharemode;
		/* Do the sharing check and return fileno if
		   okay, or < 0 (and free the entry) if error. */
	return do_open_check(fileno, rdonly, lpDevice, ax);
}


BOOL share_open_check
	(const char FAR *filename)/* FAR  pointer to fully qualified filename */
{
	int i;
	for (i = 0; i < file_table_size; i++) {
		if (fnmatches(filename, file_table[i].filename))
			return 1;
	}
	return 0;
}

	/* DOS calls this to record the fact that it has successfully
	   closed a file, or the fact that the open for this file failed. */
void share_close_file(WORD fileno) {		/* file_table entry number */

	remove_all_locks(fileno);
	free_file_table_entry(fileno);
}

static WORD do_access_check(
	 WORD fileno,
	 UDWORD ofs,
	 UDWORD len,
	 BOOL allow_same_fd)
{
	int i;
	char *filename = file_table[fileno].filename;
	lock_t *lptr;
	unsigned long endofs = ofs + len;

	if (endofs < ofs) {
		endofs = 0xffffffffL;
		len = endofs-ofs;
	}

	if (len < 1L) return 0;

	for (i = 0; i < lock_table_size; i++) {
		lptr = &lock_table[i];
		if (   (lptr->used)
			&& (!allow_same_fd || fileno != lptr->fileno)
			&& (fnmatches(filename, file_table[lptr->fileno].filename))
			&& (   ( (ofs>=lptr->start) && (ofs<lptr->end) )
				|| ( (endofs>lptr->start) && (endofs<=lptr->end) )   )   ) {
			return -1;
		}
	}
	return 0;
}

	/* DOS calls this to determine whether it can access (read or
	   write) a specific section of a file.
	   Returns zero if okay to access or lock (no portion of the
	   region is already locked).  Otherwise returns non-zero and
	   generates a critical error (if allowcriter is non-zero).
	   If non-zero is returned, it is the negated return value for
	   the DOS call. */
WORD share_access_check(
	 WORD fileno,		/* file_table entry number */
	 UDWORD ofs,	/* offset into file */
	 UDWORD len,	/* length (in bytes) of region to access */
	 struct dhdr FAR * lpDevice,	/* allow a critical error to be generated */
	 UWORD ax)
{
	int err = do_access_check(fileno, ofs, len, 1);
	if (err) {
		if (lpDevice) {
			file_t *fptr = &file_table[fileno];
			UWORD flags;
			UWORD err;

			flags = 0x0e00;	/* disk I/O; fail allowed; data area */
			err = 0x0e;	/* lock violation */
			if ( (fptr->filename[0]!='\0') && (fptr->filename[1]==':') )
				flags |= fptr->filename[0]-'A';
			share_criterr(flags, err, lpDevice, ax);
		}
		return DE_ACCESS;		/* access denied */
	}
	return 0;
}

	/* DOS calls this to lock or unlock a specific section of a file.
	   Returns zero if successfully locked or unlocked.  Otherwise
	   returns non-zero.
	   If the return value is non-zero, it is the negated error
	   return code for the DOS 0x5c call. */
WORD share_lock_unlock(
	 WORD fileno,		/* file_table entry number */
	 UDWORD ofs,	/* offset into file */
	 UDWORD len,	/* length (in bytes) of region to lock or unlock */
	 WORD unlock) {		/* non-zero to unlock; zero to lock */

	int i;
	lock_t *lptr;
	unsigned long endofs = ofs + len;

    if (endofs < ofs) {
		endofs = 0xffffffffL;
		len = endofs-ofs;
	}

	if (len < 1L) return 0;

    /* there was a error in the code below preventing any other
     than the first locked region to be unlocked (japheth, 09/2005) */

	if (unlock) {
		for (i = 0; i < lock_table_size; i++) {
			lptr = &lock_table[i];
			if (   (lptr->used)
				&& (lptr->fileno == fileno)
				&& (lptr->start == ofs)
				&& (lptr->end == endofs)   ) {
				lptr->used = 0;
				return 0;
			}
		}
			/* Not already locked by us; can't unlock. */
		return DE_LOCK;		/* lock violation */
	} else {
		if (do_access_check(fileno, ofs, len, 0)) {
				/* Already locked; can't lock. */
			return DE_LOCK;		/* lock violation */
		}
		for (i = 0; i < lock_table_size; i++) {
			lptr = &lock_table[i];
			if (!lptr->used) {
				lptr->used = 1;
				lptr->start = ofs;
				lptr->end = ofs+(unsigned long)len;
				lptr->fileno = fileno;
				return 0;
			}
		}
		return -(0x24);		/* sharing buffer overflow */
	}
}

WORD share_is_file_open(const char FAR * filename)
{
	int i;

	for (i = 0; i < file_table_size; i++) {
		if (fnmatches(filename, file_table[i].filename))
			return 1;
	}
	return 0;
}

		/* ------------- INIT ------------- */
int share_init(void)
{
	memset(file_table, 0, file_table_size * sizeof(file_t));
	memset(lock_table, 0, lock_table_size * sizeof(lock_t));
	return 0;
}
