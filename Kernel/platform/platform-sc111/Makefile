ASRCS = crt0.s z180.s commonmem.s sc111.s monitor.s
CSRCS += devices.c main.c devtty.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../../dev/devide_discard.c ../../dev/ds1302_discard.c
DSRCS = ../../dev/devide.c ../../dev/mbr.c ../../dev/blkdev.c ../../dev/ds1302.c
DASRCS = ../../dev/ds1302_rcbus.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.s,%.rel, $(DASRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(DAOBJS)

CROSS_CCOPTS += -I../../dev/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DAOBJS): %.rel: ../../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ fuzix.com makecpmloader loader.bin loader.tmp boot-romwbw.bin

z180.rel: z180.s kernel.def ../../cpu-z180/z180.s

boot-romwbw.bin: boot-romwbw.s
	sdasz80 -o boot-romwbw.s
	sdldz80 -i boot-romwbw.rel
	makebin -s 62464 boot-romwbw.ihx boot-romwbw.tmp                 # binary from physical 0
	dd if=boot-romwbw.tmp of=boot-romwbw.bin bs=512 count=2 skip=120 # cut leading 61440 bytes

romwbw: boot-romwbw.bin

loader.bin: loader.s
	sdasz80 -o loader.s
	sdldz80 -i loader.rel
	makebin -s 65536 loader.ihx loader.tmp
	dd if=loader.tmp bs=256 skip=208 of=loader.bin

image: loader.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

#
# The diskimage contains both the ROMWBW loader from 0x0400 to 0x0800
# and the SCM loader from 0x2800 to 0x3000. Kernel at 0x3000 and
# filesystem at 1M is shared between both loaders
#
# |-----------------|0x0000:0000
# | partition table |
# | 0x17C: bootdev  |
# |-----------------|0x0000:0200
# |                 |
# |-----------------|0x0000:0400
# | ROMWBW header   |
# | ROMWBW loader   |
# |-----------------|0x0000:0800
# |                 |
# |-----------------|0x0000:2800
# | SCM loader      |
# |-----------------|0x0000:3000
# | FUZIX kernel    |
# |                 |
# |-----------------|0x0010:0000
# | filesystem      |
# |                 |
#
diskimage: image romwbw
	rm -f $(IMAGES)/disk.img
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img conv=notrunc
	# Kernel boot string at 0x17C with signature: 0xB5 0x5E hda1
	printf "\265\136%s\000" "hda1" | dd of=$(IMAGES)/disk.img seek=380 bs=1 conv=notrunc
	# ROMWBW loader at 0x400
	dd if=boot-romwbw.bin of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc
	# SCM loader at 0x2800
	dd if=loader.bin of=$(IMAGES)/disk.img bs=512 skip=20 seek=20 count=4 conv=notrunc
	# Kernel starting 0x3000
	dd if=../../fuzix.bin of=$(IMAGES)/disk.img bs=512 seek=24 conv=notrunc
	# Filesystem starting at 1M/0x0010:0000
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
