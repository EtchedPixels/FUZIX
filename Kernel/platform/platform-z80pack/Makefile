
DSRCS = ../dev/z80pack/devlpr.c ../dev/z80pack/devtty.c ../dev/z80pack/devfd.c ../dev/z80pack/devrtc.c
#NSRCS = ../dev/net/net_at.c
NSRCS = ../dev/net/net_native.c
#NSRCS = ../dev/net/net_z80pack.c
CSRCS += devices.c main.c

ASRCS = crt0.s z80pack.s
ASRCS += tricks.s commonmem.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DOBJS = $(patsubst ../dev/z80pack/%.c,%.rel, $(DSRCS))
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(NOBJS)

CROSS_CCOPTS += -I../dev/z80pack/

JUNK = *.rel *.lst *.asm *.sym *.rst

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.rel: ../dev/z80pack/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(NOBJS): %.rel: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<


clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	sdasz80 -o bootblock.s
	sdldz80 -m -i bootblock.rel
	makebin -s 128 bootblock.ihx > bootblock.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Boot floppy
	dd if=bootblock.bin of=$(IMAGES)/boot.dsk bs=256256 conv=sync
	dd if=../fuzix.bin of=$(IMAGES)/boot.dsk bs=1 seek=193024 conv=notrunc
	# Root hard disk
	(cd $(FUZIX_ROOT)/Standalone/filesystem-src; ./build-filesystem $(ENDIANFLAGH) $(IMAGES)/hd-fuzix.dsk 128 8160)
