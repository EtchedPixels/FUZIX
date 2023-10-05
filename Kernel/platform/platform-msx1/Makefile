CROSS_CCOPTS += -I../../dev/ -I../../dev/net/

CSRCS = devtty.c devfd.c devlpr.c devide_sunrise.c
CSRCS += devices.c main.c

DISCSRCS = discard.c

ASRCS = msx1.s crt0.s vdp.s slots.s cartridge.s
ASRCS += tricks.s commonmem.s sunrise.s

DISCARD_DSRCS =
DSRCS = ../../dev/tinydisk.c ../../dev/tinydisk_discard.c

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DISCOBJS = $(DISCSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))

OBJS  = $(COBJS) $(AOBJS) $(DISCOBJS) $(DISCARD_DOBJS) $(DOBJS)

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DISCOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(DISCARD_DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f *.rel *.sym *.ihx *.lst *.rst  core *~

image:
	../../tools/cartman ../../fuzix.bin ../../fuzix.map fuzix.cart

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	cp fuzix.cart $(IMAGES)
