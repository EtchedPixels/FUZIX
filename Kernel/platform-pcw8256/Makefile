
CSRCS = devlpr.c devtty.c devfd.c devinput.c devvd.c
CSRCS += devices.c main.c

DSRCS = ../dev/devide.c ../dev/blkdev.c ../dev/mbr.c
DISCARD_DSRCS = ../dev/devide_discard.c

NSRCS = ../dev/net/net_native.c

CROSS_CCOPTS += -I ../dev/

ASRCS =  crt0.s commonmem.s pcw8256.s vd.s
ASRCS += tricks.s fdc765.s

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))

OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(DISCARD_DOBJS) $(NOBJS)

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
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.lk  core *~

image:
	# Make a blank disc
	dd if=/dev/zero of=bootdisk.raw bs=512 count=360
	# Add the boot block in sector 0
	dd if=BOOTBLOCK/zout/765.cim of=bootdisk.raw conv=notrunc
	# Add the kernel
	dd if=../fuzix.bin of=bootdisk.raw bs=512 seek=1 conv=notrunc