
CSRCS = devlpr.c devtty.c devfd.c devhd.c devgfx.c
CSRCS += devices.c main.c
DISCARD_CSRCS = discard.c devhd_discard.c

ASRCS = trs80.s trs80-bank.s crt0.s
ASRCS += tricks.s commonmem.s floppy.s

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

image: trs80load.bin
	dd if=/dev/zero of=boot.raw bs=256 count=400
	# Boot block at 0,0
	dd if=trs80load.bin of=boot.raw bs=256 count=1 conv=notrunc
	# 16K of common at 1,0
	dd if=../common.bin of=boot.raw bs=256 count=61 seek=10 conv=notrunc
	# Followed directly by bank1 (32K)
	dd if=../bank1.bin of=boot.raw bs=256 count=128 seek=71 conv=notrunc
	# And then bank2 (32K)
	dd if=../bank2.bin of=boot.raw bs=256 count=128 seek=199 conv=notrunc
	# And turn it into a single sided 40 track fm image
	../tools/makejv3 sd40s boot.raw boot.jv3

trs80load.bin: trs80load.s
	sdasz80 -o trs80load.s
	sdldz80 -i trs80load.rel
	makebin -s 17152 trs80load.ihx trs80load.tmp
	dd if=trs80load.tmp of=trs80load.bin bs=256 skip=66 count=1
