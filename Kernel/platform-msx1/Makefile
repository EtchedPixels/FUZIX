
CSRCS = devtty.c devfd.c devhd.c devlpr.c
CSRCS += devices.c main.c

ASRCS = msx1.s crt0.s vdp.s
ASRCS += tricks.s commonmem.s bootrom.s

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
OBJS  = $(COBJS) $(AOBJS)

JUNK = $(CSRCS:.c=.lst) $(CSRCS:.c=.asm) $(CSRCS:.c=.sym) $(ASRCS:.s=.lst) $(ASRCS:.s=.sym) $(CSRCS:.c=.rst) $(ASRCS:.s=.rst)

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	dd if=../fuzix.bin of=../fuzix.com bs=256 skip=1
	dd if=../fuzix.bin of=../fuzix.cart bs=16384 conv=sync
