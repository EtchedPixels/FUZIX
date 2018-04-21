
CSRCS = devtty.c devfd.c
CSRCS += devices.c main.c

ASRCS = crt0.s cromemco.s
ASRCS += tricks.s commonmem.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)

OBJS  = $(AOBJS) $(COBJS)

JUNK = *.rel *.lst *.asm *.sym *.rst

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	sdasz80 -o cloader-16fdc.s
	sdldz80 -m -i cloader-16fdc.rel
	makebin -s 256 cloader-16fdc.ihx > cloader-16fdc.tmp
	dd if=cloader-16fdc.tmp of=cloader-16fdc.bin bs=1 skip=128

