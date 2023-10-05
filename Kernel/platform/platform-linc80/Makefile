ASRCS = crt0.s tricks.s commonmem.s linc80.s piospi.s piosd.s
CSRCS = devices.c main.c devtty.c sd.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../../dev/tinyide_discard.c ../../dev/tinysd_discard.c
DISCARD_DSRCS += ../../dev/tinydisk_discard.c
DSRCS = ../../dev/tinyide.c ../../dev/tinydisk.c ../../dev/tinysd.c
NSRCS = 

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(NOBJS)

CROSS_CCOPTS += -I../../dev/ -I../../dev/net/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ fuzix.com loader.tmp

image:  loader.bin
	dd if=../../fuzix.bin of=fuzix.com bs=256 skip=1 count=248

loader.bin: loader.s
	sdasz80 -o loader.s
	sdldz80 -i loader.rel
	makebin -s 65536 loader.ihx loader.tmp
	dd if=loader.tmp bs=256 skip=208 of=loader.bin

piospi.rel: ../../dev/z80softspi.s
linc80.rel: ../../dev/z80sio.s

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the boot block into the partition sector
	dd if=loader.bin bs=446 count=1 of=$(IMAGES)/disk.img conv=notrunc
	# Add the rest of the loader
	dd if=loader.bin bs=512 skip=1 of=$(IMAGES)/disk.img seek=1 conv=notrunc
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=fuzix.com of=$(IMAGES)/disk.img bs=512 seek=24 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
