ifneq ($(MOOH),)
ASOPTS += --defsym MOOH=1
CROSS_CCOPTS += -DMOOH=1
MEMSRC = mem-nx32-mooh.s
else
MEMSRC = mem-nx32.s
endif
MEMOBJ = $(MEMSRC:.s=$(BINEXT))

CSRCS = devlpr.c devtty.c devfd.c ttydw.c devinput.c
CSRCS += vc.c devices.c main.c libc.c

CDSRCS = discard.c

DSRCS = ../dev/devdw.c ../dev/blkdev.c ../dev/devide.c \
        ../dev/crt9128.c \
        ../dev/devsd.c ../dev/devscsi.c

DDSRCS = ../dev/devide_discard.c ../dev/devscsi_discard.c ../dev/mbr.c \
         ../dev/gpt.c ../dev/devsd_discard.c

ASRCS = crt0.s dragon.s $(MEMSRC) video.s ide.s spi.s scsi_tc3.s
ASRCS += tricks.s commonmem.s usermem_sam.s floppy.s ../dev/drivewire/drivewire.s
ASRCS += multihead.s vc_asm.s

COBJS = $(CSRCS:.c=$(BINEXT))
CDOBJS = $(CDSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))
DDOBJS = $(patsubst ../dev/%.c,%.o, $(DDSRCS))
OBJS  = $(COBJS) $(CDOBJS) $(AOBJS) $(DOBJS) $(DDOBJS)

CROSS_CCOPTS += -I../dev/

../dev/drivewire/drivewire.o: ASOPTS += --defsym BECKER=$(BECKER)

JUNK = $(CSRCS:.c=.o) $(ASRCS:.s=.o)

all: $(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(CDOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %$(BINEXT): ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(DDOBJS): %$(BINEXT): ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

clean:
	rm -f $(OBJS) $(JUNK)

dwfuz.bin: dwload.asm
	lwasm --pragma=condundefzero -r -b -odwfuz.bin dwload.asm -DFUZIX -DRAMORG=0x600 -DBECKER

image: dwfuz.bin
	$(CROSS_LD) -o ../fuzix.bin -Map=../fuzix.map --script=fuzix.link --oformat=decb \
	crt0.o commonmem.o usermem_sam.o \
	dragon.o $(MEMOBJ) ../bankfixed.o \
	../start.o ../version.o ../lowlevel-6809.o \
	tricks.o main.o ../timer.o ../kdata.o devfd.o floppy.o devices.o \
	multihead.o crt9128.o vc.o vc_asm.o \
	../dev/drivewire/drivewire.o devdw.o ttydw.o ../memalloc_none.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o ../syscall_exec16.o \
	../syscall_fs3.o ../syscall_exec.o ../blk512.o ../devinput.o \
	devinput.o devlpr.o devtty.o libc.o ../vt.o video.o ../font8x8.o \
	devide.o blkdev.o ide.o devide_discard.o mbr.o gpt.o \
	devsd.o devsd_discard.o spi.o devscsi.o devscsi_discard.o \
	scsi_tc3.o discard.o
	../tools/pad256 ../fuzix.bin
	../tools/lw-checkmap ../fuzix.map

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Write the rest of the images to the disks
	dd if=../fuzix.bin of=$(IMAGES)/disk.img bs=512 seek=64 conv=notrunc
	cp dwfuz.bin $(IMAGES)
