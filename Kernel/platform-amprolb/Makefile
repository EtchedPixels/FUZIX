ASRCS = crt0.s tricks.s commonmem.s lb.s ncr5380.s dart.s floppy.s
CSRCS = devices.c main.c devtty.c devfd.c devscsi.c devlpr.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../dev/tinydisk_discard.c ../dev/tinyscsi_discard.c
DSRCS = ../dev/tinydisk.c ../dev/tinyscsi.c
DHSRCS = 
DSRCS +=  
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

CROSS_CC_HIGH = --codeseg COMMONMEM

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_HIGH) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_HIGH) -c $<

$(DHOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_HIGH) -c $<

$(DAOBJS): %.rel: ../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ diskstrap fuzix.rom
	rm -f cpmboot.tmp cpmboot.com

cpmboot.com: cpmboot.s
	sdasz80 -o cpmboot.s
	sdldz80 -i cpmboot.rel
	makebin -s 65535 cpmboot.ihx cpmboot.tmp
	dd if=cpmboot.tmp of=cmpboot.com bs=256 skip=1

image: cpmboot.com
	dd if=../fuzix.bin of=fuzix.rom bs=32768 count=1
	# We have a 512 byte hole we leave for the loader
	dd if=../fuzix.bin of=diskstrap bs=33280 count=1 skip=1
	cat diskstrap >>cpmboot.com

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel extra blocks
	dd if=diskstrap of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc
	cp cpmboot.com $(IMAGES)
	cp fuzix.rom $(IMAGES)
