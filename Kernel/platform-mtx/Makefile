
CSRCS = devlpr.c devtty.c devfd.c devsil.c devinput.c sdspi.c ide.c
CSRCS += devices.c main.c devrtc.c

DISCSRCS = discard.c

ASRCS = crt0.s mtx.s vdp.s floppy.s
ASRCS += tricks.s commonmem.s

DISCARD_DSRCS = ../dev/devide_discard.c ../dev/devsd_discard.c
DSRCS = ../dev/blkdev.c ../dev/devide.c ../dev/mbr.c
DSRCS += ../dev/ppide_rbc.c ../dev/devsd.c

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
	rm -f biosloader.ihx biosloader.tmp biosloader.bin

cfloader.bin: cfloader.s
	sdasz80 -o cfloader.s
	sdldz80 -i cfloader.rel
	makebin -s 65536 cfloader.ihx cfloader.tmp
	dd if=cfloader.tmp bs=256 skip=254 | dd bs=128 count=1 of=cfloader.bin

biosloader.bin: biosloader.s
	sdasz80 -o biosloader.s
	sdldz80 -i biosloader.rel
	makebin -s 65536 biosloader.ihx biosloader.tmp
	dd if=biosloader.tmp bs=256 skip=238 | dd bs=128 count=1 of=biosloader.bin

image: cfloader.bin biosloader.bin
	cp ../fuzix.bin fuzix.com
