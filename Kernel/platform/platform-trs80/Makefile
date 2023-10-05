
CSRCS = devlpr.c devtty.c devfd.c devhd.c devgfx.c
CSRCS += devices.c main.c devinput.c
DISCARD_CSRCS = discard.c devhd_discard.c

ASRCS = trs80.s trs80-bank.s crt0.s
ASRCS += tricks.s commonmem.s floppy.s drivewire.s

DSRCS = ../../dev/devdw.c

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DOBJS = $(patsubst ../../dev/%.c,%.rel, $(DSRCS))
OBJS  = $(COBJS) $(AOBJS) $(DISCARD_COBJS) $(DOBJS)

CROSS_CCOPTS += -I../../dev/

all:	$(OBJS) trs80load.bin

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DOBJS): %.rel: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.lk  core *~

image: trs80load.bin hdload.bin
	dd if=/dev/zero of=boot.raw bs=256 count=720
	# Boot block at 0,0
	dd if=trs80load.bin of=boot.raw bs=256 count=1 conv=notrunc
	# Kernel lives on the end of the disk (so we can add an fs)
	dd if=../../fuzix.bin of=boot.raw bs=1 seek=129024 conv=notrunc
	../../tools/makejv3 -s -t dd40s -d boot.raw boot4p.jv3
	dd if=hdload.bin of=hdboot.raw bs=256 count=1 conv=notrunc
	../../tools/makejv3 -s -t dd40s -d hdboot.raw hdboot4p.jv3

trs80load.bin: trs80load.s
	sdasz80 -o trs80load.s
	sdldz80 -i trs80load.rel
	makebin -s 256 trs80load.ihx trs80load.bin

hdload.bin: hdload.s
	sdasz80 -o hdload.s
	sdldz80 -i hdload.rel
	makebin -s 256 hdload.ihx hdload.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Boot floppy
	cp boot4p.jv3 $(IMAGES)/boot.jv3
	cp hdboot4p.jv3 $(IMAGES)/hdboot.jv3
	# Build a file system to fit
	(cd $(FUZIX_ROOT)/Standalone/filesystem-src; ./build-filesystem $(ENDIANFLAGH) $(IMAGES)/filesys.img 256 21760)
	cat xtrsheader $(IMAGES)/filesys.img >$(IMAGES)/hard4p-0
	../../tools/trslabel $(IMAGES)/hard4p-0
