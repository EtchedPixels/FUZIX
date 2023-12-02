CROSS_CCOPTS += -I../../dev/ -I../../dev/net/


CSRCS = devtty.c
CSRCS += devices.c main.c wiznet.c

DISCSRCS = discard.c

ASRCS = sc720.S crt0.S
ASRCS += tricks.S commonmem.S

DISCARD_DSRCS = ../../dev/tinydisk_discard.c ../../dev/ds1302_discard.c ../../dev/tinyide_discard.c
DSRCS = ../../dev/tinydisk.c ../../dev/tinyide.c
DSRCS += ../../dev/ds1302.c
NSRCS = ../../dev/net/net_w5x00.c ../../dev/net/net_w5300.c ../../dev/net/net_native.c

DASRCS = ../../dev/ds1302_rcbusu.S

COBJS = $(CSRCS:.c=.o)
AOBJS = $(ASRCS:.S=.o)
NOBJS = $(patsubst ../../dev/net/%.c,%.o, $(NSRCS))
DISCOBJS = $(DISCSRCS:.c=.o)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.S,%.o, $(DASRCS))

OBJS  = $(COBJS) $(AOBJS) $(NOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS)

JUNK =  *.o

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCOBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.o: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

$(DAOBJS): %.o: ../../dev/%.S
	$(CROSS_AS) $(ASOPTS) $@ $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ *.ihx *.bin *.o *.tmp fuzix.romwbw

#
#	Attach the kernel to the boot block
#
image:	bootblock
	cat boot-romwbw.bin ../../fuzix.bin >fuzix.romwbw
#
#	Compile up the boot block
#
bootblock:
	asz80 boot-romwbw.S
	ldz80 -b boot-romwbw.o -o boot-romwbw.tmp
	# Chop off the leading 61440 bytes we don't want
	dd if=boot-romwbw.tmp of=boot-romwbw.bin bs=512 count=2 skip=120

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=fuzix.romwbw of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.20M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
