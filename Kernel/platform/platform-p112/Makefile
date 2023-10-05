ASRCS = crt0.s z180.s commonmem.s p112.s ds1302-p112.s monitor.s flopboot.s
CSRCS += devices.c main.c devtty.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../dev/devide_discard.c ../dev/ds1302_discard.c
DSRCS = ../dev/blkdev.c ../dev/devide.c ../dev/devfd.c ../dev/mbr.c
DSRCS += ../dev/ds1302.c
DSRCS += ../dev/devrd.c
DASRCS = ../dev/devfd_hw.s ../dev/devrd_hw.s ../dev/devrd_z180_hw.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DAOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS)

CROSS_CCOPTS += -I../dev/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS) flopboot.bin

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
	rm -f $(OBJS) $(JUNK)  core *~ fuzix.com fuzix-boot.fdd

z180.rel:		z180.s kernel.def ../cpu-z180/z180.s

flopboot.bin:	flopboot.s
	$(CROSS_AS) $(ASOPTS) flopboot.s
	sdldz80 -nmi flopboot.rel
	makebin -s 65536 flopboot.ihx > flopboot.bin

image:
	../cpm-loader/makecpmloader ../cpm-loader/cpmload.bin ../fuzix.bin 0x88 fuzix.com
	../cpm-loader/makecpmloader ../cpm-loader/fuzixload.bin ../fuzix.bin 0x88 fuzix
	./flopboot-mkimage flopboot.bin ../fuzix.bin fuzix-boot.fdd

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Boot floppy and CP/M boot
	cp fuzix-boot.fdd fuzix.com $(IMAGES)
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
