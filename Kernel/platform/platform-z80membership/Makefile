CROSS_CCOPTS += -I../../dev/


CSRCS = devtty.c devices.c main.c sdcard.c

DISCSRCS = discard.c

ASRCS = z80membership.s crt0.s
ASRCS += tricks.s commonmem.s

DISCARD_DSRCS = ../../dev/devsd_discard.c
DSRCS = ../../dev/blkdev.c ../../dev/devsd.c ../../dev/mbr.c
NSRCS =

DASRCS = 

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
NOBJS = $(patsubst ../../dev/net/%.c,%.rel, $(NSRCS))
DISCOBJS = $(DISCSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.s,%.rel, $(DASRCS))

OBJS  = $(COBJS) $(AOBJS) $(NOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS)

JUNK =  *.lst *.asm *.sym *.rst *.lst *.rel *.tmp *.ihx

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DAOBJS): %.rel: ../../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ *.tmp

loader.ihx: loader.s
	sdasz80 -o loader.s
	sdldz80 -i loader.rel

image: loader.ihx

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition for FAT and FUZIX
	dd if=parttab of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Land the Fuzix file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=12288 conv=notrunc
	# Build the MSDOS partition
	dd if=/dev/zero of=dos.part bs=512 count=10240
	mkdosfs dos.part
	mkdir -p tmpdir
	sudo mount -o loop,umask=0 dos.part tmpdir
	cp loader.ihx  tmpdir/FUZIX.HEX
	sudo umount tmpdir
	# Attach the MSDOS partition to the disk
	dd if=dos.part of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=../../fuzix.bin of=$(IMAGES)/disk.img bs=512 seek=3 conv=notrunc
	# Clean up
	rm dos.part
	rmdir tmpdir
