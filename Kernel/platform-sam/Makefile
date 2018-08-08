
CSRCS = devlpr.c devtty.c devfd.c
CSRCS += devices.c main.c

DSRCS = ../dev/devide.c ../dev/mbr.c ../dev/blkdev.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../dev/devide_discard.c

ASRCS = sam.s crt0.s sam_vt.s floppy.s atom.s
ASRCS += tricks.s commonmem.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(DISCARD_COBJS) $(DISCARD_DOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:	boot setup

boot:
	sdasz80 -o boot.s
	sdldz80 -i boot.rel
	# 512 bytes at 4K
	makebin -s 16896 -p boot.ihx > boot.bin

setup:
	sdasz80 -o setup.s
	sdldz80 -n -f setup.lnk
	# 5.5K at 32K
	makebin -s 38400 -p setup.ihx > setup.bin
