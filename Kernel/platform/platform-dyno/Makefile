ASRCS = crt0.s z180.s commonmem.s dyno.s monitor.s
CSRCS += devices.c main.c devtty.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../dev/tinyide_discard.c ../dev/tinydisk_discard.c
DSRCS = ../dev/tinyide.c ../dev/tinydisk.c ../dev/tinyide_ppide_rbc.c ../dev/bq4845.c
DSRCS += ../dev/devfd.c
DASRCS = ../dev/devfd_hw.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(DAOBJS)

CROSS_CCOPTS += -I../dev/

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

clean:
	rm -f $(OBJS) $(JUNK)  core *~ fuzix.com makecpmloader
	rm -f loader.bin loader.tmp boot-romwbw.bin boot-romwbw.tmp

z180.rel:		z180.s kernel.def ../cpu-z180/z180.s

boot-romwbw.bin: boot-romwbw.s
	sdasz80 -o boot-romwbw.s
	sdldz80 -i boot-romwbw.rel
	makebin -s 62464 boot-romwbw.ihx boot-romwbw.tmp                 # binary from physical 0
	dd if=boot-romwbw.tmp of=boot-romwbw.bin bs=512 count=2 skip=120 # cut leading 61440 bytes

image: boot-romwbw.bin
	cat boot-romwbw.bin >fuzix.romwbw
	dd if=/dev/zero bs=256 count=1 >>fuzix.romwbw
	cat ../fuzix.bin >>fuzix.romwbw

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img conv=notrunc
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=fuzix.romwbw of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
