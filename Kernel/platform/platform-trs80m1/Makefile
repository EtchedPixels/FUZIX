
CSRCS = devlpr.c devtty.c
DISCARD_CSRCS = discard.c devhd_discard.c
NSRCS = ../dev/net/net_native.c

ASRCS = trs80.s trs80-bank.s crt0.s vtsupport.s
ASRCS += tricks.s commonmem.s floppy.s floppy3.s stringy.s ide.s

# buffers.c must be in CODE2, direct users are more convenient there as
# the asm helpers then can avoid another switch
C2SRCS = buffers.c devfd.c devhd.c
# And these so CODE1 is under 32K
C2SRCS += devices.c main.c devstringy.c devinput.c devgfx.c
DSRCS = ../dev/tinydisk.c ../dev/tinyide.c
DISCARD_DSRCS = ../dev/tinyide_discard.c ../dev/tinydisk_discard.c


COBJS = $(CSRCS:.c=.rel)
C2OBJS = $(C2SRCS:.c=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
AOBJS = $(ASRCS:.s=.rel)
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
OBJS  = $(COBJS) $(C2OBJS) $(AOBJS) $(DISCARD_COBJS) $(DOBJS) $(DISCARD_DOBJS) $(NOBJS)

CROSS_CCOPTS += -I../dev/

all:	$(OBJS) trs80load.bin

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(C2OBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

clean:
	rm -f *.rel *.lst *.asm *.lst *.sym *.adb *.rst *.ihx core *~

image: trs80load.bin trs80load-m3.bin
	dd if=/dev/zero of=boot.raw bs=256 count=400
	# Boot block at 0,0
	dd if=trs80load.bin of=boot.raw bs=256 count=1 conv=notrunc
	# 16K of common at 1,0
	dd if=../common.bin of=boot.raw bs=256 count=61 seek=10 conv=notrunc skip=67
	# Followed directly by bank1 (32K)
	dd if=../bank1.bin of=boot.raw bs=256 count=128 seek=71 conv=notrunc skip=128
	# And then bank2 (32K)
	dd if=../bank2.bin of=boot.raw bs=256 count=128 seek=199 conv=notrunc skip=128
	# And turn it into a single sided 40 track fm image
	../tools/makejv3 -s -t sd40s -d boot.raw boot.jv3

	dd if=/dev/zero of=boot.raw bs=256 count=720
	# Boot block at 0,0
	dd if=trs80load-m3.bin of=boot.raw bs=256 count=1 conv=notrunc
	# 16K of common at 1,0
	dd if=../common.bin of=boot.raw bs=256 count=61 seek=18 conv=notrunc skip=67
	# Followed directly by bank1 (32K)
	dd if=../bank1.bin of=boot.raw bs=256 count=128 seek=79 conv=notrunc skip=128
	# And then bank2 (32K)
	dd if=../bank2.bin of=boot.raw bs=256 count=128 seek=207 conv=notrunc skip=128
	# And turn it into a single sided 40 track fm image
	../tools/makejv3 -s -t dd40s -d boot.raw boot3.jv3

trs80load.bin: trs80load.s
	sdasz80 -o trs80load.s
	sdldz80 -i trs80load.rel
	makebin -s 17152 trs80load.ihx trs80load.tmp
	dd if=trs80load.tmp of=trs80load.bin bs=256 skip=66 count=1

trs80load-m3.bin: trs80load-m3.s
	sdasz80 -o trs80load-m3.s
	sdldz80 -i trs80load-m3.rel
	makebin -s 17408 trs80load-m3.ihx trs80load-m3.tmp
	dd if=trs80load-m3.tmp of=trs80load-m3.bin bs=256 skip=66 count=1

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	cp boot.jv3 $(IMAGES)
	cp boot3.jv3 $(IMAGES)
	(cd $(FUZIX_ROOT)/Standalone/filesystem-src; ./build-filesystem $(ENDIANFLAGH) $(IMAGES)/filesys.img 256 21760)
	cat xtrsheader $(IMAGES)/filesys.img >$(IMAGES)/hard1-0
	../tools/trslabel $(IMAGES)/hard1-0
