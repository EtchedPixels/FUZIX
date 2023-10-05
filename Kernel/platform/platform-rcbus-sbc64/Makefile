ASRCS = crt0.s tricks.s commonmem.s rcbus.s
CSRCS = devices.c main.c devtty.c platform_ide.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../../dev/tinyide_discard.c ../../dev/ds1302_discard.c ../../dev/tinydisk_discard.c
DSRCS = ../../dev/tinyide.c ../../dev/tinydisk.c
DSRCS +=  ../../dev/ds1302.c
DASRCS = ../../dev/ds1302_rcbus.s
NSRCS = 

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.s,%.rel, $(DASRCS))
NOBJS = $(patsubst ../../dev/net/%.c,%.rel, $(NSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DAOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(NOBJS)

CROSS_CCOPTS += -I../../dev/ -I../../dev/net/

CROSS_CC_HIGH = --codeseg COMMONMEM

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DAOBJS): %.rel: ../../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ diskstrap fuzix.rom boot loader.inc

image:	boot loader boot-zrcc

boot:	boot.s
	sdasz80 -o boot.s
	sdldz80 -i boot.rel
	makebin -s 4096 boot.ihx boot
	od -v -w1 -tx1 boot | cut -b 9- | head -4096 | \
		sed -e 's/^/        .db 0x/' >loader.inc

boot-zrcc: boot-zrcc.s
	sdasz80 -o boot-zrcc.s
	sdldz80 -i boot-zrcc.rel
	makebin -s 512 boot-zrcc.ihx | dd bs=256 skip=1 | ../tools/doubleup > boot-zrcc

loader:	loader.s loader.inc
	sdasz80 -o loader.s
	sdldz80 -i loader.rel

loader.inc: boot boot-zrcc

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=../../fuzix.bin of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc
	# Add the zrcc bootstrap
	dd if=boot-zrcc of=$(IMAGES)/disk.img bs=446 count=1 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.20M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
	cp boot $(IMAGES)
