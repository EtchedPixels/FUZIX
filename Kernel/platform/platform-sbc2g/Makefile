CROSS_CCOPTS += -I../../dev/ -I../../dev/net/


CSRCS = devtty.c
CSRCS += devices.c main.c

DISCSRCS = discard.c

ASRCS = sbc2g.s crt0.s
ASRCS += tricks.s commonmem.s

DISCARD_DSRCS = ../../dev/tinydisk_discard.c ../../dev/tinyide_discard.c
DSRCS = ../../dev/tinydisk.c ../../dev/tinyide.c
NSRCS = 

DASRCS =

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
NOBJS = $(patsubst ../../dev/net/%.c,%.rel, $(NSRCS))
DISCOBJS = $(DISCSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.s,%.rel, $(DASRCS))

OBJS  = $(COBJS) $(AOBJS) $(NOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS)

JUNK =  *.lst *.asm *.sym *.rst *.lst *.ihx *.tmp *.rel

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DAOBJS): %.rel: ../../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

clean:
	rm -f $(JUNK)  core *~ loader.bin

image:	loader.bin

loader.bin: loader.s
	sdasz80 -o loader.s
	sdldz80 -i loader.rel
	makebin -s 65536 loader.ihx loader.tmp
	dd if=loader.tmp bs=256 skip=192 of=loader.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=part.bin of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=16512 conv=notrunc
	# Add the kernel
	dd if=../../fuzix.bin of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc
	# Add the bootloader
	dd if=loader.bin of=$(IMAGES)/disk.img seek=16384 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
