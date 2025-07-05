CSRCS = devtty.c devices.c main.c devfdc765.c wiznet.c
CDSRCS = discard.c
DSRCS = ../../dev/tinyide.c ../../dev/tinydisk.c 
DDSRCS = ../../dev/tinyide_discard.c ../../dev/tinydisk_discard.c 
DZSRCS = ../../dev/cpc/cpcidesme.c ../../dev/cpc/cpckeyboard.c ../../dev/cpc/devinput.c
DZSRCS += ../../dev/cpc/ch375.c ../../dev/cpc/td_block_io.c
DDZSRCS = ../../dev/cpc/ch375_discard.c
ASRCS = crt0.s cpcsme.s cpcvideo.s fdc765.s
ASRCS += usermem.s tricks.s commonmem.s
NSRCS = ../../dev/net/net_w5x00.c ../../dev/net/net_native.c

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
CDOBJS = $(CDSRCS:.c=.rel)

DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))
DDOBJS = $(patsubst ../../dev/%.c,%.rel, $(DDSRCS))
DZOBJS = $(patsubst ../../dev/cpc/%.c,%.rel, $(DZSRCS))
DDZOBJS = $(patsubst ../../dev/cpc/%.c,%.rel, $(DDZSRCS))
NOBJS = $(patsubst ../../dev/net/%.c,%.rel, $(NSRCS))
OBJS  = $(AOBJS) $(COBJS) $(CDOBJS) $(DOBJS) $(DDOBJS) $(DZOBJS) $(DDZOBJS) $(NOBJS)

CROSS_CCOPTS += -I../../dev/ -I../../dev/cpc/ -I../../dev/net 


all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<
	
$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(CDOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DDOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DZOBJS): %.rel: ../../dev/cpc/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DDZOBJS): %.rel: ../../dev/cpc/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<



clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.rel core *~ 
	rm -f loader.tmp loader.ihx loader.lst loader.bin padding.bin disk.raw
	rm -f fuzix1.bin fuzix2.bin fuzix3.bin fuzix4.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

image:
# Create a snapshot file with fuzix code at right places and set execution at 0x100
	createSnapshot $(IMAGES)/fuzixcpc.sna -l ../../fuzix.bin 0x10100 -s Z80_PC 0x100 -s GA_RAMCFG 0xc2

#Assemble loader
	sdasz80 -l -o runfuzix.s
	sdldz80 -i runfuzix.rel
	hex2bin runfuzix.ihx
#Split fuzix bin in basic loadable chunks
	dd if=../../fuzix.bin of=fuzix1.bin bs=16128 count=1
	dd if=../../fuzix.bin of=fuzix2.bin bs=1 count=16384 skip=16128
	dd if=../../fuzix.bin of=fuzix3.bin bs=1 count=16384 skip=32512
	dd if=../../fuzix.bin of=fuzix4.bin bs=1 skip=48896
#Create DSK image
	iDSK $(IMAGES)/fuzix.dsk -n
#Convert to CPC ASCII BASIC file format
	flip -m fuzix.bas
#Insert files to DSK image
	iDSK $(IMAGES)/fuzix.dsk -i fuzix.bas -t 0 -f
	iDSK $(IMAGES)/fuzix.dsk -i fuzix.scr -t 1 -c c000 -f
	iDSK $(IMAGES)/fuzix.dsk -i runfuzix.bin -e 4000 -c 4000 -t 1 -f
	iDSK $(IMAGES)/fuzix.dsk -i fuzix1.bin -t 1 -c 4100 -f
	iDSK $(IMAGES)/fuzix.dsk -i fuzix2.bin -t 1 -c 4000 -f
	iDSK $(IMAGES)/fuzix.dsk -i fuzix3.bin -t 1 -c 4000 -f
	iDSK $(IMAGES)/fuzix.dsk -i fuzix4.bin -t 1 -c 4000 -f

diskimage: 
# Make a blank disk image and partition it P1->32M,FUZIX P2->8M,FUZIX P3->4M,FUZIX.SWAP P4->(64-(32+8+4+1(reserved)))M,FAT16
	dd if=/dev/zero of=$(IMAGES)/disk.img bs=512 count=131072
	echo 'label: dos' | sfdisk $(IMAGES)/disk.img
	sfdisk $(IMAGES)/disk.img < diskimage.sfdisk
	
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
