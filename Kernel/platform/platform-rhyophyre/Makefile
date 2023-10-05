ASRCS = crt0.s z180.s commonmem.s rph.s ds1302-rph.s monitor.s
CSRCS += devices.c main.c devtty.c ps2.c devinput.c 7220.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../dev/ds1302_discard.c ../dev/tinyide_discard.c
DISCARD_DSRCS += ../dev/tinydisk_discard.c
DSRCS = ../dev/tinydisk.c
DSRCS += ../dev/tinyide_ppide_rbc.c ../dev/tinyide.c
DSRCS += ../dev/ds1302.c ../dev/ps2kbd.c ../dev/ps2mouse.c
DASRCS = ../dev/devrd_hw.s ../dev/devrd_z180_hw.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(DAOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DAOBJS): %.rel: ../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ fuzix.com makecpmloader

z180.rel:		z180.s kernel.def ../cpu-z180/z180.s

bootblock:
	sdasz80 -o boot-romwbw.s
	sdldz80 -i boot-romwbw.rel
	# This makes us a binary from physical 0
	makebin -s 62464 boot-romwbw.ihx boot-romwbw.tmp
	# Chop off the leading 61440 bytes we don't want
	dd if=boot-romwbw.tmp of=boot-romwbw.bin bs=512 count=2 skip=120

#
#	Compile up the boot block
#
image: bootblock
	cat boot-romwbw.bin >fuzix.romwbw
	dd if=/dev/zero bs=256 count=1 >>fuzix.romwbw
	cat ../fuzix.bin >>fuzix.romwbw

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=fuzix.romwbw of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
