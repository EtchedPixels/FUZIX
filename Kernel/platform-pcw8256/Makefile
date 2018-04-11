
CSRCS = devlpr.c devtty.c devfd.c devfhd.c fidhd.c
CSRCS += devices.c main.c

DSRCS = ../dev/devide.c ../dev/blkdev.c ../dev/mbr.c
DISCARD_DSRCS = ../dev/devide_discard.c

CROSS_CCOPTS += -I ../dev/

ASRCS =  crt0.s commonmem.s pcw8256.s
ASRCS += tricks.s fdc765.s

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))

OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(DISCARD_DOBJS)

JUNK = $(CSRCS:.c=.lst) $(CSRCS:.c=.asm) $(CSRCS:.c=.sym) $(ASRCS:.s=.lst) $(ASRCS:.s=.sym) $(CSRCS:.c=.rst) $(ASRCS:.s=.rst)

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
