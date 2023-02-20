CC=gcc
#Use this for playing with the experimental libdsk support
#CCOPTS=-O2 -DLIBDSK -ldsk -g -Wall -pedantic -Wno-char-subscripts -Wno-deprecated-declarations
CCOPTS=-O2 -g -Wall -pedantic -Wno-char-subscripts -Wno-deprecated-declarations

TARGETS=mkfs fsck ucp chmem size elf2aout mkftl sethint mkfs400 fsck400

all:	$(TARGETS)

clean:
	rm -f $(TARGETS) *.o *~

mkfs:	mkfs.o util.o
	$(CC) $(CCOPTS) -o $@ $< util.o

mkfs400: mkfs400.o util400.o
	$(CC) $(CCOPTS) -o $@ $< util400.o

fsck:	fsck.o util.o
	$(CC) $(CCOPTS) -o $@ $< util.o

fsck400: fsck400.o util.o
	$(CC) $(CCOPTS) -o $@ $< util400.o

ucp:	ucp.o util.o
	$(CC) $(CCOPTS) -o $@ $< util.o

chmem:	chmem.o
	$(CC) $(CCOPTS) -o $@ $<

sethint: sethint.o
	$(CC) $(CCOPTS) -o $@ $<

size:	size.o
	$(CC) $(CCOPTS) -o $@ $<

elf2aout:	elf2aout.o
	$(CC) $(CCOPTS) -o $@ $<

mkftl: mkftl.o ../Kernel/lib/dhara/journal.c ../Kernel/lib/dhara/error.c ../Kernel/lib/dhara/map.c
	$(CC) $(CCOPTS) -o $@ $^

%.o:	%.c
	$(CC) $(CCOPTS) -c -o $@ $<
