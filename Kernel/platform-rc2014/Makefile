ASRCS = crt0.s tricks.s commonmem.s rc2014.s monitor.s usermem.s vdp1.s video-9345.s soft81.s
CSRCS = devices.c main.c devtty.c ide.c sd.c ps2.c
CSRCS += devinput.c devgpio.c wiznet.c
CSRCS += softzx81.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../dev/devide_discard.c ../dev/ds1302_discard.c
DISCARD_DSRCS += ../dev/devsd_discard.c
DSRCS = ../dev/devfd.c ../dev/devide.c ../dev/mbr.c ../dev/blkdev.c
DSRCS +=  ../dev/ds1302.c ../dev/ppide_rbc.c ../dev/devsd.c
DSRCS += ../dev/ps2kbd.c ../dev/ps2mouse.c ../dev/amd9511.c
DSRCS += ../dev/ds12885.c ../dev/ds12885_z80.c
DASRCS = ../dev/devfd_hw-banked.s ../dev/ds1302_rcbus.s
DASRCS += ../dev/z80dma.s ../dev/z80softsd.s ../dev/z80softspi.s
DASRCS += ../dev/ps2bitbang-rcbus-kbd.s ../dev/ps2bitbang-rcbus-mouse.s
CSRCS += ps2direct.c
NSRCS = ../dev/net/net_w5x00.c ../dev/net/net_w5300.c
DRSRCS = ../dev/rcbus/zxkeyboard.c
DRASRCS = ../dev/rcbus/zxkey.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))
DROBJS = $(patsubst ../dev/rcbus/%.c,%.rel, $(DRSRCS))
DRAOBJS = $(patsubst ../dev/rcbus/%.s,%.rel, $(DRASRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DAOBJS) $(DISCARD_DOBJS)
OBJS += $(DISCARD_COBJS) $(NOBJS) $(DROBJS) $(DRAOBJS)

CROSS_CCOPTS += -I../dev/ -I../dev/net/ -I../dev/rcbus/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(DAOBJS): %.rel: ../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_NETWORK) -c $<

$(DROBJS): %.rel: ../dev/rcbus/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(DRAOBJS): %.rel: ../dev/rcbus/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<


clean:
	rm -f $(OBJS) $(JUNK)  core *~ bootrom.ihx bootrom.bin fuzix.com fuzix.rom

#
#	Compile up the boot block
#
image: bootblock
	cat boot-romwbw.bin >fuzix.romwbw
	dd if=../common.bin bs=16384 count=1 >>fuzix.romwbw
	dd if=../bank1.bin bs=16384 skip=1 count=2 conv=sync >>fuzix.romwbw
	dd if=../common.bin bs=16384 skip=3 count=1 conv=sync >>fuzix.romwbw
	dd if=../bank2.bin bs=16384 skip=1 count=2 conv=sync >>fuzix.romwbw

bootblock:
	sdasz80 -o boot-romwbw.s
	sdldz80 -i boot-romwbw.rel
	# This makes us a binary from physical 0
	makebin -s 62464 boot-romwbw.ihx boot-romwbw.tmp
	# Chop off the leading 61440 bytes we don't want
	dd if=boot-romwbw.tmp of=boot-romwbw.bin bs=512 count=2 skip=120

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
