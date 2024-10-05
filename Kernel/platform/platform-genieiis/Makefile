
CSRCS = devlpr.c devtty.c devfd.c devhd.c
CSRCS += devices.c main.c devinput.c devgfx.c
DISCARD_CSRCS = discard.c devhd_discard.c
NSRCS = ../../dev/net/net_native.c

ASRCS = genieiis.s ram192-bank.s crt0.s vtsupport.s
ASRCS += tricks.s commonmem.s floppy.s

DSRCS = 
DISCARD_DSRCS = 

COBJS = $(CSRCS:.c=.rel)
C2OBJS = $(C2SRCS:.c=.rel)
DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))
AOBJS = $(ASRCS:.s=.rel)
NOBJS = $(patsubst ../../dev/net/%.c,%.rel, $(NSRCS))
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DISCARD_DSRCS))
OBJS  = $(COBJS) $(C2OBJS) $(AOBJS) $(DISCARD_COBJS) $(DOBJS) $(DISCARD_DOBJS) $(NOBJS)

CROSS_CCOPTS += -I../../dev/

all:	$(OBJS) trs80load.bin

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(C2OBJS): %.rel: %.c
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
	rm -f *.rel *.lst *.asm *.lst *.sym *.adb *.rst *.ihx core *~

image: genieload.bin
	# TODO: image for disk
	# And turn it into a single sided 40 track fm image
	../../tools/makejv3 -s -t sd40s -d boot.raw boot.jv3

genieload.bin: genieload.s
	sdasz80 -o genieload.s
	sdldz80 -i genieload.rel
	makebin -s 65536 genieload.ihx genieload.tmp
	dd if=genieload.tmp of=genieload.bin bs=256 skip=255 count=1

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	cp boot.jv3 $(IMAGES)
	(cd $(FUZIX_ROOT)/Standalone/filesystem-src; ./build-filesystem $(ENDIANFLAGH) $(IMAGES)/filesys.img 256 21760)
	cat xtrsheader $(IMAGES)/filesys.img >$(IMAGES)/hard1-0
	../../tools/trslabel $(IMAGES)/hard1-0
