ASRCS = crt0.s tricks.s commonmem.s zeta-v2.s monitor.s
ASRCS += ds1302-n8vem.s devrd_zeta2_hw.s
CSRCS = devices.c main.c devtty.c devrd_zeta2.c
DISCARD_CSRCS = discard.c devtty_discard.c
DISCARD_DSRCS = ../dev/ds1302_discard.c ../dev/tinyide_discard.c ../dev/tinydisk_discard.c
DSRCS = ../dev/devfd.c ../dev/tinydisk.c ../dev/tinyide.c
DSRCS += ../dev/ds1302.c ../dev/devrd.c ../dev/tinyide_ppide_rbc.c
DASRCS = ../dev/devfd_hw.s ../dev/devrd_hw.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DAOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS)

CROSS_CCOPTS += -I../dev/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS) diskboot.bin

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
	rm -f $(OBJS) $(JUNK)  core *~ bootrom.ihx bootrom.bin diskboot.bin fuzix.com fuzix.rom

diskboot.bin:	diskboot.s
	$(CROSS_AS) $(ASOPTS) diskboot.s
	sdldz80 -nmi diskboot.rel
	makebin -s 65536 diskboot.ihx | dd bs=512 skip=125 count=1 of=diskboot.bin

image:
	sdasz80 -o bootrom.s
	sdldz80 -m -i bootrom.rel
	makebin -s 136 bootrom.ihx > bootrom.bin
	# Work around bugs in Fedora dd
	cat bootrom.bin ../fuzix.bin > fuzix.tmp
	dd if=fuzix.tmp conv=sync bs=524288 count=1 of=fuzix.rom
	../cpm-loader/makecpmloader ../cpm-loader/cpmload.bin ../fuzix.bin 0x88 fuzix.com
	../cpm-loader/makecpmloader ../cpm-loader/fuzixload.bin ../fuzix.bin 0x88 fuzix

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# ROM
	cp fuzix.rom $(IMAGES)
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the boot block into the partition sector
	dd if=diskboot.bin bs=446 count=1 of=$(IMAGES)/disk.img conv=notrunc
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=../fuzix.bin of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
