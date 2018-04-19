
CSRCS = devlpr.c devtty.c devfd.c devhd.c
CSRCS += devices.c main.c vt.c

DISCSRCS = discard.c

ASRCS = ubee.s crt0.s
ASRCS += tricks.s commonmem.s floppy.s

NSRCS = ../dev/net/net_native.c

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))
DISCOBJS = $(DISCSRCS:.c=.rel)

OBJS  = $(COBJS) $(AOBJS) $(NOBJS) $(DISCOBJS)

JUNK = $(CSRCS:.c=.lst) $(CSRCS:.c=.asm) $(CSRCS:.c=.sym) $(ASRCS:.s=.lst) $(ASRCS:.s=.sym) $(CSRCS:.c=.rst) $(ASRCS:.s=.rst)

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~  fuzix.ss80 bootstrap.bin

image:
	sdasz80 -o bootstrap.s
	sdldz80 -m -i bootstrap.rel
	sdasz80 -o hellotest.s
	sdldz80 -m -i hellotest.rel
	makebin -s 640 bootstrap.ihx | dd of=bootstrap.bin bs=128 skip=1
	makebin -s 640 hellotest.ihx | dd of=hellotest.bin bs=512 skip=1
	dd if=/dev/zero of=fuzix.ss80 bs=512 count=800
	dd if=bootstrap.bin of=fuzix.ss80 conv=notrunc
	dd if=../fuzix.bin of=fuzix.ss80 bs=512 seek=1 conv=notrunc
