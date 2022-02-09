
CSRCS += devices.c main.c devfd.c devinput.c devlpr.c devtty.c

ASRCS = crt0.s cpm.s lowlevel.s
ASRCS += tricks.s commonmem.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(NOBJS)

JUNK = *.rel *.lst *.asm *.sym *.rst

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	cp ../fuzix.bin fuzix56.com
	(cd ..; $(CROSS_LD) -n -k $(LIBZ80) -f platform-$(TARGET)/fuzix48.lnk)
	(cd ..; tools/bihx fuzix.ihx; tools/binprep)
	cp ../fuzix.bin fuzix48.com

