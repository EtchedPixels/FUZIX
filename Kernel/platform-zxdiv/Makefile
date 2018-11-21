CSRCS = devtty.c devices.c main.c bank128.c divide.c divmmc.c
CDSRCS = discard.c
DSRCS = ../dev/devide.c ../dev/devsd.c ../dev/blkdev.c
DDSRCS = ../dev/devide_discard.c ../dev/devsd_discard.c ../dev/mbr.c
ASRCS = crt0.s zx128.s zxvideo.s
ASRCS += tricks.s commonmem.s loader-divide.s

COBJS = $(CSRCS:.c=.rel)
CDOBJS = $(CDSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DDOBJS = $(patsubst ../dev/%.c,%.rel, $(DDSRCS))
OBJS  = $(COBJS) $(CDOBJS) $(AOBJS) $(DOBJS) $(DDOBJS)

CROSS_CCOPTS += -I../dev/

CROSS_CC_SEG3 = --codeseg CODE3

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) -c $<

$(CDOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) -c $<

$(DDOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.rel core *~ 

# Re-order the image and snapshop it
image:
	# The bootstrap to con Fatware
	dd if=../common.bin of=BOOT.BIN bs=8192 count=1
	# Rest of the base image, starts at 0x2200, pad it
	dd if=../common.bin bs=65536 count=1 conv=sync | \
		dd of=strap.bin bs=512 skip=17
	# Banked segments 1 and 7
	dd if=../bank2.bin bs=16384 skip=3 >>strap.bin conv=sync
	dd if=../bank3.bin bs=16384 skip=3 >>strap.bin conv=sync
