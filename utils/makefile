include ../mkfiles/generic.mak

CFLAGS = -I..$(DIRSEP)hdr

production: patchobj.com exeflat.exe

patchobj.com: patchobj.c
	$(CLT) $(CFLAGS) -o $@ patchobj.c

exeflat.exe: exeflat.c ../hdr/exe.h
	$(CLC) $(CFLAGS) -o $@ exeflat.c


clobber: clean

clean:
	$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.las *.cod *.err status.me
	$(RM) exeflat.exe patchobj.com

