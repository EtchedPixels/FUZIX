CSRCS = devtty.c devices.c main.c
CDSRCS = discard.c
DSRCS = ../../dev/tinyide.c ../../dev/tinydisk.c ../../dev/devfdc765.c ../../dev/tinysd.c ../../dev/ch375.c
DDSRCS = ../../dev/tinyide_discard.c ../../dev/tinydisk_discard.c ../../dev/tinysd_discard.c
DZSRCS = ../../dev/cpc/cpcide.c ../../dev/cpc/cpckeyboard.c ../../dev/cpc/devinput.c ../../dev/cpc/albireo.c
DDZSRCS =
ASRCS = crt0.s cpc6128.s cpcvideo.s fdc765.s
ASRCS += tricks.s commonmem.s
NSRCS = 

COBJS = $(CSRCS:.c=.rel)
CDOBJS = $(CDSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))
DDOBJS = $(patsubst ../../dev/%.c,%.rel, $(DDSRCS))
DZOBJS = $(patsubst ../../dev/cpc/%.c,%.rel, $(DZSRCS))
DDZOBJS = $(patsubst ../../dev/cpc/%.c,%.rel, $(DDZSRCS))
NOBJS = $(patsubst ../../dev/net/%.c,%.rel, $(NSRCS))
OBJS  = $(COBJS) $(CDOBJS) $(AOBJS) $(DOBJS) $(DDOBJS) $(DZOBJS) $(DDZOBJS) $(NOBJS)

CROSS_CCOPTS += -I../../dev/ -I../../dev/cpc/ -I../../dev/net 

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

$(DZOBJS): %.rel: ../../dev/cpc/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) -c $<

$(DDZOBJS): %.rel: ../../dev/cpc/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.rel core *~ 
	rm -f loader.tmp loader.ihx loader.lst loader.bin padding.bin disk.raw
	rm -rf fuzix.dsk

image:
	# Copy snapshot file template, transfer fuzix code to 0x100 and set execution at 0x100
	cp $(FUZIX_ROOT)/Standalone/filesystem-src/6128.sna $(IMAGES)/
	createSnapshot $(IMAGES)/6128.sna --loadFileData ../../fuzix.bin 256
	printf '\x01' | dd of=$(IMAGES)/6128.sna  bs=1 seek=24 conv=notrunc
	sdasz80 -l -o loader.s
	sdldz80 -i loader.rel
	hex2bin loader.ihx

	dd if=/dev/zero of=padding.bin bs=512 count=360
	# Make a disk image to work from
	dd if=loader.bin of=padding.bin seek=0 bs=512 conv=notrunc
	dd if=../../fuzix.bin of=padding.bin bs=512 seek=1 conv=notrunc
	cat padding.bin >disk.raw
	# And generate a 40 track cpc system disk from it
	../../tools/raw2dskcpc disk.raw fuzix.dsk
	cp fuzix.dsk $(IMAGES)/fuzix.dsk
	
IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.64M of=$(IMAGES)/disk.img bs=64M conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=67584 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/hdfheader $(IMAGES)/disk.img > $(IMAGES)/emu-ide.hdf
	