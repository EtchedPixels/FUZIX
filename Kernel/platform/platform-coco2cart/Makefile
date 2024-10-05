
CSRCS = devices.c main.c devinput.c

# Code that must be present for the bootstrap
CISRCS = devsdc.c

C3SRCS = libc.c devtty.c

CDSRCS = discard.c 

DSRCS = ../../dev/devdw.c ../../dev/tinydisk.c ../../dev/tinyide.c
DDSRCS = ../../dev/tinydisk_discard.c ../../dev/tinyide_discard.c

ASRCS = crt0.s coco2.s ide.s usermem.s sdc.s
ASRCS += bootstrap.s bootstrap_sdc.s #bootstrap_dw.s
ASRCS += tricks.s commonmem.s ../../dev/drivewire/drivewire.s video.s

COBJS = $(CSRCS:.c=$(BINEXT))
CIOBJS = $(CISRCS:.c=$(BINEXT))
C3OBJS = $(C3SRCS:.c=$(BINEXT))
CDOBJS = $(CDSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DDOBJS = $(patsubst ../../dev/%.c,%.o, $(DDSRCS))
OBJS  = $(COBJS) $(C3OBJS) $(CDOBJS) $(AOBJS) $(DOBJS) $(DDOBJS) $(CIOBJS)

CROSS_CCOPTS += -I../../dev/

all: $(OBJS)

export CROSS_CC_SEGD=-mcode-section=.text3

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(CIOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(C3OBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGD) -c $<

$(CDOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %$(BINEXT): ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGD) -c $<

$(DDOBJS): %$(BINEXT): ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

clean:
	rm -f *.o *~ fuzixide.rom fuzixsdc.rom fuxix.img

image:
	$(CROSS_LD) -o ../../fuzix.bin -Map=../../fuzix.map --script=fuzix.link \
	--oformat=decb \
	crt0.o bootstrap.o commonmem.o coco2.o discard.o ../../simple.o \
	../../start.o ../../version.o ../../cpu-6809/lowlevel-6809.o \
	tricks.o main.o ../../timer.o ../../kdata.o devices.o \
	../../dev/drivewire/drivewire.o devdw.o tinyide.o tinyide_discard.o ide.o \
	tinydisk.o tinydisk_discard.o ../../memalloc_none.o \
	../../devio.o ../../filesys.o ../../process.o ../../inode.o ../../syscall_fs.o \
	../../syscall_proc.o ../../syscall_other.o ../../syscall_exec.o ../../mm.o ../../swap.o \
	../../tty.o ../../devsys.o ../../usermem.o usermem.o ../../syscall_fs2.o \
	../../syscall_exec16.o devtty.o libc.o ../../vt.o video.o  ../../syscall_fs3.o \
	../../blk512.o ../../font8x8.o devsdc.o sdc.o ../../devinput.o devinput.o
	../../tools/lw-checkmap ../../fuzix.map
	../../tools/decb-image <../../fuzix.bin fuzix.img
	# This has changed with the discard change
	# We now have C000-FFFF ROM
	# 0400-7FFF single RAM block
	# This is much simpler and storage size simply isn't an issue
	dd if=fuzix.img of=fuzixide.rom bs=1024 skip=48 count=16
	dd if=fuzix.img of=fuzix.ide bs=1024 skip=1 count=31

	$(CROSS_LD) -o ../../fuzix-sdc.bin -Map=../../fuzix-sdc.map --script=fuzix.link \
	--oformat=decb \
	crt0.o bootstrap_sdc.o commonmem.o coco2.o discard.o ../../simple.o \
	../../start.o ../../version.o ../../cpu-6809/lowlevel-6809.o \
	tricks.o main.o ../../timer.o ../../kdata.o devices.o \
	../../dev/drivewire/drivewire.o devdw.o tinyide.o tinyide_discard.o ide.o \
	tinydisk.o tinydisk_discard.o ../../memalloc_none.o \
	../../devio.o ../../filesys.o ../../process.o ../../inode.o ../../syscall_fs.o \
	../../syscall_proc.o ../../syscall_other.o ../../syscall_exec.o ../../mm.o ../../swap.o \
	../../tty.o ../../devsys.o ../../usermem.o usermem.o ../../syscall_fs2.o \
	../../syscall_exec16.o devtty.o libc.o ../../vt.o video.o  ../../syscall_fs3.o \
	../../blk512.o ../../font8x8.o devsdc.o sdc.o ../../devinput.o devinput.o
	../../tools/decb-image <../../fuzix-sdc.bin fuzix-sdc.img
	dd if=fuzix-sdc.img of=fuzixsdc.rom bs=1024 skip=48 count=16
	dd if=fuzix-sdc.img of=fuzix.sdc bs=1024 skip=1 count=31


IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# We have 2048 blocks at the start which are the partition table then our
	# image blocks
	cp $(IMAGES)/disk.img $(IMAGES)/sd.img
	# Write the rest of the images to the disks
	dd if=fuzix.ide of=$(IMAGES)/disk.img bs=512 seek=1 conv=notrunc
	dd if=fuzix.sdc of=$(IMAGES)/sd.img bs=512 seek=1 conv=notrunc
	# Cartridge ROM
	cp fuzixide.rom $(IMAGES)/fuzixide.rom
	cp fuzixsdc.rom $(IMAGES)/fuzixsdc.rom
