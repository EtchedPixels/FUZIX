
CSRCS = ../dev/devsd.c ../dev/mbr.c ../dev/blkdev.c devfd.c devhd.c devlpr.c
CSRCS += devices.c main.c devtty.c
DSRCS = devmegasd.c
ASRCS = msx2.s crt0.s vdp.s
ASRCS += tricks.s commonmem.s bootrom.s

CROSS_CCOPTS += -I../dev/

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DOBJS = $(DSRCS:.c=.rel)
OBJS  = $(COBJS) $(AOBJS) $(DOBJS)

JUNK = $(CSRCS:.c=.lst) $(CSRCS:.c=.asm) $(CSRCS:.c=.sym) $(ASRCS:.s=.lst) $(ASRCS:.s=.sym) $(CSRCS:.c=.rst) $(ASRCS:.s=.rst)

all:	$(OBJS)

$(DOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) --codeseg CODE2 -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	dd if=../fuzix.bin of=../fuzix.com bs=256 skip=1
	dd if=../fuzix.bin of=../fuzix.rom bs=16384 conv=sync
