
CSRCS = devnascom.c devgm8x9.c devgm833.c mm58174.c
CSRCS += devices.c main.c devinput.c
DISCARD_CSRCS = discard.c

ASRCS = nascom.s nascom-pagemode.s nascom-vt.s crt0.s
ASRCS += tricks.s commonmem.s gm8x9.s gm849a_sasi.s gm833.s

DSRCS = ../dev/devscsi.c ../dev/blkdev.c ../dev/mbr.c
DISCARD_DSRCS = ../dev/devscsi_discard.c

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
OBJS  = $(COBJS) $(AOBJS) $(DISCARD_COBJS) $(DOBJS) $(DISCARD_DOBJS)

CROSS_CCOPTS += -I../dev/

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(OBJS) core *~  *.asm *.rst *.lst *.sym *.rel
	rm -f nasboot.lk nasboot.bin

image:
	sdasz80 -o nasboot.s
	sdldz80 -m -i nasboot.rel
	makebin -s 1024 nasboot.ihx | dd of=nasboot.bin bs=512 skip=1
	cp ../fuzix.bin fuzix.com
