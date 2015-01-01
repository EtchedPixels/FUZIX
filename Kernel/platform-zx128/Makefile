CSRCS = devtty.c

CSRCS2 = bankzx128.c
CSRCS2 += devices.c main.c devmdv.c

ASRCS = crt0.s zx128.s zxvideo.s microdrive.s
ASRCS += tricks.s commonmem.s

COBJS = $(CSRCS:.c=.rel)
COBJS2 = $(CSRCS2:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
OBJS  = $(COBJS) $(COBJS2) $(AOBJS)

JUNK = $(CSRCS:.c=.lst) $(CSRCS:.c=.asm) $(CSRCS:.c=.sym) $(ASRCS:.s=.lst) $(ASRCS:.s=.sym) $(CSRCS:.c=.rst) $(ASRCS:.s=.rst)

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(COBJS2): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	../tools/bintomdv ../fuzix.tmp fuzix.cart fuzixboot.mdv
