
CSRCS = devlpr.c devtty.c ../dev/z80pack/devfd.c
CSRCS += devices.c main.c

ASRCS = crt0.s z80pack.s
ASRCS += tricks.s commonmem.s

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
OBJS  = $(COBJS) $(AOBJS)

CROSS_CCOPTS += -I../dev/z80pack/

JUNK = $(CSRCS:.c=.lst) $(CSRCS:.c=.asm) $(CSRCS:.c=.sym) $(ASRCS:.s=.lst) $(ASRCS:.s=.sym) $(CSRCS:.c=.rst) $(ASRCS:.s=.rst)

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	sdasz80 -o bootblock.s
	sdldz80 -m -i bootblock.rel
	makebin -s 128 bootblock.ihx > bootblock.bin
