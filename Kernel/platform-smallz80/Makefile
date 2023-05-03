CROSS_CCOPTS += -I../dev/


CSRCS = devtty.c devices.c devrtc.c main.c

DISCSRCS = discard.c

ASRCS = smallz80.s crt0.s
ASRCS += tricks.s commonmem.s

DISCARD_DSRCS = ../dev/devide_discard.c
DSRCS = ../dev/blkdev.c ../dev/devide.c ../dev/mbr.c
NSRCS =

DASRCS = 

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))
DISCOBJS = $(DISCSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))

OBJS  = $(COBJS) $(AOBJS) $(NOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS)

JUNK =  *.lst *.asm *.sym *.rst *.lst *.rel *.tmp *.ihx

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DAOBJS): %.rel: ../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ loader.bin *.tmp fuzix

loader.bin: loader.s
	sdasz80 -o loader.s
	sdldz80 -i loader.rel
	makebin -s 65536 loader.ihx loader.tmp
	dd if=loader.tmp bs=256 skip=250 of=loader2.tmp
	dd if=loader2.tmp bs=1536 conv=sync of=loader.bin count=1

image: loader.bin
	# Kernel below E800
	dd if=../fuzix.bin of=base.tmp bs=59392 count=1
	# Kernel above F600
	dd if=../fuzix.bin of=top.tmp bs=62976 skip=1 count=1
	# Pack the pieces together
	cat base.tmp top.tmp >fuzix

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image
	dd if=partition.bin of=$(IMAGES)/disk.img bs=134217728 conv=sync count=1
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=fuzix of=$(IMAGES)/disk.img bs=512 seek=3 conv=notrunc
	# Add the bootstrap
	dd if=loader.bin of=$(IMAGES)/disk.img conv=notrunc
	# Put the partition bits in
	dd if=partition.bin seek=0 of=$(IMAGES)/disk.img bs=1 seek=446 skip=446 count=66 conv=notrunc
	# Make an emulator image of it. Geometry matters for this system
	cat idehdr.bin $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
