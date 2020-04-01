
CSRCS = devlpr.c devtty.c devfd.c devinput.c devvd.c
CSRCS += devices.c main.c
DISCARD_CSRCS = discard.c

DSRCS = ../dev/devide.c ../dev/blkdev.c ../dev/mbr.c ../dev/devfdc765.c
DISCARD_DSRCS = ../dev/devide_discard.c

NSRCS = ../dev/net/net_native.c

CROSS_CCOPTS += -I ../dev/

ASRCS =  crt0.s commonmem.s pcw8256.s vd.s
ASRCS += tricks.s fdc765.s

COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))

OBJS  = $(COBJS) $(DISCARD_COBJS) $(AOBJS) $(DOBJS) $(DISCARD_DOBJS) $(NOBJS)

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.lk  core *~
	rm -f bootblock-dotmatrix bootblock-daisy *.tmp *.ihx
	rm -f dotmatrix.raw daisy.raw

image: ../tools/pcwsum bootblock-daisy bootblock-dotmatrix
	# Make a blank disc
	dd if=/dev/zero of=dotmatrix.raw bs=512 count=360
	# Add the boot block in sector 0
	dd if=bootblock-dotmatrix of=dotmatrix.raw conv=notrunc
	# Add the kernel
	dd if=../fuzix.bin of=dotmatrix.raw bs=512 seek=1 conv=notrunc
	# Repeat for Daisywheel
	dd if=/dev/zero of=daisy.raw bs=512 count=360
	# Add the boot block in sector 0
	dd if=bootblock-daisy of=daisy.raw conv=notrunc
	# Add the kernel
	dd if=../fuzix.bin of=daisy.raw bs=512 seek=1 conv=notrunc

bootblock-daisy: ../tools/pcwsum
	sdasz80 -o bootblock.s
	sdldz80 -i bootblock.rel
	makebin -s 61952 bootblock.ihx bootblock.tmp
	dd if=bootblock.tmp of=bootblock-daisy bs=256 skip=240
	../tools/pcwsum bootblock-daisy daisy

bootblock-dotmatrix: bootblock-daisy
	cp bootblock-daisy bootblock-dotmatrix
	../tools/pcwsum bootblock-dotmatrix dotmatrix

../tools/pcwsum: ../tools/pcwsum.c
