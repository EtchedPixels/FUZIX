CC=gcc
#Use this for playing with the experimental libdsk support
#CCOPTS=-O2 -DLIBDSK -ldsk -g -Wall -pedantic -Wno-char-subscripts -Wno-deprecated-declarations
CCOPTS=-O2 -g -Wall -pedantic -Wno-char-subscripts -Wno-deprecated-declarations
TARGETS=mkfs fsck ucp chmem size mkftl

all:	$(TARGETS)

clean:
	rm -f $(TARGETS) *.o *~

mkfs:	mkfs.o util.o
	$(CC) $(CCOPTS) -o $@ $< util.o

fsck:	fsck.o util.o
	$(CC) $(CCOPTS) -o $@ $< util.o

ucp:	ucp.o util.o
	$(CC) $(CCOPTS) -o $@ $< util.o

chmem:	chmem.o
	$(CC) $(CCOPTS) -o $@ $<

size:	size.o
	$(CC) $(CCOPTS) -o $@ $<

mkftl: mkftl.o ../Kernel/lib/dhara/journal.c ../Kernel/lib/dhara/error.c ../Kernel/lib/dhara/map.c
	$(CC) $(CCOPTS) -o $@ $^

%.o:	%.c
	$(CC) $(CCOPTS) -c -o $@ $<
