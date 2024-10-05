ASRCS = crt0.s tricks.s commonmem.s kc87.s usermem.s
CSRCS = devices.c main.c devtty.c devrtc.c devrd.c ide.c vt.c maths32.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../../dev/tinyide_discard.c ../../dev/tinydisk_discard.c
DSRCS = ../../dev/tinyide.c ../../dev/tinydisk.c
DASRCS = 
NSRCS = 

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.s,%.rel, $(DASRCS))
NOBJS = $(patsubst ../../dev/net/%.c,%.rel, $(NSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DHOBJS) $(DAOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(NOBJS)

CROSS_CCOPTS += -I. -I../../dev/ -I../../dev/net/

CROSS_CC_HIGH = --codeseg COMMONMEM

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(DAOBJS): %.rel: ../../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ fuzix.rom bootstrap.tmp
	rm -f mega.rom megarom_shuffle

bootstrap: bootstrap.s megarom_shuffle
	sdasz80 -o bootstrap.s
	sdldz80 -i bootstrap.rel
	# This makes us a binary from physical 0
	makebin -s 62464 bootstrap.ihx bootstrap.tmp
	>bootstrap.bin
	dd if=bootstrap.tmp of=bootstrap.bin bs=512 skip=96 count=1
	# Bank 0
	# First 10K bank is the bootstrap plus the relevant common bits
	dd if=../../common.bin of=bootstrap.bin bs=512 skip=2 seek=1 count=19
	# Now pack the kernel banks
	# Bank 1
	dd if=../../bank1.bin bs=512 skip=96 count=20 >block.tmp
	dd if=block.tmp bs=10240 conv=sync count=1 >>bootstrap.bin
	# Bank 2
	dd if=../../bank2.bin bs=512 skip=96 count=20 >block.tmp
	dd if=block.tmp bs=10240 conv=sync count=1 >>bootstrap.bin
	# Bank 3
	dd if=../../bank3.bin bs=512 skip=96 count=20 >block.tmp
	dd if=block.tmp bs=10240 conv=sync count=1 >>bootstrap.bin
	# Bank 4
	dd if=../../bank4.bin bs=512 skip=96 count=20 >block.tmp
	dd if=block.tmp bs=10240 conv=sync count=1 >>bootstrap.bin
	# Bank 5: Code for 4000
	dd if=../../common.bin bs=512 skip=32 count=16 >>bootstrap.bin
	dd if=/dev/zero bs=2048 count=1 >>bootstrap.bin
	# Bank 6: Code for 6000
	dd if=../../common.bin bs=512 skip=48 count=16 >>bootstrap.bin
	dd if=/dev/zero bs=2048 count=1 >>bootstrap.bin
	# Bank 7: Discard for 8200
	dd if=../../common.bin bs=512 skip=65 count=20 >>bootstrap.bin
	./megarom_shuffle <bootstrap.bin >mega.rom

megarom_shuffle: ../../tools/megarom_shuffle.c
	gcc -Wall -pedantic -Werror ../../tools/megarom_shuffle.c -o megarom_shuffle
	
image:	bootstrap

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.20M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
	# ROM
	cp mega.rom $(IMAGES)/mega.rom
