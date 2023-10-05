CSRCS = devtty.c devices.c main.c buffers.c divide.c zxcf.c devrd.c ide.c
CDSRCS = discard.c
DSRCS = ../dev/devide.c ../dev/blkdev.c
DDSRCS = ../dev/devide_discard.c ../dev/mbr.c
DZSRCS = ../dev/zx/zxkeyboard.c
DZSRCS += ../dev/zx/devinput.c
DDZSRCS =
ASRCS = crt0.s zx48.s zxvideo.s rd_divideplus.s
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
	rm -f *.ihx *.tmp loader loader-zxcf
	rm -f FUZIX.BIN FUZIX

# Re-order the image and turn it into the raw image to load from disk
image:
	# Build a generic DivIDE plus loader
	sdasz80 -o loader.s
	sdldz80 -i loader.rel
	makebin -s 57856 loader.ihx loader.tmp
	dd bs=1 skip=57344 if=loader.tmp of=loader
	# Build a generic ZXCF+ style loader
	sdasz80 -o loader-zxcf.s
	sdldz80 -i loader-zxcf.rel
	makebin -s 57856 loader-zxcf.ihx loader-zxcf.tmp

	# Now build a FUZIX.BIN to load
	# in the order we need it
	dd if=../bank1.bin bs=16384 >FUZIX.BIN conv=sync
	dd if=../bank2.bin bs=16384 >>FUZIX.BIN conv=sync
	dd if=../bank3.bin bs=16384 >>FUZIX.BIN conv=sync
	dd if=../bank4.bin bs=16384 >>FUZIX.BIN conv=sync
	dd if=../common.bin bs=16384 >>FUZIX.BIN skip=1 count=1 conv=sync
	dd if=../common.bin bs=16384 >>FUZIX.BIN skip=2 count=1 conv=sync
