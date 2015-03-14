
CSRCS = devlpr.c devtty.c devrd.c devsdspi.c ethernet.c
CSRCS += devices.c main.c

ASRCS = crt0.s devrd_hw.s socz80.s
ASRCS += tricks.s usermem.s commonmem.s

DSRCS = ../dev/devsd.c ../dev/mbr.c ../dev/blkdev.c ../dev/devsd_discard.c
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
OBJS  = $(COBJS) $(AOBJS) $(DOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	../cpm-loader/makecpmloader ../cpm-loader/cpmload.bin ../fuzix.bin 0x88 fuzix.com
