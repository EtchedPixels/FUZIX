
DSRCS = ../dev/devsd.c ../dev/mbr.c ../dev/blkdev.c
CSRCS += devfd.c devhd.c devlpr.c
CSRCS += devices.c main.c devtty.c ../dev/rp5c01.c devrtc.c ../dev/v99xx.c vdp.c
COMMON_CSRCS = devmegasd.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../dev/devsd_discard.c
ASRCS = msx2.s crt0.s
ASRCS += tricks.s commonmem.s bootrom.s diskboot.s

CROSS_CCOPTS += -I../dev/

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
COMMON_COBJS = $(COMMON_CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(DISCARD_COBJS) $(DISCARD_DOBJS) $(COMMON_COBJS)

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) --codeseg CODE2 -c $<

$(COMMON_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) --codeseg CODE1 -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.lk  core *~
	rm -f fuzix.ascii8.rom

image:
	dd if=../fuzix.bin of=fuzix.tmp bs=256 skip=1
	dd if=../fuzix_disk.bin of=fuzix.com bs=256 skip=241
	cp fuzix.com /tmp/com1
	cat fuzix.tmp >> fuzix.com
#	rm -f fuzix.tmp
	dd if=../fuzix_boot.bin of=fuzix.ascii8.rom skip=1 bs=16384 conv=sync
	dd if=../fuzix.bin of=fuzix.ascii8.rom seek=1 bs=16384 conv=sync

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	cp fuzix.ascii8.rom $(IMAGES)
	cp fuzix.com $(IMAGES)
