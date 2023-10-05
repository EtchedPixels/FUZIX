
CSRCS = ttydw.c dwtime.c devsys.c
CSRCS += devices.c main.c libc.c devsdc.c devlpr.c devrtsd.c mbr.c

CDSRCS = ../../dev/tinyide_discard.c ../../dev/tinysd_discard.c ../../dev/tinydisk_discard.c

DSRCS = ../../dev/devdw.c ../../dev/tinydisk.c ../../dev/tinyide.c ../../dev/tinysd.c

NSRCS = ../../dev/net/net_native.c

ASRCS = coco3.s crt0.s ide.s sd.s rtsd.s
ASRCS += tricks.s commonmem.s usermem_gime.s ../../dev/drivewire/drivewire.s sdc.s videoll.s

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
NOBJS = $(patsubst ../../dev/net/%.c,%.o, $(NSRCS))
CDOBJS = $(CDSRCS:.c=$(BINEXT))
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(CDOBJS) $(NOBJS)

CROSS_CCOPTS += -I../../dev/

HIGH = -mcode-section=.text3

JUNK = $(CSRCS:.c=.o) $(ASRCS:.s=.o) $(DSRCS:.c=.o)

# Set 'SUBTARGET' to one of: emu, real, fpga, nano. 

ifndef SUBTARGET
SUBTARGET = emu
endif

ifeq ($(SUBTARGET),real)
COCO_IDE = 1
COCO_SDC = 1
endif

ifeq ($(SUBTARGET),emu)
COCO_IDE = 1
COCO_BECKER = 1
endif

ifeq ($(SUBTARGET),fpga)
COCO_SDFPGA = 1
COCO_BECKER = 1
endif

ifeq ($(SUBTARGET),nano)
COCO_SDNANO = 1
endif

ifdef COCO_SDC
DRIVERS += devsdc.o sdc.o
CROSS_CC += -DCONFIG_COCOSDC
endif

ifdef COCO_IDE
DRIVERS += tinyide.o tinyide_discard.o ide.o
CROSS_CC += -DCONFIG_COCOIDE
endif

ifdef COCO_SDFPGA
DRIVERS += tinysd.o tinysd_discard.o sd.o
CROSS_CC += -DCONFIG_COCOSDFPGA
endif

ifdef COCO_SDNANO
DRIVERS += devrtsd.o rtsd.o
CROSS_CC += -DCONFIG_COCOSDNANO
endif

ifdef COCO_BECKER
ASOPTS = --defsym BECKER=1
else
ASOPTS = --defsym BECKER=0 
endif

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(CDOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %$(BINEXT): ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(HIGH) -c $<

$(NOBJS): %$(BINEXT): ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*.o


clean:
	rm -f $(OBJS) $(JUNK) fuzix.dsk boot.bin core *~
	rm -f fuzix.tmp

image: boot.bin
	$(CROSS_CC) $(CROSS_CCOPTS) -O0 -c -o ../../bank16k.o ../../mm/bank16k.c
	$(CROSS_CC) $(CROSS_CCOPTS) -O0 -c -o devtty.o devtty.c
	$(CROSS_CC) $(CROSS_CCOPTS) -O0 -c -o ../../timer.o ../../timer.c
	$(CROSS_CC) $(CROSS_CCOPTS) -O0 -c -o net_native.o ../../dev/net/net_native.c
	$(CROSS_CC) $(CROSS_CC_VIDEO) $(CROSS_CCOPTS) -O0 -c -o video.o video.c
	$(CROSS_CC) $(CROSS_CC_VIDEO) $(CROSS_CCOPTS) -O0 -c -o ../../usermem.o ../../usermem.c
	$(CROSS_LD) -o ../../fuzix.bin -Map=../../fuzix.map --script=fuzix.link --oformat=decb \
	crt0.o commonmem.o \
	coco3.o ../../start.o ../../version.o ../../cpu-6809/lowlevel-6809.o \
	tricks.o main.o ../../timer.o ../../kdata.o devices.o \
	../../dev/drivewire/drivewire.o devdw.o ttydw.o tinydisk.o tinydisk_discard.o devlpr.o \
	../../devio.o ../../filesys.o ../../process.o ../../inode.o ../../syscall_fs.o \
	../../syscall_proc.o ../../syscall_other.o ../../mm.o ../../bank16k.o ../../swap.o \
	../../tty.o ../../devsys.o ../../usermem.o ../../syscall_fs2.o ../../syscall_exec16.o \
	../../syscall_fs3.o ../../syscall_exec.o ../../blk512.o ../../memalloc_none.o \
	../../cpu-6809/usermem_std-6809.o devtty.o libc.o ../../vt.o usermem_gime.o video.o \
	videoll.o dwtime.o devsys.o mbr.o ../../network.o \
	../../syscall_net.o net_native.o $(DRIVERS)
# make DECB disk w/ new kernel
	rm -f fuzix.dsk
	decb dskini fuzix.dsk
	decb copy -2 -b ../../fuzix.bin ./fuzix.dsk,FUZIX.BIN
	decb copy -2 -b boot.bin ./fuzix.dsk,BOOT.BIN
	decb copy -l -0 -a AUTOEXEC.BAS ./fuzix.dsk,AUTOEXEC.BAS

boot.bin: boot/boot.s
	lwasm -lboot.list -oboot.bin boot/boot.s

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the floppy image (assumes the usual binary hdbdos image and offset)
	$(FUZIX_ROOT)/Kernel/tools/diskpad <fuzix.dsk >fuzix.tmp
	dd if=fuzix.tmp of=$(IMAGES)/disk.img bs=512 seek=339420 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
