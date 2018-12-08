ASRCS = crt0.s tbblue.s tmxvideo.s
ASRCS += tricks.s commonmem.s
CSRCS = devtty.c devices.c main.c
CDSRCS = discard.c
DSRCS = ../dev/devsd.c ../dev/blkdev.c
DDSRCS = ../dev/devsd_discard.c ../dev/mbr.c
DZSRCS = ../dev/zx/divmmc.c ../dev/zx/zxkeyboard.c ../dev/zx/devinput.c
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
	rm -f FUZIX FUZIX.BIN load-esx.ihx load-esx.tmp

image:
	# Build an esxdos friendly setup
	sdasz80 -o load-esx.s
	sdldz80 -i load-esx.rel
	makebin -s 8704 load-esx.ihx load-esx.tmp
	# Generate the image file we need
	dd if=load-esx.tmp of=FUZIX bs=8192 skip=1

	dd if=/dev/zero bs=256 count=1 >FUZIX.BIN
	cat ../fuzix.bin >>FUZIX.BIN
