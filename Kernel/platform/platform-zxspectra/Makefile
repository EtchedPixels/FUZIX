CSRCS = devices.c main.c wiznet.c bank.c ide_simple.c
CDSRCS = discard.c
DSRCS = ../dev/devide.c ../dev/devsd.c ../dev/blkdev.c
DDSRCS = ../dev/devide_discard.c ../dev/devsd_discard.c ../dev/mbr.c
DZSRCS = ../dev/zx/zxmmc.c ../dev/zx/zxkeyboard.c
DZSRCS += ../dev/zx/devinput.c ../dev/zx/zxtty.c
DDZSRCS =
ASRCS = crt0.s zx128.s zxvideo.s
ASRCS += tricks.s commonmem.s

NSRCS = ../dev/net/net_w5x00.c

COBJS = $(CSRCS:.c=.rel)
CDOBJS = $(CDSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DDOBJS = $(patsubst ../dev/%.c,%.rel, $(DDSRCS))
DZOBJS = $(patsubst ../dev/zx/%.c,%.rel, $(DZSRCS))
DDZOBJS = $(patsubst ../dev/zx/%.c,%.rel, $(DDZSRCS))
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))
OBJS  = $(COBJS) $(CDOBJS) $(AOBJS) $(DOBJS) $(DDOBJS) $(DZOBJS) $(DDZOBJS) $(NOBJS)

CROSS_CCOPTS += -I../dev/ -I../dev/zx/ -I../dev/net/

CROSS_CC_SEG3 = --codeseg CODE3
CROSS_CC_SEG4 = --codeseg CODE4

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) -c $<

$(CDOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) -c $<

$(DDOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DZOBJS): %.rel: ../dev/zx/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) -c $<

$(DDZOBJS): %.rel: ../dev/zx/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG4) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.rel core *~ 
	rm -f block? loader.bin

image:
	sdasz80 -o loader.s
	sdldz80 -i loader.rel
	makebin -s 65536 loader.ihx loader.tmp
	# Extract the loader itself
	dd if=loader.tmp bs=512 skip=95 | ../tools/plus3boot >loader.bin
	# Padding to fill the virtual disk
	dd if=/dev/zero of=disk.raw bs=512 count=360
	# Make a disk image to work from
	dd if=loader.bin of=disk.raw conv=notrunc
	# CODE1 to CODE4 in order
	dd if=../bank1.bin of=disk.raw skip=96 seek=1 conv=notrunc
	dd if=../bank2.bin of=disk.raw skip=96 seek=33 conv=notrunc
	dd if=../bank3.bin of=disk.raw skip=96 seek=65 conv=notrunc
	dd if=../bank4.bin of=disk.raw skip=96 seek=97 conv=notrunc
	# 4000-7FFF from the common block
	dd if=../common.bin of=disk.raw skip=32 seek=129 conv=notrunc
	# And generate a 40 track plus3 disk from it
	../tools/raw2dsk disk.raw fuzix.dsk

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	dd if=hdfstart.bin of=$(IMAGES)/disk.hdf
	# Write a byte at the end to force the size
	dd if=/dev/zero of=$(IMAGES)/disk.hdf bs=1 seek=134218261 count=1
	# Sector 67584 (second partition on the hdfstart we used)
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.hdf bs=34603542 seek=1 conv=notrunc
	dd if=/dev/zero bs=512 count=65536 of=tmp.fat
	mkfs.fat -F16 tmp.fat
	# Sector 2048
	dd if=tmp.fat of=$(IMAGES)/disk.hdf bs=1049110 seek=1 conv=notrunc
	cp fuzix.dsk $(IMAGES)
