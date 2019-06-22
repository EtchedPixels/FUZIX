
CSRCS = devlpr.c devtty.c devfd.c devsil.c devinput.c
CSRCS += devices.c main.c

DISCSRCS = discard.c

ASRCS = crt0.s mtx.s vdp.s floppy.s
ASRCS += tricks.s commonmem.s

DISCARD_DSRCS = ../dev/devide_discard.c
DSRCS = ../dev/blkdev.c ../dev/devide.c ../dev/mbr.c

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCOBJS = $(DISCSRCS:.c=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DISCOBJS) $(DISCARD_DOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = *.rel *.lst *.asm *.sym *.rst

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~  fuzix.com
	rm -f cfloader.ihx cfloader.tmp cfloader.bin

cfloader.bin: cfloader.s
	sdasz80 -o cfloader.s
	sdldz80 -i cfloader.rel
	makebin -s 65536 cfloader.ihx cfloader.tmp
	dd if=cfloader.tmp bs=256 skip=254 | dd bs=128 count=1 of=cfloader.bin

image: cfloader.bin
	cp ../fuzix.bin fuzix.com
