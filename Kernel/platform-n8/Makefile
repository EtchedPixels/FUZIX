ASRCS = crt0.s z180.s commonmem.s n8.s ds1302-n8.s monitor.s
CSRCS += devices.c main.c devtty.c devsdspi.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../dev/devsd_discard.c ../dev/ds1302_discard.c
DSRCS = ../dev/devsd.c ../dev/mbr.c ../dev/blkdev.c ../dev/ds1302.c
DASRCS =

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(DAOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin *.tmp

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DAOBJS): %.rel: ../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ fuzix.romwbw fuzix

z180.rel:		z180.s kernel.def ../cpu-z180/z180.s

#
#	Compile up the boot block
#
image: bootblock
	cat boot-romwbw.bin >fuzix.romwbw
	dd if=/dev/zero bs=256 count=1 >>fuzix.romwbw
	cat ../fuzix.bin >>fuzix.romwbw

bootblock:
	sdasz80 -o boot-romwbw.s
	sdldz80 -i boot-romwbw.rel
	# This makes us a binary from physical 0
	makebin -s 62464 boot-romwbw.ihx boot-romwbw.tmp
	# Chop off the leading 61440 bytes we don't want
	dd if=boot-romwbw.tmp of=boot-romwbw.bin bs=512 count=2 skip=120
