CSRCS += devices.c main.c devtty.c devsdspi.c
DSRCS = ../dev/devide.c ../dev/devsd.c ../dev/mbr.c ../dev/blkdev.c ../dev/ds1302.c
DDSRCS = ../dev/devide_discard.c ../dev/devsd_discard.c ../dev/ds1302_discard.c
ASRCS = crt0.s z180.s commonmem.s mark4.s ds1302-mark4.s monitor.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DDOBJS = $(patsubst ../dev/%.c,%.rel, $(DDSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DDOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS) diskboot.bin

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DDOBJS): %.rel: ../dev/%.c
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
