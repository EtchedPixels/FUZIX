TARGET=z80pack
CPU=z80

export TARGET CPU

all: stand ltools libs apps kernel


stand:
	(cd Standalone; make)

ltools:
	(cd Library; make; make install)

libs:
	(cd Library/libs; make -f Makefile.$(CPU))

apps:
	(cd Applications; make)

kernel:
	(cd Kernel; make)

clean:
	(cd Standalone; make clean)
	(cd Library/libs; make -f Makefile.$(CPU) clean)
	(cd Library; make clean)
	(cd Kernel; make clean)
	(cd Applications; make clean)
