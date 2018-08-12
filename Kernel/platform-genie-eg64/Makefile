
CSRCS =
DISCARD_CSRCS = discard.c devhd_discard.c
NSRCS = ../dev/net/net_native.c

ASRCS = trs80.s trs80-bank.s crt0.s
ASRCS += tricks.s commonmem.s floppy.s ide.s

CSRCS = devfd.c devhd.c
CSRCS += devices.c main.c devinput.c
CSRCS += devlpr.c devtty.c devgfx.c
DSRCS = ../dev/blkdev.c ../dev/devide.c ../dev/mbr.c
DISCARD_DSRCS = ../dev/devide_discard.c

COBJS = $(CSRCS:.c=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
AOBJS = $(ASRCS:.s=.rel)
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
OBJS  = $(COBJS) $(AOBJS) $(DISCARD_COBJS) $(DOBJS) $(DISCARD_DOBJS) $(NOBJS)

CROSS_CCOPTS += -I../dev/

all:	$(OBJS) trs80load.bin

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

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
	rm -f *.rel *.lst *.asm *.lst *.sym *.adb *.rst *.ihx core *~ boot.raw

image: trs80load.bin
	dd if=/dev/zero of=boot.raw bs=256 count=400
	# Boot block at 0,0
	dd if=trs80load.bin of=boot.raw bs=256 count=1 conv=notrunc
	# With the image straight afterwards
	dd if=../fuzix.bin of=boot.raw bs=256 skip=1 conv=notrunc
	../tools/makejv3 -s -t sd40s -d boot.raw boot.jv3

trs80load.bin: trs80load.s
	sdasz80 -o trs80load.s
	sdldz80 -i trs80load.rel
	makebin -s 17152 trs80load.ihx trs80load.tmp
	dd if=trs80load.tmp of=trs80load.bin bs=256 skip=66 count=1
