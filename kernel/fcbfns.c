/****************************************************************/
/*                                                              */
/*                          fcbfns.c                            */
/*                                                              */
/*           Old CP/M Style Function Handlers for Kernel        */
/*                                                              */
/*                      Copyright (c) 1995                      */
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
/****************************************************************/

#include "portab.h"
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId =
    "$Id$";
#endif

#define FCB_SUCCESS     0
#define FCB_ERR_NODATA  1
#define FCB_ERR_EOF     3
#define FCB_ERR_WRITE   1
#define D_ALL   D_NORMAL | D_RDONLY | D_HIDDEN | D_SYSTEM | D_DIR | D_ARCHIVE

#ifdef PROTO
fcb FAR *ExtFcbToFcb(xfcb FAR * lpExtFcb);
fcb FAR *CommonFcbInit(xfcb FAR * lpExtFcb, BYTE * pszBuffer,
                       COUNT * pCurDrive);
void FcbNameInit(fcb FAR * lpFcb, BYTE * pszBuffer, COUNT * pCurDrive);
VOID FcbNextRecord(fcb FAR * lpFcb);
BOOL FcbCalcRec(xfcb FAR * lpXfcb);
#else
fcb FAR *ExtFcbToFcb();
fcb FAR *CommonFcbInit();
void FcbNameInit();
VOID FcbNextRecord();
BOOL FcbCalcRec();
#endif

#define TestCmnSeps(lpFileName) (strchr(":<|>+=,", *lpFileName) != NULL)
#define TestFieldSeps(lpFileName) (*(lpFileName) <= ' ' || strchr("/\"[]<>|.", *lpFileName) != NULL)

static dmatch Dmatch;

VOID FatGetDrvData(UCOUNT drive, UCOUNT FAR * spc, UCOUNT FAR * bps,
                   UCOUNT FAR * nc, BYTE FAR ** mdp)
{
  UCOUNT navc;

  /* get the data available from dpb                       */
  *nc = 0xffff;                 /* pass 0xffff to skip free count */
  if (DosGetFree((UBYTE) drive, spc, &navc, bps, nc))
    /* Point to the media desctriptor for this drive               */
    *mdp = (BYTE FAR *) & (CDSp->cds_table[drive].cdsDpb->dpb_mdb);
}

#define PARSE_SEP_STOP          0x01
#define PARSE_DFLT_DRIVE        0x02
#define PARSE_BLNK_FNAME        0x04
#define PARSE_BLNK_FEXT         0x08

#define PARSE_RET_NOWILD        0
#define PARSE_RET_WILD          1
#define PARSE_RET_BADDRIVE      0xff

#ifndef IPL
WORD FcbParseFname(int wTestMode, BYTE FAR ** lpFileName, fcb FAR * lpFcb)
{
  COUNT nIndex;
  WORD wRetCodeName, wRetCodeExt;

  /* pjv -- ExtFcbToFcb?                                          */
  /* Start out with some simple stuff first.  Check if we are     */
  /* going to use a default drive specificaton.                   */
  if (!(wTestMode & PARSE_DFLT_DRIVE))
    lpFcb->fcb_drive = FDFLT_DRIVE;
  if (!(wTestMode & PARSE_BLNK_FNAME))
  {
    for (nIndex = 0; nIndex < FNAME_SIZE; ++nIndex)
      lpFcb->fcb_fname[nIndex] = ' ';
  }
  if (!(wTestMode & PARSE_BLNK_FEXT))
  {
    for (nIndex = 0; nIndex < FEXT_SIZE; ++nIndex)
      lpFcb->fcb_fext[nIndex] = ' ';
  }

  /* Undocumented behavior, set record number & record size to 0  */
  lpFcb->fcb_cublock = lpFcb->fcb_recsiz = 0;

  if (!(wTestMode & PARSE_SEP_STOP))
  {
    *lpFileName = ParseSkipWh(*lpFileName);
    if (TestCmnSeps(*lpFileName))
      ++ * lpFileName;
  }

  /* Undocumented "feature," we skip white space anyway           */
  *lpFileName = ParseSkipWh(*lpFileName);

  /* Now check for drive specification                            */
  if (*(*lpFileName + 1) == ':')
  {
    /* non-portable construct to be changed                 */
    REG UBYTE Drive = DosUpFChar(**lpFileName) - 'A';

    if (Drive >= lastdrive)
      return PARSE_RET_BADDRIVE;

    lpFcb->fcb_drive = Drive + 1;
    *lpFileName += 2;
  }

  /* special cases: '.' and '..' */
  if (**lpFileName == '.')
  {
    lpFcb->fcb_fname[0] = '.';
    ++*lpFileName;
    if (**lpFileName == '.')
    {
      lpFcb->fcb_fname[1] = '.';
      ++*lpFileName;
    }
    return PARSE_RET_NOWILD;
  }

  /* Now to format the file name into the string                  */
  *lpFileName =
      GetNameField(*lpFileName, (BYTE FAR *) lpFcb->fcb_fname, FNAME_SIZE,
                   (BOOL *) & wRetCodeName);

  /* Do we have an extension? If do, format it else return        */
  if (**lpFileName == '.')
    *lpFileName =
        GetNameField(++*lpFileName, (BYTE FAR *) lpFcb->fcb_fext,
                     FEXT_SIZE, (BOOL *) & wRetCodeExt);

  return (wRetCodeName | wRetCodeExt) ? PARSE_RET_WILD : PARSE_RET_NOWILD;
}

BYTE FAR *ParseSkipWh(BYTE FAR * lpFileName)
{
  while (*lpFileName == ' ' || *lpFileName == '\t')
    ++lpFileName;
  return lpFileName;
}

#if 0                           /* defined above */
BOOL TestCmnSeps(BYTE FAR * lpFileName)
{
  BYTE *pszTest, *pszCmnSeps = ":<|>+=,";

  for (pszTest = pszCmnSeps; *pszTest != '\0'; ++pszTest)
    if (*lpFileName == *pszTest)
      return TRUE;
  return FALSE;
}
#endif

#if 0
BOOL TestFieldSeps(BYTE FAR * lpFileName)
{
  BYTE *pszTest, *pszCmnSeps = "/\"[]<>|.";

  /* Another non-portable construct                               */
  if (*lpFileName <= ' ')
    return TRUE;

  for (pszTest = pszCmnSeps; *pszTest != '\0'; ++pszTest)
    if (*lpFileName == *pszTest)
      return TRUE;
  return FALSE;
}
#endif

BYTE FAR *GetNameField(BYTE FAR * lpFileName, BYTE FAR * lpDestField,
                       COUNT nFieldSize, BOOL * pbWildCard)
{
  COUNT nIndex = 0;
  BYTE cFill = ' ';

  *pbWildCard = FALSE;
  while (*lpFileName != '\0' && !TestFieldSeps(lpFileName)
         && nIndex < nFieldSize)
  {
    if (*lpFileName == ' ')
      break;
    if (*lpFileName == '*')
    {
      *pbWildCard = TRUE;
      cFill = '?';
      ++lpFileName;
      break;
    }
    if (*lpFileName == '?')
      *pbWildCard = TRUE;
    *lpDestField++ = DosUpFChar(*lpFileName++);
    ++nIndex;
  }

  /* Blank out remainder of field on exit                         */
  for (; nIndex < nFieldSize; ++nIndex)
    *lpDestField++ = cFill;
  return lpFileName;
}

static VOID FcbNextRecord(fcb FAR * lpFcb)
{
  if (++lpFcb->fcb_curec > 128)
  {
    lpFcb->fcb_curec = 0;
    ++lpFcb->fcb_cublock;
  }
}

static ULONG FcbRec(VOID)
{
  UWORD tmp = 128;

  return ((ULONG) lpFcb->fcb_cublock * tmp) + lpFcb->fcb_curec;
}

BOOL FcbRead(xfcb FAR * lpXfcb, COUNT * nErrorCode)
{
  sft FAR *s;
  LONG lPosit;
  COUNT nRead;
  psp FAR *p = MK_FP(cu_psp, 0);

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  /* Get the SFT block that contains the SFT      */
  if ((s = idx_to_sft(lpFcb->fcb_sftno)) == (sft FAR *) - 1)
    return FALSE;

  /* If this is not opened another error          */
  if (s->sft_count == 0)
    return FALSE;

  /* Now update the fcb and compute where we need to position     */
  /* to.                                                          */
  lPosit = FcbRec() * lpFcb->fcb_recsiz;
  if (SftSeek(s, lPosit, 0) != SUCCESS)
  {
    *nErrorCode = FCB_ERR_EOF;
    return FALSE;
  }

  /* Do the read                                                  */
  nRead = DosReadSft(s, lpFcb->fcb_recsiz, p->ps_dta, nErrorCode);

  /* Now find out how we will return and do it.                   */
  if (nRead == lpFcb->fcb_recsiz)
  {
    *nErrorCode = FCB_SUCCESS;
    FcbNextRecord(lpFcb);
    return TRUE;
  }
  else if (nRead < 0)
  {
    *nErrorCode = FCB_ERR_EOF;
    return TRUE;
  }
  else if (nRead == 0)
  {
    *nErrorCode = FCB_ERR_NODATA;
    return FALSE;
  }
  else
  {
    fmemset(&p->ps_dta[nRead], 0, lpFcb->fcb_recsiz - nRead);
    *nErrorCode = FCB_ERR_EOF;
    FcbNextRecord(lpFcb);
    return FALSE;
  }
}

BOOL FcbWrite(xfcb FAR * lpXfcb, COUNT * nErrorCode)
{
  sft FAR *s;
  LONG lPosit;
  COUNT nWritten;
  psp FAR *p = MK_FP(cu_psp, 0);

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  /* Get the SFT block that contains the SFT      */
  if ((s = idx_to_sft(lpFcb->fcb_sftno)) == (sft FAR *) - 1)
    return FALSE;

  /* If this is not opened another error          */
  if (s->sft_count == 0)
    return FALSE;

  /* Now update the fcb and compute where we need to position     */
  /* to.                                                          */
  lPosit = FcbRec() * lpFcb->fcb_recsiz;
  if (SftSeek(s, lPosit, 0) != SUCCESS)
  {
    *nErrorCode = FCB_ERR_EOF;
    return FALSE;
  }

  nWritten = DosWriteSft(s, lpFcb->fcb_recsiz, p->ps_dta, nErrorCode);

  /* Now find out how we will return and do it.                   */
  if (nWritten == lpFcb->fcb_recsiz)
  {
    lpFcb->fcb_fsize = s->sft_size;
    FcbNextRecord(lpFcb);
    *nErrorCode = FCB_SUCCESS;
    return TRUE;
  }
  else if (nWritten <= 0)
  {
    *nErrorCode = FCB_ERR_WRITE;
    return TRUE;
  }
  *nErrorCode = FCB_ERR_WRITE;
  return FALSE;
}

BOOL FcbGetFileSize(xfcb FAR * lpXfcb)
{
  COUNT FcbDrive, hndl;

  /* Build a traditional DOS file name                            */
  lpFcb = CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);

  /* check for a device                                           */
  if (IsDevice(SecPathName) || (lpFcb->fcb_recsiz == 0))
  {
    return FALSE;
  }
  hndl = DosOpen(SecPathName, O_RDONLY);
  if (hndl >= 0)
  {
    LONG fsize;

    /* Get the size                                         */
    fsize = DosGetFsize(hndl);

    /* compute the size and update the fcb                  */
    lpFcb->fcb_rndm = fsize / lpFcb->fcb_recsiz;
    if ((fsize % lpFcb->fcb_recsiz) != 0)
      ++lpFcb->fcb_rndm;

    /* close the file and leave                             */
    return DosClose(hndl) == SUCCESS;
  }
  else
    return FALSE;
}

BOOL FcbSetRandom(xfcb FAR * lpXfcb)
{
  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  /* Now update the fcb and compute where we need to position     */
  /* to. */
  lpFcb->fcb_rndm = FcbRec();

  return TRUE;
}

BOOL FcbCalcRec(xfcb FAR * lpXfcb)
{

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  /* Now update the fcb and compute where we need to position     */
  /* to.                                                          */
  lpFcb->fcb_cublock = lpFcb->fcb_rndm / 128;
  lpFcb->fcb_curec = lpFcb->fcb_rndm & 127;

  return TRUE;
}

BOOL FcbRandomBlockRead(xfcb FAR * lpXfcb, COUNT nRecords,
                        COUNT * nErrorCode)
{
  FcbCalcRec(lpXfcb);

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  do
    FcbRead(lpXfcb, nErrorCode);
  while ((--nRecords > 0) && (*nErrorCode == 0));

  /* Now update the fcb                                           */
  lpFcb->fcb_rndm = FcbRec();

  return TRUE;
}

BOOL FcbRandomBlockWrite(xfcb FAR * lpXfcb, COUNT nRecords,
                         COUNT * nErrorCode)
{
  FcbCalcRec(lpXfcb);

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  do
    FcbWrite(lpXfcb, nErrorCode);
  while ((--nRecords > 0) && (*nErrorCode == 0));

  /* Now update the fcb                                           */
  lpFcb->fcb_rndm = FcbRec();

  return TRUE;
}

BOOL FcbRandomIO(xfcb FAR * lpXfcb, COUNT * nErrorCode,
                 BOOL(*FcbFunc) (xfcb FAR *, COUNT *))
{
  UWORD uwCurrentBlock;
  UBYTE ucCurrentRecord;

  FcbCalcRec(lpXfcb);

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  uwCurrentBlock = lpFcb->fcb_cublock;
  ucCurrentRecord = lpFcb->fcb_curec;

  (*FcbFunc) (lpXfcb, nErrorCode);

  lpFcb->fcb_cublock = uwCurrentBlock;
  lpFcb->fcb_curec = ucCurrentRecord;
  return TRUE;
}

/*
static sft FAR *FcbGetFreeSft(COUNT * sft_idx)
see get_free_sft in dosfns.c
*/

BOOL FcbCreate(xfcb FAR * lpXfcb)
{
  sft FAR *sftp;
  COUNT sft_idx, FcbDrive;
  struct dhdr FAR *dhp;

  /* Build a traditional DOS file name                            */
  lpFcb = CommonFcbInit(lpXfcb, PriPathName, &FcbDrive);

  sft_idx = DosCreatSft(PriPathName, 0);
  if (sft_idx < 0)
    return FALSE;

  sftp = idx_to_sft(sft_idx);
  sftp->sft_mode |= SFT_MFCB;

  /* check for a device                                           */
  dhp = IsDevice(PriPathName);
  lpFcb->fcb_sftno = sft_idx;
  lpFcb->fcb_curec = 0;
  lpFcb->fcb_recsiz = (dhp ? 0 : 128);
  if (!dhp)
    lpFcb->fcb_drive = FcbDrive;
  lpFcb->fcb_fsize = 0;
  lpFcb->fcb_date = dos_getdate();
  lpFcb->fcb_time = dos_gettime();
  lpFcb->fcb_rndm = 0;
  return TRUE;
}

STATIC fcb FAR *ExtFcbToFcb(xfcb FAR * lpExtFcb)
{
  if (*((UBYTE FAR *) lpExtFcb) == 0xff)
    return &lpExtFcb->xfcb_fcb;
  else
    return (fcb FAR *) lpExtFcb;
}

STATIC fcb FAR *CommonFcbInit(xfcb FAR * lpExtFcb, BYTE * pszBuffer,
                              COUNT * pCurDrive)
{
  fcb FAR *lpFcb;

  /* convert to fcb if needed first                               */
  lpFcb = ExtFcbToFcb(lpExtFcb);

  /* Build a traditional DOS file name                            */
  FcbNameInit(lpFcb, pszBuffer, pCurDrive);

  /* and return the fcb pointer                                   */
  return lpFcb;
}

void FcbNameInit(fcb FAR * lpFcb, BYTE * szBuffer, COUNT * pCurDrive)
{
  BYTE loc_szBuffer[2 + FNAME_SIZE + 1 + FEXT_SIZE + 1];        /* 'A:' + '.' + '\0' */
  BYTE *pszBuffer = loc_szBuffer;

  /* Build a traditional DOS file name                            */
  if (lpFcb->fcb_drive != 0)
  {
    *pCurDrive = lpFcb->fcb_drive;
    pszBuffer[0] = 'A' + lpFcb->fcb_drive - 1;
    pszBuffer[1] = ':';
    pszBuffer += 2;
  }
  else
  {
    *pCurDrive = default_drive + 1;
  }
  ConvertName83ToNameSZ(pszBuffer, (BYTE FAR *) lpFcb->fcb_fname);
  truename(loc_szBuffer, szBuffer, FALSE);
  /* XXX fix truename error handling */
}

BOOL FcbOpen(xfcb FAR * lpXfcb)
{
  sft FAR *sftp;
  struct dhdr FAR *dhp;
  COUNT FcbDrive, sft_idx;

  /* Build a traditional DOS file name                            */
  lpFcb = CommonFcbInit(lpXfcb, PriPathName, &FcbDrive);

  sft_idx = DosOpenSft(PriPathName, O_RDWR);
  if (sft_idx < 0)
    return FALSE;

  sftp = idx_to_sft(sft_idx);
  sftp->sft_mode |= SFT_MFCB;

  /* check for a device                                           */
  lpFcb->fcb_curec = 0;
  lpFcb->fcb_rndm = 0;
  lpFcb->fcb_sftno = sft_idx;
  dhp = IsDevice(PriPathName);
  if (dhp)
  {
    lpFcb->fcb_recsiz = 0;
    lpFcb->fcb_fsize = 0;
    lpFcb->fcb_date = dos_getdate();
    lpFcb->fcb_time = dos_gettime();
  }
  else
  {
    lpFcb->fcb_drive = FcbDrive;
    lpFcb->fcb_recsiz = 128;
    lpFcb->fcb_fsize = sftp->sft_size;
    lpFcb->fcb_date = sftp->sft_date;
    lpFcb->fcb_time = sftp->sft_time;
  }
  return TRUE;
}

BOOL FcbDelete(xfcb FAR * lpXfcb)
{
  COUNT FcbDrive;

  /* Build a traditional DOS file name                            */
  CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);

  /* check for a device                                           */
  if (IsDevice(SecPathName))
  {
    return FALSE;
  }
  else
  {
    BYTE FAR *lpOldDta = dta;
    dmatch Dmatch;

    dta = (BYTE FAR *) & Dmatch;
    if (DosFindFirst
        (D_ALL,
         SecPathName[1] == ':' ? &SecPathName[2] : SecPathName) != SUCCESS)
    {
      dta = lpOldDta;
      return FALSE;
    }
    do
    {
      truename(Dmatch.dm_name, SecPathName, FALSE);
      if (DosDelete(SecPathName) != SUCCESS)
      {
        dta = lpOldDta;
        return FALSE;
      }
    }
    while (DosFindNext() == SUCCESS);
    dta = lpOldDta;
    return TRUE;
  }
}

BOOL FcbRename(xfcb FAR * lpXfcb)
{
  rfcb FAR *lpRenameFcb;
  COUNT FcbDrive;

  /* Build a traditional DOS file name                            */
  lpRenameFcb = (rfcb FAR *) CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);

  /* check for a device                                           */
  if (IsDevice(SecPathName))
  {
    return FALSE;
  }
  else
  {
    BYTE FAR *lpOldDta = dta;
    dmatch Dmatch;

    dta = (BYTE FAR *) & Dmatch;
    if (DosFindFirst
        (D_ALL,
         SecPathName[1] == ':' ? &SecPathName[2] : SecPathName) != SUCCESS)
    {
      dta = lpOldDta;
      return FALSE;
    }

    do
    {
      fcb LocalFcb;
      BYTE *pToName, *pszFrom;
      BYTE FAR *pFromPattern;
      COUNT nIndex;

      /* First, expand the find match into fcb style  */
      /* file name entry                              */
      /* Fill with blanks first                       */
      memset(LocalFcb.fcb_fname, ' ', FNAME_SIZE);
      memset(LocalFcb.fcb_fext, ' ', FEXT_SIZE);

      /* next move in the file name while overwriting */
      /* the filler blanks                            */
      pszFrom = Dmatch.dm_name;
      pToName = LocalFcb.fcb_fname;
      for (nIndex = 0; nIndex < FNAME_SIZE; nIndex++)
      {
        if (*pszFrom != 0 && *pszFrom != '.')
          *pToName++ = *pszFrom++;
        else if (*pszFrom == '.')
        {
          ++pszFrom;
          break;
        }
        else
          break;
      }

      if (*pszFrom != '\0')
      {
        pToName = LocalFcb.fcb_fext;
        for (nIndex = 0; nIndex < FEXT_SIZE; nIndex++)
        {
          if (*pszFrom != '\0')
            *pToName++ = *pszFrom++;
          else
            break;
        }
      }

      /* Overlay the pattern, skipping '?'            */
      /* I'm cheating because this assumes that the   */
      /* struct alignments are on byte boundaries     */
      pToName = LocalFcb.fcb_fname;
      for (pFromPattern = lpRenameFcb->renNewName,
           nIndex = 0; nIndex < FNAME_SIZE + FEXT_SIZE; nIndex++)
      {
        if (*pFromPattern != '?')
          *pToName++ = *pFromPattern++;
        else
          ++pFromPattern;
      }

      /* now to build a dos name again                */
      LocalFcb.fcb_drive = 0;
      FcbNameInit((fcb FAR *) & LocalFcb, SecPathName, &FcbDrive);

      truename(Dmatch.dm_name, PriPathName, FALSE);

      if (DosRenameTrue(PriPathName, SecPathName) != SUCCESS)
      {
        dta = lpOldDta;
        return FALSE;
      }
    }
    while (DosFindNext() == SUCCESS);
    dta = lpOldDta;
    return TRUE;
  }
}

/* TE:the MoveDirInfo() is now done by simply copying the dirEntry into the FCB
   this prevents problems with ".", ".." and saves code
   BO:use global SearchDir, as produced by FindFirst/Next
*/

BOOL FcbClose(xfcb FAR * lpXfcb)
{
  sft FAR *s;

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  /* An already closed FCB can be closed again without error */
  if (lpFcb->fcb_sftno == (BYTE) 0xff)
    return TRUE;

  /* Get the SFT block that contains the SFT      */
  if ((s = idx_to_sft(lpFcb->fcb_sftno)) == (sft FAR *) - 1)
    return FALSE;

  /* change time and set file size                */
  s->sft_size = lpFcb->fcb_fsize;
  if (!(s->sft_flags & SFT_FSHARED))
    dos_setfsize(s->sft_status, lpFcb->fcb_fsize);
  DosSetFtimeSft(lpFcb->fcb_sftno, lpFcb->fcb_date, lpFcb->fcb_time);
  if (DosCloseSft(lpFcb->fcb_sftno) == SUCCESS)
  {
    lpFcb->fcb_sftno = (BYTE) 0xff;
    return TRUE;
  }
  return FALSE;
}

/* close all files the current process opened by FCBs */
VOID FcbCloseAll()
{
  COUNT idx = 0;
  sft FAR *sftp;

  for (idx = 0; (sftp = idx_to_sft(idx)) != (sft FAR *) - 1; idx++)
    if ((sftp->sft_mode & SFT_MFCB) && sftp->sft_psp == cu_psp)
      DosCloseSft(idx);
}

BOOL FcbFindFirst(xfcb FAR * lpXfcb)
{
  BYTE FAR *lpDir;
  COUNT FcbDrive;
  psp FAR *lpPsp = MK_FP(cu_psp, 0);

  /* First, move the dta to a local and change it around to match */
  /* our functions.                                               */
  lpDir = (BYTE FAR *) dta;
  dta = (BYTE FAR *) & Dmatch;

  /* Next initialze local variables by moving them from the fcb   */
  lpFcb = CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);
  if (lpXfcb->xfcb_flag == 0xff)
  {
    wAttr = lpXfcb->xfcb_attrib;
    fmemcpy(lpDir, lpXfcb, 7);
    lpDir += 7;
  }
  else
    wAttr = D_ALL;

  if (DosFindFirst(wAttr, SecPathName) != SUCCESS)
  {
    dta = lpPsp->ps_dta;
    return FALSE;
  }

  *lpDir++ = FcbDrive;
  fmemcpy(lpDir, &SearchDir, sizeof(struct dirent));

  lpFcb->fcb_dirclst = (UWORD) Dmatch.dm_dircluster;
  lpFcb->fcb_strtclst = Dmatch.dm_entry;

/*
    This is undocumented and seen using Pcwatch and Ramview.
    The First byte is the current directory count and the second seems
    to be the attribute byte.
 */
  lpFcb->fcb_sftno = Dmatch.dm_drive;   /* MSD seems to save this @ fcb_date. */
#if 0
  lpFcb->fcb_cublock = Dmatch.dm_entry;
  lpFcb->fcb_cublock *= 0x100;
  lpFcb->fcb_cublock += wAttr;
#endif

  dta = lpPsp->ps_dta;
  return TRUE;
}

BOOL FcbFindNext(xfcb FAR * lpXfcb)
{
  BYTE FAR *lpDir;
  COUNT FcbDrive;
  psp FAR *lpPsp = MK_FP(cu_psp, 0);

  /* First, move the dta to a local and change it around to match */
  /* our functions.                                               */
  lpDir = (BYTE FAR *) dta;
  dta = (BYTE FAR *) & Dmatch;

  /* Next initialze local variables by moving them from the fcb   */
  lpFcb = CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);

  /* Reconstrct the dirmatch structure from the fcb               */
  Dmatch.dm_drive = lpFcb->fcb_sftno;

  fmemcpy(Dmatch.dm_name_pat, lpFcb->fcb_fname, FNAME_SIZE + FEXT_SIZE);
  DosUpFMem((BYTE FAR *) Dmatch.dm_name_pat, FNAME_SIZE + FEXT_SIZE);

  Dmatch.dm_attr_srch = wAttr;
  Dmatch.dm_entry = lpFcb->fcb_strtclst;
  Dmatch.dm_dircluster = lpFcb->fcb_dirclst;

  if ((xfcb FAR *) lpFcb != lpXfcb)
  {
    wAttr = lpXfcb->xfcb_attrib;
    fmemcpy(lpDir, lpXfcb, 7);
    lpDir += 7;
  }
  else
    wAttr = D_ALL;

  if (DosFindNext() != SUCCESS)
  {
    dta = lpPsp->ps_dta;
    CritErrCode = 0x12;
    return FALSE;
  }

  *lpDir++ = FcbDrive;
  fmemcpy((struct dirent FAR *)lpDir, &SearchDir, sizeof(struct dirent));

  lpFcb->fcb_dirclst = (UWORD) Dmatch.dm_dircluster;
  lpFcb->fcb_strtclst = Dmatch.dm_entry;

  lpFcb->fcb_sftno = Dmatch.dm_drive;
#if 0
  lpFcb->fcb_cublock = Dmatch.dm_entry;
  lpFcb->fcb_cublock *= 0x100;
  lpFcb->fcb_cublock += wAttr;
#endif

  dta = lpPsp->ps_dta;
  return TRUE;
}
#endif

/*
 * Log: fcbfns.c,v - for newer entries see "cvs log fcbfns.c"
 *
 * Revision 1.7  2000/03/31 05:40:09  jtabor
 * Added Eric W. Biederman Patches
 *
 * Revision 1.6  2000/03/17 22:59:04  kernel
 * Steffen Kaiser's NLS changes
 *
 * Revision 1.5  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.4  1999/09/23 04:40:46  jprice
 * *** empty log message ***
 *
 * Revision 1.2  1999/04/04 18:51:43  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:42:15  jprice
 * New version without IPL.SYS
 *
 * Revision 1.5  1999/02/09 02:54:23  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.4  1999/02/04 03:18:37  jprice
 * Formating.  Added comments.
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
 *    Rev 1.7   06 Dec 1998  8:44:10   patv
 * Expanded fcb functions for new I/O subsystem.
 *
 *    Rev 1.6   04 Jan 1998 23:14:38   patv
 * Changed Log for strip utility
 *
 *    Rev 1.5   03 Jan 1998  8:36:02   patv
 * Converted data area to SDA format
 *
 *    Rev 1.4   16 Jan 1997 12:46:38   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.3   29 May 1996 21:15:14   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.2   01 Sep 1995 17:48:44   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:50:26   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:06:06   patv
 * Initial revision.
 */
