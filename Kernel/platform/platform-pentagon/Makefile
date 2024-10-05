CSRCS = devices.c main.c
CDSRCS = discard.c
DSRCS = ../../dev/tinyide.c ../../dev/tinysd.c ../../dev/tinydisk.c
DDSRCS = ../../dev/tinyide_discard.c ../../dev/tinysd_discard.c
DDSRCS += ../../dev/tinydisk_discard.c
DZSRCS = ../../dev/zx/nemoide.c ../../dev/zx/zxmmc.c ../../dev/zx/zxkeyboard.c
DZSRCS += ../../dev/zx/devinput.c ../../dev/zx/bankbig.c ../../dev/zx/zxtty.c
DDZSRCS =
ASRCS = crt0.s pentagon.s zxvideo.s
ASRCS += tricks.s commonmem.s

COBJS = $(CSRCS:.c=.rel)
CDOBJS = $(CDSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))
DDOBJS = $(patsubst ../../dev/%.c,%.rel, $(DDSRCS))
DZOBJS = $(patsubst ../../dev/zx/%.c,%.rel, $(DZSRCS))
DDZOBJS = $(patsubst ../../dev/zx/%.c,%.rel, $(DDZSRCS))
OBJS  = $(COBJS) $(CDOBJS) $(AOBJS) $(DOBJS) $(DDOBJS) $(DZOBJS) $(DDZOBJS)

CROSS_CCOPTS += -I../../dev/ -I../../dev/zx/

CROSS_CC_SEG3 = --codeseg CODE3

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) -c $<

$(CDOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) -c $<

$(DDOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DZOBJS): %.rel: ../../dev/zx/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) -c $<

$(DDZOBJS): %.rel: ../../dev/zx/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.rel core *~ 
	rm -f block? loader.bin

loader.bin: loader.s
	sdasz80 -o loader.s
	sdldz80 -i loader.rel
	makebin -s 32768 loader.ihx loader.tmp
	dd if=loader.tmp bs=2048 skip=15 count=1 conv=sync of=loader.bin

image: loader.bin
	# Common memory 4000-7FFF - bank 5
	dd if=../../common.bin bs=16384 conv=sync skip=1 count=1 of=block0
	# CODE1 C000-FFFF - bank 0
	dd if=../../bank1.bin bs=16384 conv=sync skip=3 count=1 of=block1
	# Common memory 8000-BFFF - bank 2
	dd if=../../common.bin bs=16384 conv=sync skip=2 count=1 of=block2
	# CODE2 C000-FFFF - bank 6
	dd if=../../bank2.bin bs=16384 conv=sync skip=3 count=1 of=block3
	# CODE3 C000-FFFF - bank 7 (video)
	dd if=../../bank3.bin bs=16384 conv=sync skip=3 count=1 of=block4
	# CODE4 C000-FFFF - bank 3
	dd if=../../bank4.bin bs=16384 conv=sync skip=3 count=1 of=block5

	cp base-image.trd fuzix.trd
	# Insert 4K loader
	../../tools/trdify fuzix.trd BOOT0 loader.bin
	# Kernel image
	../../tools/trdify fuzix.trd BLOCK0 block0
	../../tools/trdify fuzix.trd BLOCK1 block1
	../../tools/trdify fuzix.trd BLOCK2 block2
	../../tools/trdify fuzix.trd BLOCK3 block3
	../../tools/trdify fuzix.trd BLOCK4 block4
	../../tools/trdify fuzix.trd BLOCK5 block5

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	cp fuzix.trd $(IMAGES)/
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/hdfheader $(IMAGES)/disk.img > $(IMAGES)/emu-ide.hdf
