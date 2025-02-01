CSRCS = devtty.c devices.c main.c devrd.c rd_cpcsme.c
CDSRCS = discard.c
DSRCS = ../../dev/tinyide.c ../../dev/tinydisk.c ../../dev/devfdc765.c ../../dev/ch375.c
DDSRCS = ../../dev/tinyide_discard.c ../../dev/tinydisk_discard.c
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
	# Create a snapshot file with fuzix code at right places and set execution at 0x100
	dd if=../../fuzix.bin of=bank012.bin bs=48896 count=1
	dd if=../../fuzix.bin of=bank7.bin bs=1 skip=48896
	createSnapshot $(IMAGES)/6128.sna -l bank012.bin 0x100 -l bank7.bin 0x1c000 -s Z80_PC 0x100

	#Assemble loader
	sdasz80 -l -o loader.s
	sdldz80 -i loader.rel
	hex2bin loader.ihx
	# Make a system disk raw image to work from with loader at cpm boot sector followed by fuzix.bin
	dd if=/dev/zero of=padding.bin bs=512 count=360
	dd if=loader.bin of=padding.bin seek=0 bs=512 conv=notrunc
	dd if=../../fuzix.bin of=padding.bin bs=512 seek=1 conv=notrunc
	cat padding.bin >disk.raw
	# And generate a 40 track cpc system disk dsk file from it
	../../tools/raw2dskcpc disk.raw fuzix.dsk 40 1 64
	cp fuzix.dsk $(IMAGES)/fuzix.dsk
	
IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image and partition it P1->32M,FUZIX P2->8M,FUZIX P3->4M,FUZIX.SWAP P4->(64-(32+8+4+1(reserved)))M,FAT16
	dd if=/dev/zero of=$(IMAGES)/disk.img bs=512 count=131072
	echo 'label: dos' | sfdisk $(IMAGES)/disk.img
	echo -e 'size=32M, id=7e\n size=8M, id=7e\n size=4M, id=7f\n size=+, id=4\n' | sfdisk $(IMAGES)/disk.img
	
	# Make a FAT16 filesystem for partition P4 so we can play with dos utils (dosread, etc.), root directory entries are limited by available RAM
	dd if=/dev/zero of=$(IMAGES)/FAT16.img bs=512 count=38912
	mkfs.fat -F 16 -r 128 -v $(IMAGES)/FAT16.img #we could mount this now as loop device and put stuff on it before adding to the disk image

   	# Add the file systems
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=67584 conv=notrunc
	dd if=$(IMAGES)/FAT16.img of=$(IMAGES)/disk.img bs=512 seek=92160 conv=notrunc
	
	# Make a 720k floppy with minimal root filesystem
	(cd $(FUZIX_ROOT)/Standalone/filesystem-src; ./build-mini-filesystem $(ENDIANFLAG) $(FUZIX_ROOT)/Images/$(TARGET)/root.raw 64 1440)
	../../tools/raw2dskcpc $(FUZIX_ROOT)/Images/$(TARGET)/root.raw $(FUZIX_ROOT)/Images/$(TARGET)/root.dsk 80 2 0
	