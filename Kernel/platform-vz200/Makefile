ASRCS = crt0.s tricks.s commonmem.s vz200.s
CSRCS = devices.c main.c devtty.c sd.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../dev/tinysd_discard.c
DSRCS = ../dev/tinysd.c
DASRCS =
NSRCS = 

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DHOBJS = $(patsubst ../dev/%.c,%.rel, $(DHSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DHOBJS) $(DAOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(NOBJS)

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
	rm -f $(OBJS) $(JUNK)  core *~ diskstrap fuzix.rom booter.tmp

image: booter
	dd if=../fuzix.bin of=fuzix.rom bs=32768 skip=1 count=1
	# Build the disk bootstrap 
	dd if=../fuzix.bin of=diskstrap bs=256 skip=1 count=128

booter:
	sdasz80 -o booter.s
	sdldz80 -i booter.rel
	makebin -s 65535 booter.ihx booter.tmp
	dd if=booter.tmp of=booter.vz bs=1 skip=16360 count=536
	rm -f booter.tmp

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel extra blocks
	dd if=diskstrap of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
	# ROM
	cp fuzix.rom $(IMAGES)/fuzix.rom
