CC=gcc
CCOPTS=-O2 -g -Wall -Wno-char-subscripts -Wno-deprecated-declarations
TARGETS=mkfs fsck ucp chmem size
UTILS=util.o devio.o xfs1.o xfs1a.o xfs1b.o xfs2.o

all:	$(TARGETS)

clean:
	rm -f $(TARGETS) *.o *~

mkfs:	mkfs.o util.o
	$(CC) $(CCOPTS) -o $@ $< util.o

fsck:	fsck.o util.o
	$(CC) $(CCOPTS) -o $@ $< util.o

ucp:	ucp.o $(UTILS)
	$(CC) $(CCOPTS) -o $@ $< $(UTILS)

chmem:	chmem.o
	$(CC) $(CCOPTS) -o $@ $<

size:	size.o
	$(CC) $(CCOPTS) -o $@ $<

%.o:	%.c
	$(CC) $(CCOPTS) -c -o $@ $<
