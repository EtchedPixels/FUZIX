CSRCS = devtty.c devices.c main.c
CDSRCS = discard.c
DSRCS = ../../dev/tinysd.c ../../dev/tinydisk.c
DDSRCS = ../../dev/tinysd_discard.c ../../dev/tinydisk_discard.c
DZSRCS = ../../dev/zx/divmmc.c ../../dev/zx/zxkeyboard.c
DZSRCS += ../../dev/zx/devinput.c ../../dev/zx/bank128.c
DDZSRCS = ../../dev/zx/zxuno.c
ASRCS = crt0.s zx128.s zxvideo.s
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

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(CDOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DDOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DZOBJS): %.rel: ../../dev/zx/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DDZOBJS): %.rel: ../../dev/zx/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.rel core *~ 
	rm -f BOOT.BIN strap.bin load-esx.ihx load-esx.tmp
	rm -f FUZIX.BIN FUZIX

# Re-order the image and snapshop it
image:
	# Build an esxdos friendly setup
	sdasz80 -o load-esx.s
	sdldz80 -i load-esx.rel
	makebin -s 16384 load-esx.ihx load-esx.tmp
	# Generate the image file we need
	dd if=load-esx.tmp of=FUZIX bs=8192 skip=1

	# Now build a FUZIX.BIN to load
	# in the order we need it
	cp ../../fuzix.bin FUZIX.BIN

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
	dd if=$(IMAGES)/disk.hdf of=$(IMAGES)/disk.mmc bs=534 skip=1
