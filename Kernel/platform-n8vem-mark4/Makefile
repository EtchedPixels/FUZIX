ASRCS = crt0.s z180.s commonmem.s mark4.s ds1302-mark4.s monitor.s
CSRCS += devices.c main.c devtty.c devsdspi.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../dev/devide_discard.c ../dev/devsd_discard.c ../dev/ds1302_discard.c
DSRCS = ../dev/devide.c ../dev/devsd.c ../dev/mbr.c ../dev/blkdev.c ../dev/ds1302.c ../dev/devrd_z180.c
DASRCS = ../dev/devrd_z180_hw.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(DAOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS) diskboot.bin

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DAOBJS): %.rel: ../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ fuzix.com makecpmloader diskboot.bin

z180.rel:		z180.s kernel.def ../cpu-z180/z180.s

diskboot.bin:	diskboot.s
	$(CROSS_AS) $(ASOPTS) diskboot.s
	sdldz80 -nmi diskboot.rel
	makebin -s 65536 diskboot.ihx | dd bs=512 skip=125 count=1 of=diskboot.bin

image:
	../cpm-loader/makecpmloader ../cpm-loader/cpmload.bin ../fuzix.bin 0x88 fuzix.com
	../cpm-loader/makecpmloader ../cpm-loader/fuzixload.bin ../fuzix.bin 0x88 fuzix
