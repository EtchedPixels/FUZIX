ASRCS = crt0.s tricks.s commonmem.s rc2014.s monitor.s vfdterm.s
CSRCS = devices.c main.c devtty.c vfd-debug.c vfd-term.c
CSRCS += devinput.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../dev/devide_discard.c ../dev/ds1302_discard.c
DSRCS = ../dev/devfd.c ../dev/devide.c ../dev/mbr.c ../dev/blkdev.c
DSRCS +=  ../dev/ds1302.c ../dev/ppide_rbc.c
DASRCS = ../dev/devfd_hw-banked.s ../dev/ds1302_rc2014-banked.s
DASRCS += ../dev/z80dma.s
NSRCS = ../dev/net/net_native.c

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DAOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(NOBJS)

CROSS_CCOPTS += -I../dev/ -I../dev/net/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(DAOBJS): %.rel: ../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ bootrom.ihx bootrom.bin fuzix.com fuzix.rom

#
#	Compile up the boot block
#
image: bootblock
	cat boot-romwbw.bin >fuzix.romwbw
	dd if=../common.bin bs=16384 count=1 >>fuzix.romwbw
	dd if=../bank1.bin bs=16384 skip=1 count=2 conv=sync >>fuzix.romwbw
	dd if=../common.bin bs=16384 skip=3 count=1 conv=sync >>fuzix.romwbw
	dd if=../bank2.bin bs=16384 skip=1 count=2 conv=sync >>fuzix.romwbw

bootblock:
	sdasz80 -o boot-romwbw.s
	sdldz80 -i boot-romwbw.rel
	# This makes us a binary from physical 0
	makebin -s 62464 boot-romwbw.ihx boot-romwbw.tmp
	# Chop off the leading 61440 bytes we don't want
	dd if=boot-romwbw.tmp of=boot-romwbw.bin bs=512 count=2 skip=120
