ASRCS = crt0.s zxevo.s video.s
ASRCS += tricks.s commonmem.s
CSRCS = devtty.c devices.c main.c
CDSRCS = discard.c
DSRCS = ../dev/devide.c ../dev/devsd.c ../dev/blkdev.c
DDSRCS = ../dev/devide_discard.c ../dev/devsd_discard.c ../dev/mbr.c
DZSRCS = ../dev/zx/evommc.c ../dev/zx/zxkeyboard.c ../dev/zx/devinput.c
DDZSRCS =
NSRCS = ../dev/net/net_native.c

COBJS = $(CSRCS:.c=.rel)
CDOBJS = $(CDSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DDOBJS = $(patsubst ../dev/%.c,%.rel, $(DDSRCS))
DZOBJS = $(patsubst ../dev/zx/%.c,%.rel, $(DZSRCS))
DDZOBJS = $(patsubst ../dev/zx/%.c,%.rel, $(DDZSRCS))
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))

OBJS  = $(COBJS) $(CDOBJS) $(AOBJS) $(DOBJS) $(DDOBJS) $(DZOBJS) $(DDZOBJS) $(NOBJS)

CROSS_CCOPTS += -I../dev/ -I../dev/zx/ -I../dev/net/

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(CDOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DDOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DZOBJS): %.rel: ../dev/zx/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DDZOBJS): %.rel: ../dev/zx/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.rel core *~ 
	rm -f load-fat.ihx load-fat.tmp load-hd.ihx load-hd.tmp
	rm -f load-fat load-hd FUZIX.BIN
image:
	sdasz80 -o load-hd.s
	sdldz80 -i load-hd.rel
	makebin -s 25088 load-hd.ihx load-hd.tmp
	# Generate the image file we need
	# 512 bytes at 24576
	dd if=load-hd.tmp of=load-hd bs=512 skip=48

	sdasz80 -o load-fat.s
	sdldz80 -i load-fat.rel
	makebin -s 25088 load-fat.ihx load-fat.tmp
	# Generate the image file we need
	# 512 bytes at 24576
	dd if=load-fat.tmp of=load-fat bs=512 skip=48
	dd if=/dev/zero bs=256 count=1 >FUZIX.BIN
	cat ../fuzix.bin >>FUZIX.BIN
