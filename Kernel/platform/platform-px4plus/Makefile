
CSRCS = devlpr.c devtty.c devfd.c sio.c
CSRCS += devices.c

CCSRCS = main.c

CDSRCS = init.c

ASRCS = crt0.s px4plus.s video.s sio-asm.s
ASRCS += tricks.s commonmem.s swapdev.s romdisc.s

COBJS = $(CSRCS:.c=.rel)
CCOBJS = $(CCSRCS:.c=.rel)
CDOBJS = $(CDSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
OBJS  = $(COBJS) $(CCOBJS) $(CDOBJS) $(AOBJS)

JUNK = $(CSRCS:.c=.lst) $(CSRCS:.c=.asm) $(CSRCS:.c=.sym) $(ASRCS:.s=.lst) $(ASRCS:.s=.sym) $(CSRCS:.c=.rst) $(ASRCS:.s=.rst)

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(CCOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c --codeseg COMMONMEM $<

$(CDOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c --codeseg DISCARD $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
