
CSRCS = ../dev/devsd.c ../dev/mbr.c ../dev/blkdev.c
CSRCS += devfd.c devhd.c devlpr.c
CSRCS += devices.c main.c devtty.c ../dev/rp5c01.c devrtc.c ../dev/v99xx.c vdp.c
COMMON_CSRCS = devmegasd.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../dev/devsd_discard.c
ASRCS = msx2.s crt0.s
ASRCS += tricks.s commonmem.s bootrom.s

CROSS_CCOPTS += -I../dev/

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DOBJS = $(DSRCS:.c=.rel)
COMMON_COBJS = $(COMMON_CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(DISCARD_COBJS) $(DISCARD_DOBJS) $(COMMON_COBJS)

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) --codeseg CODE2 -c $<

$(COMMON_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) --codeseg COMMONMEM -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<
clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.lk  core *~ 

image:
	dd if=../fuzix.bin of=../fuzix.com bs=256 skip=1
	dd if=../fuzix_boot.bin of=../fuzix.ascii8.rom skip=1 bs=16384 conv=sync
	dd if=../fuzix.bin of=../fuzix.ascii8.rom seek=1 bs=16384 conv=sync
