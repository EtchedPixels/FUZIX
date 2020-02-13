CC=gcc
CCOPTS=-O2 -DLIBDSK -ldsk -g -Wall -pedantic -Wno-char-subscripts -Wno-deprecated-declarations
TARGETS=mkfs fsck ucp chmem size

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

%.o:	%.c
	$(CC) $(CCOPTS) -c -o $@ $<
