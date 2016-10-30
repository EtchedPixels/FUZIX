.SUFFIXES: .c .rel

CC = fcc
CFLAGS = -c
COPT = -O2

OBJS = ue.rel

all: ue

ue: $(OBJS)
	$(CC) -o $@ --nostdio $(OBJS)

.c.rel:
	$(CC) $(COPT) $(CFLAGS) $(DEFS) -c $< -o $@

clean realclean clobber:
	rm -f *.rel ue *~

