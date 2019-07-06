CSRCS = devtty.c devices.c main.c
CDSRCS = discard.c
DSRCS = ../dev/devsd.c ../dev/blkdev.c
DDSRCS = ../dev/devsd_discard.c ../dev/mbr.c
DZSRCS = ../dev/zx/divmmc.c ../dev/zx/zxkeyboard.c
DZSRCS += ../dev/zx/devinput.c ../dev/zx/bank128.c
DDZSRCS = ../dev/zx/zxuno.c
ASRCS = crt0.s zx128.s zxvideo.s
ASRCS += tricks.s commonmem.s

COBJS = $(CSRCS:.c=.rel)
CDOBJS = $(CDSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DDOBJS = $(patsubst ../dev/%.c,%.rel, $(DDSRCS))
DZOBJS = $(patsubst ../dev/zx/%.c,%.rel, $(DZSRCS))
DDZOBJS = $(patsubst ../dev/zx/%.c,%.rel, $(DDZSRCS))
OBJS  = $(COBJS) $(CDOBJS) $(AOBJS) $(DOBJS) $(DDOBJS) $(DZOBJS) $(DDZOBJS)

CROSS_CCOPTS += -I../dev/ -I../dev/zx/

CROSS_CC_SEG3 = --codeseg CODE3

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) -c $<

$(CDOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) -c $<

$(DDOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DZOBJS): %.rel: ../dev/zx/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) -c $<

$(DDZOBJS): %.rel: ../dev/zx/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.rel core *~ 
	rm -f BOOT.BIN strap.bin load-esx.ihx load-esx.tmp
	rm -f FUZIX.BIN FUZIX

# Re-order the image and snapshop it
image:
	# Merge the first code bank into common
	dd if=../bank1.bin bs=16384 skip=3 seek=3 count=1 \
		of=../common.bin conv=notrunc

	# Build an esxdos friendly setup
	sdasz80 -o load-esx.s
	sdldz80 -i load-esx.rel
	makebin -s 8704 load-esx.ihx load-esx.tmp
	# Generate the image file we need
	dd if=load-esx.tmp of=FUZIX bs=8192 skip=1

	# Now build a FUZIX.BIN to load
	# in the order we need it
	dd if=../common.bin bs=65536 count=1 conv=sync | \
		dd of=FUZIX.BIN
	dd if=../bank2.bin bs=16384 skip=3 >>FUZIX.BIN conv=sync
	dd if=../bank3.bin bs=16384 skip=3 >>FUZIX.BIN conv=sync
