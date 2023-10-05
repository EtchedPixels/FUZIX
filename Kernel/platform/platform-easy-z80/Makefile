ASRCS = crt0.s tricks.s commonmem.s easy-z80.s monitor.s buffers.s
CSRCS = devices.c main.c devtty.c extbuffer.c
CSRCS += devinput.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../dev/devide_discard.c ../dev/ds1302_discard.c
DSRCS = ../dev/devfd.c ../dev/devide.c ../dev/mbr.c ../dev/blkdev.c
DSRCS +=  ../dev/ds1302.c ../dev/ppide_rbc.c
DASRCS = ../dev/devfd_hw.s ../dev/ds1302_rcbus.s
NSRCS = ../dev/net/net_native.c

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DAOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(NOBJS)

CROSS_CCOPTS += -I../dev/ -I../dev/net/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DAOBJS): %.rel: ../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ bootrom.ihx bootrom.bin bootrom.tmp
	rm -f fuzix.romwbw boot-romwbw.tmp test-romwbw.tmp

#
#	Compile up the boot block
#
image: bootblock testblock
	cat boot-romwbw.bin ../fuzix.bin >fuzix.romwbw

bootblock:
	sdasz80 -o boot-romwbw.s
	sdldz80 -i boot-romwbw.rel
	# This makes us a binary from physical 0
	makebin -s 62464 boot-romwbw.ihx boot-romwbw.tmp
	# Chop off the leading 61440 bytes we don't want
	dd if=boot-romwbw.tmp of=boot-romwbw.bin bs=512 count=2 skip=120

testblock:
	sdasz80 -o test-romwbw.s
	sdldz80 -i test-romwbw.rel
	# This makes us a binary from physical 0
	makebin -s 62464 test-romwbw.ihx test-romwbw.tmp
	# Chop off the leading 61440 bytes we don't want
	dd if=test-romwbw.tmp of=test-romwbw.bin bs=512 count=2 skip=120

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=fuzix.romwbw of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
