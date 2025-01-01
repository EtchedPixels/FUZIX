ASRCS = crt0.s tricks.s commonmem.s vz200.s video.s
CSRCS = devices.c main.c devtty.c sd.c devlpr.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../../dev/tinydisk_discard.c
DSRCS = ../../dev/tinydisk.c ../../dev/tinysd.c
DASRCS =
NSRCS = 

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))
DHOBJS = $(patsubst ../../dev/%.c,%.rel, $(DHSRCS))
DAOBJS = $(patsubst ../../dev/%.s,%.rel, $(DASRCS))
NOBJS = $(patsubst ../../dev/net/%.c,%.rel, $(NSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DHOBJS) $(DAOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(NOBJS)

CROSS_CCOPTS += -I../../dev/ -I../../dev/net/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(DAOBJS): %.rel: ../../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) -c $<

$(DISCARD_DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ fuzix.boot booter.tmp

image: booter boot bootblock
	# Our image consists of the following in order starting at LBA 1
	# Bank 0 0x4000 for 10K
	# Bank 1 0x4000 for 10K
	# Bank 0 0x9000 for 28K
	# Common 0x7200 for 5.5K
	# Font packed on the end for machines with the high res mod
	dd if=../../bank1.bin of=fuzix.boot bs=512 skip=32 count=20
	dd if=../../bank2.bin of=fuzix.boot bs=512 skip=32 seek=20 count=20
	dd if=../../common.bin of=fuzix.boot bs=512 skip=72 seek=40 count=56
	dd if=../../common.bin of=fuzix.boot bs=512 skip=57 seek=96 count=11
	dd if=../../bank3.bin of=fuzix.boot bs=512 skip=56 seek=107 count=4

booter: booter.s
	sdasz80 -o booter.s
	sdldz80 -i booter.rel
	makebin -s 65535 booter.ihx booter.tmp
	dd if=booter.tmp of=booter.vz bs=1 skip=32744 count=536
#
#	Alternate Boot ROM image.
#	TODO: support bootstrapping sanely off the standard boot rom with a .VZ
#
boot: boot.s
	sdasz80 -o boot.s
	sdldz80 -i boot.rel
	makebin -s 65535 boot.ihx boot.tmp
	dd if=boot.tmp bs=1 skip=16384 >boot.tmp2
	dd if=boot.tmp2 bs=6034 count=1 conv=sync >sdboot.rom

bootblock: bootblock.s
	sdasz80 -o bootblock.s
	sdldz80 -i bootblock.rel
	makebin -s 65535 bootblock.ihx bootblock.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel extra blocks
	dd if=fuzix.boot of=$(IMAGES)/disk.img bs=512 seek=1 conv=notrunc
	# Add the boot block at the start
	dd if=bootblock.bin bs=1 skip=34816 count=384 of=$(IMAGES)/disk.img conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.20M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
	# Loader
	cp booter.vz $(IMAGES)/booter.vz
	cp sdboot.rom $(IMAGES)/sdboot.rom
