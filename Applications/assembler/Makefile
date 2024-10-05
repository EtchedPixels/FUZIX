#
#	Build 6803/68 version of the tools
#
all: as68 ld68 nm68 osize68 dumprelocs68

HDR = as.h ld.h obj.h

AOBJ = as0.o as1-6303.o as2.o as3.o as4.o as6-6303.o

CFLAGS = -DTARGET_6303 -Wall -pedantic -DOBJ_LONGNAME

as68: $(HDR) $(AOBJ)
	$(CC) -o as68 $(AOBJ)

ld68: $(HDR) ld.o
	$(CC) $(CFLAGS) -o ld68 ld.o

nm68: $(HDR) nm.o
	$(CC) $(CFLAGS) -o nm68 nm.o

osize68: $(HDR) osize.o
	$(CC) $(CFLAGS) -o osize68 osize.o

dumprelocs68: $(HDR) dumprelocs.o
	$(CC) $(CFLAGS) -o dumprelocs68 dumprelocs.o

clean:
	rm -f *.o *~
	rm -f nm68 ld68 as68 osize68 dumprelocs68
