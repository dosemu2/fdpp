/****************************************************************/
/*                                                              */
/*                         dosnames.c                           */
/*                           DOS-C                              */
/*                                                              */
/*    Generic parsing functions for file name specifications    */
/*                                                              */
/*                      Copyright (c) 1994                      */
/*                      Pasquale J. Villani                     */
/*                      All Rights Reserved                     */
/*                                                              */
/* This file is part of DOS-C.                                  */
/*                                                              */
/* DOS-C is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU General Public License  */
/* as published by the Free Software Foundation; either version */
/* 2, or (at your option) any later version.                    */
/*                                                              */
/* DOS-C is distributed in the hope that it will be useful, but */
/* WITHOUT ANY WARRANTY; without even the implied warranty of   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See    */
/* the GNU General Public License for more details.             */
/*                                                              */
/* You should have received a copy of the GNU General Public    */
/* License along with DOS-C; see the file COPYING.  If not,     */
/* write to the Free Software Foundation, 675 Mass Ave,         */
/* Cambridge, MA 02139, USA.                                    */
/*                                                              */
/****************************************************************/

#include "portab.h"

#ifdef VERSION_STRINGS
static BYTE *dosnamesRcsId = "$Id$";
#endif

#include "globals.h"

char _DirWildNameChars[] = "*?./\\\"[]:|<>+=;,";

#define PathSep(c) ((c)=='/'||(c)=='\\')
#define DriveChar(c) (((c)>='A'&&(c)<='Z')||((c)>='a'&&(c)<='z'))
#define DirChar(c)  (!strchr(_DirWildNameChars+5, (c)))
#define WildChar(c) (!strchr(_DirWildNameChars+2, (c)))
#define NameChar(c) (!strchr(_DirWildNameChars, (c)))

VOID XlateLcase(BYTE * szFname, COUNT nChars);
VOID DosTrimPath(BYTE * lpszPathNamep);

/* Should be converted to a portable version after v1.0 is released.    */
#if 0
VOID XlateLcase(BYTE * szFname, COUNT nChars)
{
  while (nChars--)
  {
    if (*szFname >= 'a' && *szFname <= 'z')
      *szFname -= ('a' - 'A');
    ++szFname;
  }
}
#endif

VOID SpacePad(BYTE * szString, COUNT nChars)
{
  REG COUNT i;

  for (i = strlen(szString); i < nChars; i++)
    szString[i] = ' ';
}

/*
    MSD durring an FindFirst search string looks like this;
    (*), & (.)  == Current directory *.*
    (\)         == Root directory *.*
    (..)        == Back one directory *.*

    This always has a "truename" as input, so we may do some shortcuts
 */
COUNT ParseDosName(BYTE * lpszFileName,
                   COUNT * pnDrive,
                   BYTE * pszDir,
                   BYTE * pszFile,
                   BYTE * pszExt,
                   BOOL bAllowWildcards)
{
  COUNT nDirCnt,
    nFileCnt,
    nExtCnt;
  BYTE *lpszLclDir,
    *lpszLclFile,
    *lpszLclExt;

  /* Initialize the users data fields                             */
  if (pszDir)
    *pszDir = '\0';
  if (pszFile)
    *pszFile = '\0';
  if (pszExt)
    *pszExt = '\0';
  lpszLclFile = lpszLclExt = lpszLclDir = 0;
  nDirCnt = nFileCnt = nExtCnt = 0;

  /* found a drive, fetch it and bump pointer past drive  */
  /* NB: this code assumes ASCII                          */
  if (pnDrive)
    *pnDrive = *lpszFileName - 'A';
  lpszFileName += 2;
  if (!pszDir && !pszFile && !pszExt)
    return SUCCESS;

  /* Now see how long a directory component we have.              */
  lpszLclDir = lpszLclFile = lpszFileName;
  while (DirChar(*lpszFileName))
  {
    if (*lpszFileName == '\\')
      lpszLclFile = lpszFileName + 1;
    ++lpszFileName;
  }
  nDirCnt = FP_OFF(lpszLclFile) - FP_OFF(lpszLclDir);
  /* Fix lengths to maximums allowed by MS-DOS.                   */
  if (nDirCnt > PARSE_MAX-1)
    nDirCnt = PARSE_MAX-1;

  /* Parse out the file name portion.                             */
  lpszFileName = lpszLclFile;
  while (bAllowWildcards ? WildChar(*lpszFileName) : NameChar(*lpszFileName))
  {
    ++nFileCnt;
    ++lpszFileName;
  }

  if (nFileCnt == 0)
/* Lixing Yuan Patch */
     if (bAllowWildcards)  /* for find first */
     {
       if (*lpszFileName != '\0')
         return DE_FILENOTFND;
       if (nDirCnt == 1) /* for d:\ */
         return DE_NFILES;
       if (pszDir)
       {
         memcpy(pszDir, lpszLclDir, nDirCnt);
         pszDir[nDirCnt] = '\0';
       }
       if (pszFile)
         memcpy(pszFile, "????????", FNAME_SIZE+1);
       if (pszExt)
         memcpy(pszExt, "???", FEXT_SIZE+1);
       return SUCCESS;
     }
   else
    return DE_FILENOTFND;


  /* Now we have pointers set to the directory portion and the    */
  /* file portion.  Now determine the existance of an extension.  */
  lpszLclExt = lpszFileName;
  if ('.' == *lpszFileName)
  {
    lpszLclExt = ++lpszFileName;
    while (*lpszFileName)
    {
      if (bAllowWildcards ? WildChar(*lpszFileName) : NameChar(*lpszFileName))
      {
        ++nExtCnt;
        ++lpszFileName;
      }
      else{
        return DE_FILENOTFND;
        }
    }
  }
  else if (*lpszFileName)
    return DE_FILENOTFND;

  /* Finally copy whatever the user wants extracted to the user's */
  /* buffers.                                                     */
  if (pszDir)
  {
    memcpy(pszDir, lpszLclDir, nDirCnt);
    pszDir[nDirCnt] = '\0';
  }
  if (pszFile)
  {
    memcpy(pszFile, lpszLclFile, nFileCnt);
    pszFile[nFileCnt] = '\0';
  }
  if (pszExt)
  {
    memcpy(pszExt, lpszLclExt, nExtCnt);
    pszExt[nExtCnt] = '\0';
  }

  /* Clean up before leaving                              */

  return SUCCESS;
}

#if 0
/* not necessary anymore because of truename */
COUNT ParseDosPath(BYTE * lpszFileName,
                   COUNT * pnDrive,
                   BYTE * pszDir,
                   BYTE * pszCurPath)
{
  COUNT nDirCnt,
    nPathCnt;
  BYTE *lpszLclDir,
   *pszBase = pszDir;

  /* Initialize the users data fields                             */
  *pszDir = '\0';
  lpszLclDir = 0;
  nDirCnt = nPathCnt = 0;

  /* Start by cheking for a drive specifier ...                   */
  if (DriveChar(*lpszFileName) && ':' == lpszFileName[1])
  {
    /* found a drive, fetch it and bump pointer past drive  */
    /* NB: this code assumes ASCII                          */
    if (pnDrive)
    {
      *pnDrive = *lpszFileName - 'A';
      if (*pnDrive > 26)
        *pnDrive -= ('a' - 'A');
    }
    lpszFileName += 2;
  }
  else
  {
    if (pnDrive)
    {
      *pnDrive = -1;
    }
  }

  lpszLclDir = lpszFileName;
  if (!PathSep(*lpszLclDir))
  {
    fstrncpy(pszDir, pszCurPath, PARSE_MAX - 1);        /*TE*/
    nPathCnt = fstrlen(pszCurPath);
    if (!PathSep(pszDir[nPathCnt - 1]) && nPathCnt < PARSE_MAX - 1) /*TE*/
      pszDir[nPathCnt++] = '\\';
    if (nPathCnt > PARSE_MAX)
      nPathCnt = PARSE_MAX;
    pszDir += nPathCnt;
  }

  /* Now see how long a directory component we have.              */
  while (NameChar(*lpszFileName)
         || PathSep(*lpszFileName)
         || '.' == *lpszFileName)
  {
    ++nDirCnt;
    ++lpszFileName;
  }

  /* Fix lengths to maximums allowed by MS-DOS.                   */
  if ((nDirCnt + nPathCnt) > PARSE_MAX - 1)     /*TE*/
    nDirCnt = PARSE_MAX - 1 - nPathCnt;

  /* Finally copy whatever the user wants extracted to the user's */
  /* buffers.                                                     */
  if (pszDir)
  {
    memcpy(pszDir, lpszLclDir, nDirCnt);
    pszDir[nDirCnt] = '\0';
  }

  /* Clean up before leaving                              */
  DosTrimPath(pszBase);

  /* Before returning to the user, eliminate any useless          */
  /* trailing "\\." since the path prior to this is sufficient.   */
  nPathCnt = strlen(pszBase);
  if (2 == nPathCnt)            /* Special case, root           */
  {
    if (!strcmp(pszBase, "\\."))
      pszBase[1] = '\0';
  }
  else if (2 < nPathCnt)
  {
    if (!strcmp(&pszBase[nPathCnt - 2], "\\."))
      pszBase[nPathCnt - 2] = '\0';
  }

  return SUCCESS;
}

VOID DosTrimPath(BYTE * lpszPathNamep)
{
  BYTE *lpszLast,
    *lpszNext,
    *lpszRoot = NULL;
  COUNT nChars,
    flDotDot;

  /* First, convert all '/' to '\'.  Look for root as we scan     */
  if (*lpszPathNamep == '\\')
    lpszRoot = lpszPathNamep;
  for (lpszNext = lpszPathNamep; *lpszNext; ++lpszNext)
  {
    if (*lpszNext == '/')
      *lpszNext = '\\';
    if (!lpszRoot &&
        *lpszNext == ':' && *(lpszNext + 1) == '\\')
      lpszRoot = lpszNext + 1;
  }

                                                /* NAMEMAX + 2, must include C: TE*/
  for (lpszLast = lpszNext = lpszPathNamep, nChars = 0;
       *lpszNext != '\0' && nChars < NAMEMAX+2;)
  {
    /* Initialize flag for loop.                            */
    flDotDot = FALSE;

    /* If we are at a path seperator, check for extra path  */
    /* seperator, '.' and '..' to reduce.                   */
    if (*lpszNext == '\\')
    {
      /* If it's '\', just move everything down one.  */
      if (*(lpszNext + 1) == '\\')
        fstrncpy(lpszNext, lpszNext + 1, NAMEMAX);
      /* also check for '.' and '..' and move down    */
      /* as appropriate.                              */
      else if (*(lpszNext + 1) == '.')
      {
        if (*(lpszNext + 2) == '.'
            && !(*(lpszNext + 3)))
        {
          /* At the end, just truncate    */
          /* and exit.                    */
          if (lpszLast == lpszRoot)
            *(lpszLast + 1) = '\0';
          else
            *lpszLast = '\0';
          return;
        }

        if (*(lpszNext + 2) == '.'
            && *(lpszNext + 3) == '\\')
        {
          fstrncpy(lpszLast, lpszNext + 3, NAMEMAX);
          /* bump back to the last        */
          /* seperator.                   */
          lpszNext = lpszLast;
          /* set lpszLast to the last one */
          if (lpszLast <= lpszPathNamep)
            continue;
          do
          {
            --lpszLast;
          }
          while (lpszLast != lpszPathNamep
                 && *lpszLast != '\\');
          flDotDot = TRUE;
        }
        /* Note: we skip strange stuff that     */
        /* starts with '.'                      */
        else if (*(lpszNext + 2) == '\\')
        {
          fstrncpy(lpszNext, lpszNext + 2, NAMEMAX);
	  flDotDot = TRUE;
        }
        /* If we're at the end of a string,     */
        /* just exit.                           */
        else if (*(lpszNext + 2) == NULL)
        {
          return;
        }
        /*
           Added this "else" because otherwise we might not pass
           any of the foregoing tests, as in the case where the
           incoming string refers to a suffix only, like ".bat"

           -SRM
         */
        else
        {
          lpszLast = lpszNext++;
        }
      }
      else
      {
        /* No '.' or '\' so mark it and bump    */
        /* past                                 */
        lpszLast = lpszNext++;
        continue;
      }

      /* Done.  Now set last to next to mark this     */
      /* instance of path seperator.                  */
      if (!flDotDot)
        lpszLast = lpszNext;
    }
    else
      /* For all other cases, bump lpszNext for the   */
      /* next check                                   */
      ++lpszNext;
  }
}
#endif

/*
 * Log: dosnames.c,v - for newer log entries do "cvs log dosnames.c"
 *
 * Revision 1.2  2000/05/08 04:29:59  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.4  2000/03/31 05:40:09  jtabor
 * Added Eric W. Biederman Patches
 *
 * Revision 1.3  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.2  1999/04/04 18:51:43  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:41:54  jprice
 * New version without IPL.SYS
 *
 * Revision 1.4  1999/02/02 04:40:49  jprice
 * Steve Miller fixed a bug with doing "cd ." would lock the machine.
 *
 * Revision 1.3  1999/02/01 01:43:28  jprice
 * Fixed findfirst function to find volume label with Windows long filenames
 *
 * Revision 1.2  1999/01/22 04:15:28  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:00  jprice
 * Imported sources
 *
 *
 *    Rev 1.8   22 Jan 1998  4:09:00   patv
 * Fixed pointer problems affecting SDA
 *
 *    Rev 1.7   04 Jan 1998 23:14:38   patv
 * Changed Log for strip utility
 *
 *    Rev 1.6   03 Jan 1998  8:36:04   patv
 * Converted data area to SDA format
 *
 *    Rev 1.5   16 Jan 1997 12:46:36   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.4   29 May 1996 21:15:12   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.3   19 Feb 1996  3:20:08   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.2   01 Sep 1995 17:48:44   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:50:26   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:05:56   patv
 * Initial revision.
 *
 */

