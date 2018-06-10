
CSRCS = devlpr.c devtty.c devfd.c devfd3.c devhd.c devgfx.c
CSRCS += devices.c main.c
DISCARD_CSRCS = discard.c devhd_discard.c

ASRCS = trs80.s trs80-bank.s crt0.s
ASRCS += tricks.s commonmem.s floppy.s floppy3.s

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
OBJS  = $(COBJS) $(AOBJS) $(DISCARD_COBJS) $(DOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = $(CSRCS:.c=.lst) $(CSRCS:.c=.asm) $(CSRCS:.c=.sym) $(ASRCS:.s=.lst) $(ASRCS:.s=.sym) $(CSRCS:.c=.rst) $(ASRCS:.s=.rst)

all:	$(OBJS) trs80load.bin

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

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
