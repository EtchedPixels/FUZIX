
CSRCS = devlpr.c devtty.c devfd.c devatom.c msm6242b.c devinput.c
CSRCS += devices.c main.c

DSRCS = ../dev/devide.c ../dev/mbr.c ../dev/blkdev.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../dev/devide_discard.c

ASRCS = sam.s crt0.s sam_vt.s floppy.s atom.s
ASRCS += tricks.s commonmem.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(DISCARD_COBJS) $(DISCARD_DOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:	boot setup
	# Make a blank floppy
	dd if=/dev/zero of=fuzix.raw bs=512 count=1600
	# Boot block at track 4 sector 1
	dd if=boot.bin of=fuzix.raw bs=512 seek=40 count=1 conv=notrunc
	# Kernel from track 4 sector 2 highest 32K first
	dd if=../fuzix.bin of=fuzix.raw bs=512 seek=41 skip=64 count=64 conv=notrunc
	# Low 32K next
	dd if=../fuzix.bin of=fuzix.raw bs=512 seek=105 count=64 conv=notrunc
	# Font and setup from 64K further on
	dd if=setup.bin of=fuzix.raw bs=512 seek=169 skip=64 conv=notrunc
	# Now shuffle it into a .mgt file
	../tools/raw2mgt fuzix.raw fuzix.mgt

boot:
	sdasz80 -o boot.s
	sdldz80 -i boot.rel
	# 512 bytes at 0
	makebin -s 512 -p boot.ihx > boot.bin

setup:
	sdasz80 -o setup.s
	sdldz80 -n -f setup.lnk
	# 5.5K at 32K
	makebin -s 38400 -p setup.ihx > setup.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	cp fuzix.mgt $(IMAGES)/
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Make an emulator image of i
	cat 40mb.hdf $(IMAGES)/disk.img > $(IMAGES)/fuzix.hdf
