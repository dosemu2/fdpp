# What you WANT on DOS is: 
# EDIT CONFIG.B, COPY CONFIG.B to CONFIG.BAT, RUN BUILD.BAT
# On Linux, use config.mak, and "make all", "make clean", or "make clobber"

default:
	@echo On DOS, please type build, clean, or clobber.
	@echo On Linux, please type make all, make clean, or make clobber.

build:
	build

bin\kwc8616.sys:
	build -r wc 86 fat16

bin\kwc8632.sys:
	build -r wc 86 fat32

# use as follows: wmake -ms zip VERSION=2029
zip_src:
	cd ..\..
	zip -9 -r -k source/ke$(VERSION)s.zip source/ke$(VERSION) -i@source/ke$(VERSION)/filelist
	cd source\ke$(VERSION)

BINLIST1 = doc bin/kernel.sys bin/sys.com
# removed - as the 2nd zip -r line to add those to the zip:
# BINLIST2 = bin/config.sys bin/autoexec.bat bin/command.com bin/install.bat

zipfat16: bin\kwc8616.sys
	mkdir doc
	mkdir doc\kernel
	copy docs\*.txt doc\kernel
	copy docs\*.cvs doc\kernel
	copy docs\copying doc\kernel
	copy docs\*.lsm doc\kernel
	del doc\kernel\build.txt
	del doc\kernel\lfnapi.txt
	copy bin\kwc8616.sys bin\kernel.sys
	zip -r -k ../ke$(VERSION)16.zip $(BINLIST)
	utils\rmfiles doc\kernel\*.txt doc\kernel\*.cvs doc\kernel\*.lsm doc\kernel\copying
	rmdir doc\kernel
	rmdir doc

zipfat32: bin\kwc8632.sys
	mkdir doc
	mkdir doc\kernel
	copy docs\*.txt doc\kernel
	copy docs\*.cvs doc\kernel
	copy docs\copying doc\kernel
	copy docs\*.lsm doc\kernel
	del doc\kernel\build.txt
	del doc\kernel\lfnapi.txt
	copy bin\kwc8632.sys bin\kernel.sys
	zip -r -k ../ke$(VERSION)32.zip $(BINLIST)
	utils\rmfiles doc\kernel\*.txt doc\kernel\*.cvs doc\kernel\*.lsm doc\kernel\copying
	rmdir doc\kernel
	rmdir doc

zip: zip_src zipfat16 zipfat32

#Linux part
#defaults: override using config.mak
export
COMPILER=owlinux

XCPU=86
XFAT=32
ifndef WATCOM
  WATCOM=$(HOME)/watcom
  PATH:=$(WATCOM)/binl:$(PATH)
endif
XUPX=upx --8086 --best
XNASM=nasm
MAKE=wmake -ms -h
XLINK=wlink
#ALLCFLAGS=-DDEBUG

-include config.mak
ifdef XUPX
  UPXOPT=-U
endif

all:
	cd utils && $(MAKE) production
	cd lib && ( test -f libm.lib || touch libm.lib )
	cd drivers && $(MAKE) production
	cd boot && $(MAKE) production
	cd sys && $(MAKE) production
	cd kernel && $(MAKE) production

clean:
	cd utils && $(MAKE) clean
	cd lib && $(MAKE) clean
	cd drivers && $(MAKE) clean
	cd boot && $(MAKE) clean
	cd sys && $(MAKE) clean
	cd kernel && $(MAKE) clean

clobber:
	cd utils && $(MAKE) clobber
	cd lib && $(MAKE) clobber
	cd drivers && $(MAKE) clobber
	cd boot && $(MAKE) clobber
	cd sys && $(MAKE) clobber
	cd kernel && $(MAKE) clobber
