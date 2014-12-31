CFLAGS          = -g -O2 -Wall -Wno-parentheses -DINDEXLIB -DUNIX -I.
LDFLAGS         = 

SRC = lk_readnl.c lkaomf51.c lkar.c lkarea.c lkdata.c lkelf.c lkeval.c \
        lkhead.c lklex.c lklib.c lklibr.c lklist.c lkmain.c lkmem.c \
        lknoice.c lkout.c lkrel.c lkrloc.c lkrloc3.c lks19.c lksdcclib.c \
        lksym.c sdld.c lksdcdb.c lkbank.c

OBJS = $(SRC:%.c=%.o)
LKOBJECTS = $(OBJS)

sdldz80: $(LKOBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(LKOBJECTS)

$(LKOBJECTS) : aslink.h

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f $(LKOBJECTS) *~ sdldz80