FCC = ../../Library/tools/fcc
FCCOPTS = -O2

.SUFFIXES: .c .rel

LIBS = ../../Library/libs/syslib.lib

all: flashrom

flashrom:	flashrom.c
	$(FCC) $(FCCOPTS) flashrom.c -o flashrom

clean:
	rm -f flashrom core *~ *.asm *.lst *.sym *.map *.rst *.lk *.noi *.rel *.bin
