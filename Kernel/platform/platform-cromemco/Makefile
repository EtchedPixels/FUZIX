
CSRCS = devtty.c devfd.c interrupt.c devices.c main.c
CDSRCS = discard.c

ASRCS = crt0.s cromemco.s usermem.s tricks.s commonmem.s vector.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
CDOBJS = $(CDSRCS:.c=.rel)

OBJS  = $(AOBJS) $(COBJS) $(CDOBJS)

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(CDOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

vector.rel: tuart.s

clean:
	rm -f *.rel *.lst *.asm *.sym *.rst core *~
	rm -f cloader-16fdc.ihx cloader-16fdc.tmp
	rm -f cloader-16fdc.lk cloader-16fdc.bin
	rm -f cloader-16fdc.map

image:
	sdasz80 -o cloader-16fdc.s
	sdldz80 -m -i cloader-16fdc.rel
	makebin -s 256 cloader-16fdc.ihx > cloader-16fdc.tmp
	dd if=cloader-16fdc.tmp of=cloader-16fdc.bin bs=1 skip=128 count=128
	# Assemble a standard IBM format SSSD 8" disk image
	cat cloader-16fdc.bin ../../fuzix.bin > fuzix.boot

diskimage: image
	(cd $(FUZIX_ROOT)/Standalone/filesystem-src; ./build-mini-filesystem $(ENDIANFLAG) $(FUZIX_ROOT)/Images/$(TARGET)/root.dsk 96 2464)
	dd if=fuzix.boot of=$(FUZIX_ROOT)/Images/$(TARGET)/fuzix.dsk bs=256256 count=1 conv=sync
