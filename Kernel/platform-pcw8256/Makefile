
CSRCS = devlpr.c devtty.c devfd.c devfhd.c fidhd.c devinput.c
CSRCS += devices.c main.c

DSRCS = ../dev/devide.c ../dev/blkdev.c ../dev/mbr.c
DISCARD_DSRCS = ../dev/devide_discard.c

NSRCS = ../dev/net/net_native.c

CROSS_CCOPTS += -I ../dev/

ASRCS =  crt0.s commonmem.s pcw8256.s
ASRCS += tricks.s fdc765.s

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))

OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(DISCARD_DOBJS) $(NOBJS)

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

$(NOBJS): %.rel: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
