CROSS_CCOPTS += -I../dev/ -I../dev/net/


CSRCS = devtty.c
CSRCS += devices.c main.c wiznet.c

DISCSRCS = discard.c

ASRCS = sbcv2.s crt0.s
ASRCS += tricks.s commonmem.s

DISCARD_DSRCS = ../dev/devide_discard.c ../dev/ds1302_discard.c
DSRCS = ../dev/blkdev.c ../dev/devide.c ../dev/mbr.c
DSRCS += ../dev/propio2.c ../dev/ds1302.c
DSRCS += ../dev/devfd.c ../dev/ppide_rbc.c
NSRCS = ../dev/net/net_w5100.c ../dev/net/net_native.c

DASRCS = ../dev/rbcfd9266_hw.s ../dev/ds1302_rbc.s

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))
DISCOBJS = $(DISCSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))

OBJS  = $(COBJS) $(AOBJS) $(NOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS)

JUNK =  *.lst *.asm *.sym *.rst *.lst

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DAOBJS): %.rel: ../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~

#
#	Attach the kernel to the boot block
#
image:	bootblock
	cat boot-romwbw.bin ../fuzix.bin >fuzix.romwbw
#
#	Compile up the boot block
#
bootblock:
	sdasz80 -o boot-romwbw.s
	sdldz80 -i boot-romwbw.rel
	# This makes us a binary from physical 0
	makebin -s 62464 boot-romwbw.ihx boot-romwbw.tmp
	# Chop off the leading 61440 bytes we don't want
	dd if=boot-romwbw.tmp of=boot-romwbw.bin bs=512 count=2 skip=120
