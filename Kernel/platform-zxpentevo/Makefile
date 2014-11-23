# CODE 1
CSRCS = devices.c main.c devtty.c 
ASRCS = crt0.s zxpentevo.s zxvideo.s
ASRCS += tricks.s commonmem.s devsd_hw.s

# CODE 2
CSRCS2 = devsd.c

COBJS = $(CSRCS:.c=.rel)
COBJS2 = $(CSRCS2:.c=.rel)

AOBJS = $(ASRCS:.s=.rel)

OBJS  = $(COBJS) $(AOBJS) $(COBJS2)

JUNK = $(CSRCS:.c=.lst) $(CSRCS:.c=.asm) $(CSRCS:.c=.sym) $(CSRCS:.c=.rst)
JUNK += $(CSRCS2:.c=.lst) $(CSRCS2:.c=.asm) $(CSRCS2:.c=.sym) $(CSRCS2:.c=.rst)
JUNK += $(ASRCS:.s=.lst) $(ASRCS:.s=.sym) $(ASRCS:.s=.rst)

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS2): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<


clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
