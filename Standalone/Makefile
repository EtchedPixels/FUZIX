CC=gcc
CCOPTS=-O2 -g -Wall -pedantic -Wno-char-subscripts -Wno-deprecated-declarations
TARGETS=mkfs fsck ucp chmem size

all:	$(TARGETS)

clean:
	rm -f $(TARGETS) *.o *~

mkfs:	mkfs.o util.o
	$(CC) $(CCOPTS) -o $@ $< util.o

fsck:	fsck.o
	$(CC) $(CCOPTS) -o $@ $<

ucp:	ucp.o $(UTILS)
	$(CC) $(CCOPTS) -o $@ $< util.o

chmem:	chmem.o
	$(CC) $(CCOPTS) -o $@ $<

size:	size.o
	$(CC) $(CCOPTS) -o $@ $<

%.o:	%.c
	$(CC) $(CCOPTS) -c -o $@ $<
