ASRCS = crt0.s tricks.s commonmem.s zeta-v2.s monitor.s
ASRCS += ds1302-n8vem.s devrd_hw.s
CSRCS = devices.c main.c devtty.c devrd.c ppide.c
DISCARD_CSRCS = discard.c devtty_discard.c
DISCARD_DSRCS = ../dev/ds1302_discard.c ../dev/devide_discard.c
DSRCS = ../dev/devfd.c ../dev/devsd.c ../dev/mbr.c ../dev/blkdev.c
DSRCS += ../dev/ds1302.c ../dev/devide.c
DASRCS = ../dev/devfd_hw.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DAOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS)

CROSS_CCOPTS += -I../dev/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS)

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
	rm -f $(OBJS) $(JUNK)  core *~ bootrom.ihx bootrom.bin

image:
	sdasz80 -o bootrom.s
	sdldz80 -m -i bootrom.rel
	makebin -s 256 bootrom.ihx > bootrom.bin
	cat bootrom.bin ../fuzix.bin > fuzix.rom
	../cpm-loader/makecpmloader ../cpm-loader/cpmload.bin ../fuzix.bin 0x88 fuzix.com
