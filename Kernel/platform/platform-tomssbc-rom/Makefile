CSRCS = devtty.c devices.c main.c
CDSRCS = discard.c
DSRCS = ../../dev/tinyide.c ../../dev/tinydisk.c
DDSRCS = ../../dev/tinyide_discard.c ../../dev/tinydisk_discard.c
ASRCS = start.s crt0.s tom.s tricks.s commonmem.s

COBJS = $(CSRCS:.c=.rel)
CDOBJS = $(CDSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))
DDOBJS = $(patsubst ../../dev/%.c,%.rel, $(DDSRCS))
OBJS  = $(COBJS) $(CDOBJS) $(AOBJS) $(DOBJS) $(DDOBJS)

CROSS_CCOPTS += -I../../dev/

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

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.rel core *~ 
	rm -f *.ihx *.tmp
	rm -f fuzix.rom
	

# Re-order the image and turn it into the raw image to load from disk
image:	../../tools/makeeprom64
	# Assemble the ROM as a single image
	../../tools/makeeprom64

../../tools/makeeprom64: ../../tools/makeeprom64.c
	gcc -Wall -pedantic ../../tools/makeeprom64.c -o ../../tools/makeeprom64

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	mv fuzix.rom $(FUZIX_ROOT)/Images/$(TARGET)
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
