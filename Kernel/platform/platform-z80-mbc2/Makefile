CROSS_CCOPTS += -I../../dev/

CSRCS = devtty.c devices.c main.c devdisk.c devrtc.c

DISCSRCS = discard.c

ASRCS = z80-mbc2.s crt0.s
ASRCS += tricks.s commonmem.s

DISCARD_DSRCS = 
DSRCS = ../../dev/blkdev.c ../../dev/mbr.c
NSRCS =

DASRCS =

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
NOBJS = $(patsubst ../../dev/net/%.c,%.rel, $(NSRCS))
DISCOBJS = $(DISCSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.s,%.rel, $(DASRCS))

OBJS  = $(COBJS) $(AOBJS) $(NOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS)

JUNK =  *.lst *.asm *.sym *.rst *.lst *.rel

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DAOBJS): %.rel: ../../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

clean:
	rm -f $(JUNK)  core *~

image:

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	(cd ../../../../Standalone/filesystem-src; ./build-filesystem $(IMAGES)/filesys.img 256 12288 )
	dd if=parttab of=$(IMAGES)/DS0N01.DSK bs=512 count=16384
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/DS0N01.DSK bs=512 seek=4096 conv=notrunc
	cp ../../fuzix.bin $(IMAGES)
