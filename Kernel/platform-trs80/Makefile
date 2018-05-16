
CSRCS = devlpr.c devtty.c devfd.c devhd.c devgfx.c
CSRCS += devices.c main.c
DISCARD_CSRCS = discard.c devhd_discard.c

ASRCS = trs80.s trs80-bank.s crt0.s
ASRCS += tricks.s commonmem.s floppy.s drivewire.s

DSRCS = ../dev/devdw.c

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
OBJS  = $(COBJS) $(AOBJS) $(DISCARD_COBJS) $(DOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = $(CSRCS:.c=.lst) $(CSRCS:.c=.asm) $(CSRCS:.c=.sym) $(ASRCS:.s=.lst) $(ASRCS:.s=.sym) $(CSRCS:.c=.rst) $(ASRCS:.s=.rst)

all:	$(OBJS) trs80load.bin

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image: trs80load.bin trs80load94.bin

trs80load.bin: trs80load.s
	sdasz80 -o trs80load.s
	sdldz80 -i trs80load.rel
	makebin -s 256 trs80load.ihx trs80load.bin
