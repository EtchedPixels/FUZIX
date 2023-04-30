CSRCS = devtty.c devices.c main.c
CDSRCS = discard.c
DSRCS = ../dev/devide.c ../dev/devsd.c ../dev/blkdev.c
DDSRCS = ../dev/devide_discard.c ../dev/devsd_discard.c ../dev/mbr.c
DZSRCS = ../dev/zx/divide.c ../dev/zx/divmmc.c ../dev/zx/zxkeyboard.c
DZSRCS += ../dev/zx/devinput.c ../dev/zx/bank128.c
DDZSRCS = ../dev/zx/zxuno.c
ASRCS = crt0.s zx128.s zxvideo.s
ASRCS += tricks.s commonmem.s loader-divide.s

COBJS = $(CSRCS:.c=.rel)
CDOBJS = $(CDSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DDOBJS = $(patsubst ../dev/%.c,%.rel, $(DDSRCS))
DZOBJS = $(patsubst ../dev/zx/%.c,%.rel, $(DZSRCS))
DDZOBJS = $(patsubst ../dev/zx/%.c,%.rel, $(DDZSRCS))
OBJS  = $(COBJS) $(CDOBJS) $(AOBJS) $(DOBJS) $(DDOBJS) $(DZOBJS) $(DDZOBJS)

CROSS_CCOPTS += -I../dev/ -I../dev/zx/

CROSS_CC_SEG3 = --codeseg CODE3

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

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.rel core *~ 
	rm -f BOOT.BIN strap.bin load-esx.ihx load-esx.tmp
	rm -f FUZIX.BIN FUZIX

# Re-order the image and snapshop it
image:
	# Merge the first code bank into common
	dd if=../bank1.bin bs=16384 skip=3 seek=3 count=1 \
		of=../common.bin conv=notrunc
	# The bootstrap to con Fatware
	# dd if=../common.bin of=BOOT.BIN bs=8192 count=1
	# Rest of the base image, starts at 0x2200, pad it
	# dd if=../common.bin bs=65536 count=1 conv=sync | \
	#	dd of=strap.bin bs=512 skip=17
	# Banked segments 1 and 7
	# dd if=../bank2.bin bs=16384 skip=3 >>strap.bin conv=sync
	# dd if=../bank3.bin bs=16384 skip=3 >>strap.bin conv=sync

	# Build an esxdos friendly setup
	sdasz80 -o load-esx.s
	sdldz80 -i load-esx.rel
	makebin -s 8704 load-esx.ihx load-esx.tmp
	# Generate the image file we need
	dd if=load-esx.tmp of=FUZIX bs=8192 skip=1

	# Now build a FUZIX.BIN to load
	# in the order we need it
	dd if=../common.bin bs=65536 count=1 conv=sync | \
		dd of=FUZIX.BIN
	dd if=../bank2.bin bs=16384 skip=3 >>FUZIX.BIN conv=sync
	dd if=../bank3.bin bs=16384 skip=3 >>FUZIX.BIN conv=sync

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)
# Adjust to wherever you have ESXDOS unpacked
ESXDOS = $(FUZIX_ROOT)/../ESXDOS

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
	hdfmonkey put $(IMAGES)/disk.hdf $(ESXDOS)/* /
	hdfmonkey put $(IMAGES)/disk.hdf FUZIX.BIN FUZIX.BIN
	hdfmonkey put $(IMAGES)/disk.hdf FUZIX BIN/FUZIX
