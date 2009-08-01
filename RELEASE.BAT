@ECHO OFF
IF "%1"=="" GOTO USAGE
REM goto to just below trunk and tags directory, assume ran in trunk directory
CD ..

ECHO tag SVN with release version - svn copy trunk/ tags/ke%1
svn copy https://freedos.svn.sourceforge.net/svnroot/freedos/kernel/trunk/ https://freedos.svn.sourceforge.net/svnroot/freedos/kernel/tags/ke%1 -m "Tag kernel release %1"
PAUSE
ECHO svn export to get clean tree
if EXIST SOURCE RMDIR /S /Q SOURCE > NUL
svn export https://freedos.svn.sourceforge.net/svnroot/freedos/kernel/tags/ke%1 SOURCE\ke%1
REM svn export https://freedos.svn.sourceforge.net/svnroot/freedos/kernel/trunk SOURCE\ke%1

SET VERSION=%1
SET LSMRET=SRC
SET LSMFILE=SOURCE\ke%1\docs\fdkernel.lsm
GOTO LSM
:SRC
ECHO zipping source
7z.exe a -tzip -mx9 -mpass15 -r ke%1s.zip SOURCE\*
ECHO creating APPINFO and expected packaging dir structure
ECHO using working configuration file
COPY trunk\CONFIG.BAT SOURCE\ke%1 > NUL
CD SOURCE\ke%1

ECHO build and packaging
SET VERSION=%1 (FAT12/FAT16)
SET FAT=16
SET BZKRET=F16
GOTO BZK
:F16
SET VERSION=%1 (FAT12/FAT16/FAT32)
SET FAT=32
SET BZKRET=F32
GOTO BZK
:F32
ECHO clean up
CD ..\..
RMDIR /S /Q SOURCE > NUL
ECHO Done.
SET BZKRET=
GOTO DONE


:BZK
ECHO build kernel %VERSION%
CALL build.bat /D KERNEL_VERSION /V "%1 " 86 upx fat%FAT%
DEL BIN\K??86??.sys
SET LSMRET=BZK_2
SET LSMFILE=docs\fdkernel.lsm
GOTO LSM
:BZK_2
SET LSMRET=
ECHO zipping FAT%FAT% release version
7z.exe a -tzip -mx9 -mpass15 -r ..\..\ke%1_86f%FAT%.zip BIN\* DOCS\*
ECHO restructuring and zipping update package
DEL BIN\K??86??.* > NUL
MKDIR DOC
MKDIR DOC\KERNEL
COPY DOCS\* DOC\KERNEL\
MKDIR APPINFO
MOVE DOC\KERNEL\*.lsm APPINFO\
7z.exe a -tzip -mx9 -mpass15 -r ..\..\kernel%FAT%.zip APPINFO\* BIN\* DOC\*
ECHO cleaning up between builds
CALL clobber.bat
RMDIR /S /Q DOC
RMDIR /S /Q APPINFO
GOTO %BZKRET%

:LSM
ECHO Begin3>%LSMFILE%
ECHO Title:          The FreeDOS Kernel>>%LSMFILE%
ECHO Version:        %VERSION%>>%LSMFILE%
ECHO Entered-date:   %DATE%>>%LSMFILE%
ECHO Description:    The FreeDOS Kernel>>%LSMFILE%
ECHO Keywords:       kernel, FreeDOS, DOS, MSDOS>>%LSMFILE%
ECHO Author:         (developers: can be reached on the freedos-kernel mailing list)>>%LSMFILE%
ECHO Maintained-by:  freedos-kernel@lists.sourceforge.net>>%LSMFILE%
ECHO Primary-site:   http://freedos.sourceforge.net/kernel/>>%LSMFILE%
ECHO Alternate-site: http://www.fdos.org/kernel/>>%LSMFILE%
ECHO Alternate-site: https://freedos.svn.sourceforge.net/svnroot/freedos>>%LSMFILE%
ECHO Original-site:  http://www.gcfl.net/pub/FreeDOS/kernel>>%LSMFILE%
ECHO Platforms:      DOS, FreeDOS, DOSEMU (OpenWatcom C or Turbo C, NASM, UPX)>>%LSMFILE%
ECHO Copying-policy: GPL2>>%LSMFILE%
ECHO End>>%LSMFILE%
SET LSMFILE=
SET VERSION=
GOTO %LSMRET%

:USAGE
ECHO Tag and build release kernels - usage: RELEASE {VERSION} e.g. RELEASE 2039
:DONE
