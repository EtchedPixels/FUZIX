
CSRCS = devtty.c
CSRCS += devices.c main.c

ASRCS = rcbus-68hc11.S crt0.S
ASRCS += tricks.S commonmem.S

CDSRCS = discard.c

DSRCS = ../../dev/devide.c ../../dev/blkdev.c ../../dev/devsd.c ../../dev/ds1302.c
DASRCS += ../../dev/ds1302_68hc11.S
DISCARD_DSRCS = ../../dev/mbr.c ../../dev/devide_discard.c ../../dev/devsd_discard.c
DISCARD_DSRCS += ../../dev/ds1302_discard.c

A68SRCS = ../../dev/68hc11/sci.S

COBJS = $(CSRCS:.c=$(BINEXT))
CDOBJS = $(CDSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=$(BINEXT))
A68OBJS = $(patsubst ../../dev/68hc11/%.S,%.o, $(A68SRCS))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.S,%.o, $(DASRCS))
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))

OBJS  = $(COBJS) $(CDOBJS) $(AOBJS) $(A68OBJS) $(DOBJS) $(DAOBJS) $(DISCARD_DOBJS)

CROSS_CCOPTS += -I../../dev/ -I../../dev/68hc11/

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) $<

$(CDOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) $<

$(AOBJS): %$(BINEXT): %.S
	cpp -E $<  >$*.s
	$(CROSS_AS) $(ASOPTS) $*.s -o $*.o

$(A68OBJS): %$(BINEXT): ../../dev/68hc11/%.S
	cpp -I. -E $<  >$*.s
	$(CROSS_AS) $(ASOPTS) $*.s -o $*.o

$(DOBJS): %$(BINEXT): ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(DAOBJS): %$(BINEXT): ../../dev/%.S
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(DISCARD_DOBJS): %$(BINEXT): ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(DOBJS) $(DAOBJS) $(OBJS) *.o *.s core *~ fuzix.bin fuzix.tmpmap

image:
	$(CROSS_LD) -M -T memory.x -relax -o ../../fuzix.elf crt0.o commonmem.o \
	rcbus-68hc11.o ../../start.o ../../version.o ../../cpu-68hc11/lowlevel-68hc11.o \
	tricks.o main.o discard.o ../../timer.o ../../kdata.o devices.o \
	blkdev.o mbr.o devide.o devide_discard.o devsd.o \
	devsd_discard.o sci.o ds1302_discard.o ds1302.o ds1302_68hc11.o \
	../../devio.o ../../filesys.o ../../process.o ../../inode.o ../../syscall_fs.o \
	../../syscall_proc.o ../../syscall_other.o ../../mm.o ../../swap.o ../../bank16k.o \
	../../tty.o ../../devsys.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_exec.o ../../syscall_exec16.o ../../blk512.o ../../memalloc_none.o \
	../../usermem.o ../../cpu-68hc11/usermem_std-68hc11.o devtty.o >../../fuzix.map
	$(CROSS_OBJCOPY) ../../fuzix.elf -O binary ../../fuzix.bin
	dd if=../../fuzix.bin bs=1 skip=61440 count=3072 of=fuzix.common
	dd if=fuzix.common of=../../fuzix.bin bs=1 seek=53760 conv=notrunc

loadsd:
	$(CROSS_CC) -c loadsd.S
	$(CROSS_LD) -M -T loadsd.x -relax -o loadsd.elf loadsd.o
	$(CROSS_OBJCOPY) loadsd.elf -O binary loadsd >loadsd.map

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: loadsd
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/sd.img bs=40017920 conv=sync
	# Add the bootstrap loader
	dd if=loadsd of=$(IMAGES)/sd.img conv=notrunc
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/sd.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=../../fuzix.bin of=$(IMAGES)/sd.img bs=512 seek=1 conv=notrunc
