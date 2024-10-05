
CSRCS =
DISCARD_CSRCS = discard.c devhd_discard.c
NSRCS = 

ASRCS = trs80.s trs80-bank.s crt0.s
ASRCS += tricks.s commonmem.s floppy.s

CSRCS = devfd.c devhd.c
CSRCS += devices.c main.c devinput.c
CSRCS += devlpr.c devtty.c
DSRCS =
DISCARD_DSRCS =

COBJS = $(CSRCS:.c=.rel)
DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))
AOBJS = $(ASRCS:.s=.rel)
NOBJS = $(patsubst ../../dev/net/%.c,%.rel, $(NSRCS))
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DISCARD_DSRCS))
OBJS  = $(COBJS) $(AOBJS) $(DISCARD_COBJS) $(DOBJS) $(DISCARD_DOBJS) $(NOBJS)

CROSS_CCOPTS += -I../../dev/

all:	$(OBJS) trs80load.bin

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(DISCARD_DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

clean:
	rm -f *.rel *.lst *.asm *.lst *.sym *.adb *.rst *.ihx core *~ boot.raw

image: trs80load.bin
	dd if=/dev/zero of=boot.raw bs=256 count=400
	# Boot block at 0,0
	dd if=trs80load.bin of=boot.raw bs=256 count=1 conv=notrunc
	# With the image straight afterwards
	# Copy 0100-36FF
	dd if=../../fuzix.bin of=boot.raw bs=256 seek=10 skip=1 count=54 conv=notrunc
	# Copy 4000-FFFF
	dd if=../../fuzix.bin of=boot.raw bs=256 skip=64 count=192 seek=64 conv=notrunc
	../../tools/makejv3 -s -t sd40s -d boot.raw boot.jv3

trs80load.bin: trs80load.s
	sdasz80 -o trs80load.s
	sdldz80 -i trs80load.rel -m
	makebin -s 256 trs80load.ihx trs80load.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	cp boot.jv3 $(IMAGES)/boot.jv3
	(cd $(FUZIX_ROOT)/Standalone/filesystem-src; ./build-filesystem $(ENDIANFLAGH) $(IMAGES)/filesys.img 256 21760)
	cat xtrsheader $(IMAGES)/filesys.img >$(IMAGES)/hard1-0
	../../tools/trslabel $(IMAGES)/hard1-0
