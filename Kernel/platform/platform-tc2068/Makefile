CSRCS = devtty.c devices.c main.c devinput.c
CDSRCS = discard.c
DSRCS = ../dev/devide.c ../dev/blkdev.c
DDSRCS = ../dev/devide_discard.c ../dev/mbr.c
DZSRCS = ../dev/zx/divide.c ../dev/zx/zxkeyboard.c
DDZSRCS =
ASRCS = crt0.s tc2068.s tmxvideo.s
ASRCS += tricks.s commonmem.s loader-divide.s

COBJS = $(CSRCS:.c=.rel)
CDOBJS = $(CDSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DDOBJS = $(patsubst ../dev/%.c,%.rel, $(DDSRCS))
DZOBJS = $(patsubst ../dev/zx/%.c,%.rel, $(DZSRCS))
DDZOBJS = $(patsubst ../dev/zx/%.c,%.rel, $(DDZSRCS))
OBJS  = $(COBJS) $(CDOBJS) $(AOBJS) $(DOBJS) $(DDOBJS) $(DZOBJS) $(DDZOBJS)

CROSS_CCOPTS += -I../dev/ -I../dev/zx/

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

$(DZOBJS): %.rel: ../dev/zx/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) -c $<

$(DDZOBJS): %.rel: ../dev/zx/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.rel core *~ 
	rm -f fuzix.dck fuzix.load

image:
	../tools/makedck ../fuzix.bin fuzix.dck
	dd if=../fuzix.bin of=fuzix.load bs=32768 count=1

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	cp fuzix.dck $(IMAGES)/
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# And bootstrap
	dd if=fuzix.load of=$(IMAGES)/disk.img bs=512 seek=1 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/hdfheader $(IMAGES)/disk.img > $(IMAGES)/emu-ide.hdf
