
CSRCS = devlpr.c devtty.c devrd.c devaudio.c devgfx.c
CSRCS += devices.c main.c

ASRCS = nc100.s crt0.s
ASRCS += tricks.s commonmem.s

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
	dd if=bootblock.cim of=../fuzix.bin bs=1 seek=256 conv=notrunc
