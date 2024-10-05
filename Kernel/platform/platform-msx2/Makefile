
DSRCS = ../../dev/devsd.c ../../dev/mbr.c ../../dev/blkdev.c
CSRCS += devfd.c devhd.c devlpr.c
CSRCS += devices.c main.c devtty.c ../../dev/rp5c01.c devrtc.c ../../dev/v99xx.c vdp.c
COMMON_CSRCS = devmegasd.c devide_sunrise.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../../dev/devsd_discard.c
ASRCS = msx2.s crt0.s
ASRCS += tricks.s commonmem.s bootrom.s diskboot.s
ASRCS += sunrise.s

CROSS_CCOPTS += -I../../dev/

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))
COMMON_COBJS = $(COMMON_CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DISCARD_DSRCS))
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(DISCARD_COBJS) $(DISCARD_DOBJS) $(COMMON_COBJS)

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) --codeseg CODE2 -c $<

$(COMMON_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) --codeseg CODE1 -c $<

$(DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.lk  core *~
	rm -f fuzix.ascii8.rom fuzix.com boot.bin

image:
	dd if=../../fuzix.bin of=fuzix.tmp bs=256 skip=1
	dd if=../../fuzix_disk.bin of=fuzix.com bs=256 skip=241
	cat fuzix.tmp >> fuzix.com
	dd if=../../fuzix_boot.bin of=fuzix.ascii8.rom skip=1 bs=16384 conv=sync
	dd if=../../fuzix.bin of=fuzix.ascii8.rom seek=1 bs=16384 conv=sync
	rm -f fuzix.tmp

boot: boot.s
	sdasz80 -o boot.s
	sdldz80 -i boot.rel
	# This makes us a binary from physical 0
	makebin -p -s 49664 boot.ihx boot.tmp
	# Chop off the leading 49152 bytes we don't want
	dd if=boot.tmp of=boot.bin bs=512 count=1 skip=96 conv=sync
	rm -f boot.ihx boot.tmp

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image boot
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the boot partition
	env LANG=c echo -en '\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\xff\x07\x00\x00' | dd iflag=fullblock of=$(IMAGES)/disk.img bs=478 seek=1 conv=notrunc
	# Add the boot sector
	dd if=boot.bin of=$(IMAGES)/disk.img bs=512 seek=1 conv=notrunc
	# Add the kernel
	dd if=fuzix.com of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	cp fuzix.ascii8.rom $(IMAGES)
	cp fuzix.com $(IMAGES)
