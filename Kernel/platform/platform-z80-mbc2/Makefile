CROSS_CCOPTS += -I../../dev/

CSRCS = devtty.c devices.c main.c devdisk.c devrtc.c

DISCSRCS = discard.c

ASRCS = z80-mbc2.S crt0.S
ASRCS += tricks.S commonmem.S

DISCARD_DSRCS = 
DSRCS = ../../dev/blkdev.c ../../dev/mbr.c
NSRCS =

DASRCS =

COBJS = $(CSRCS:.c=.o)
AOBJS = $(ASRCS:.S=.o)
NOBJS = $(patsubst ../../dev/net/%.c,%.o, $(NSRCS))
DISCOBJS = $(DISCSRCS:.c=.o)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.S,%.o, $(DASRCS))

OBJS  = $(COBJS) $(AOBJS) $(NOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS)

JUNK =  *.o

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCOBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.o: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

$(DAOBJS): %.o: ../../dev/%.S
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
