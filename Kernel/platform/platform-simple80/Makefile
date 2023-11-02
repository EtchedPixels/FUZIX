ASRCS = crt0.S tricks.S commonmem.S simple80.S usermem.S ide.S
CSRCS = devices.c main.c devtty.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../../dev/tinyide_discard.c ../dev/tinydisk_discard.c
DISCARD_DSRCS += ../../dev/ds1302_discard.c
DSRCS = ../../dev/tinyide.c ../../dev/tinydisk.c
DSRCS +=  ../../dev/ds1302.c
DASRCS = ../../dev/ds1302_rcbusu.S
NSRCS = 

AOBJS = $(ASRCS:.S=.o)
COBJS = $(CSRCS:.c=.o)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.o)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.S,%.o, $(DASRCS))
NOBJS = $(patsubst ../../dev/net/%.c,%.o, $(NSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DAOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(NOBJS)

CROSS_CCOPTS += -I../../dev/ -I../../dev/net/

JUNK = *.o *.lst *.asm *.Sym *.rst *.map *.ihx *.bin

all:	$(OBJS)

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DAOBJS): %.o: ../../dev/%.S
	$(CROSS_AS) $(ASOPTS) $@ $<

$(DISCARD_COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.o: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ loader.tmp loader.bin fuzix.com

image:	loader.bin

loader.bin: loader.S
	asz80 loader.S
	ldz80 -b loader.o -o loader.tmp
	dd if=loader.tmp bs=256 skip=254 of=loader.bin count=1

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Loader
	dd if=loader.bin of=$(IMAGES)/disk.img bs=512 seek=1 conv=notrunc
	# Add the kernel
	dd if=../../fuzix.bin of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
