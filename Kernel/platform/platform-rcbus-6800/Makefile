
CSRCS = devtty.c
CSRCS += devices.c main.c

ASRCS = rcbus-6800.S crt0.S
ASRCS += tricks.S commonmem.S

CDSRCS = discard.c

DSRCS = ../../dev/devide.c ../../dev/blkdev.c
DISCARD_DSRCS = ../../dev/mbr.c ../../dev/devide_discard.c

COBJS = $(CSRCS:.c=$(BINEXT))
CDOBJS = $(CDSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=$(BINEXT))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))

OBJS  = $(COBJS) $(CDOBJS) $(AOBJS) $(DOBJS) $(DISCARD_DOBJS)

CROSS_CCOPTS += -I../../dev/

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) $<

$(CDOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) $<

$(AOBJS): %$(BINEXT): %.S
	cpp -E $<  >$*.s
	$(CROSS_AS) $(ASOPTS) $*.s

$(DOBJS): %$(BINEXT): ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<
	mv ../../dev/$*.o .

$(DISCARD_DOBJS): %$(BINEXT): ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<
	mv ../../dev/$*.o .

clean:
	rm -f $(OBJS) *.o core *~ fuzix.bin fuzix.tmpmap

image: loader
	$(CROSS_LD) -b -C 0x100 -S 0xF200 -Z 0x20 -o ../../fuzix.bin -m fuzix.tmpmap crt0.o commonmem.o \
	rcbus-6800.o ../../start.o ../../version.o ../../cpu-6800/lowlevel-6800.o \
	tricks.o main.o discard.o ../../timer.o ../../kdata.o devices.o \
	blkdev.o mbr.o devide.o devide_discard.o ../../mm/memalloc_none.o \
	../../devio.o ../../filesys.o ../../process.o ../../inode.o ../../syscall_fs.o \
	../../syscall_proc.o ../../syscall_other.o ../../mm.o ../../swap.o ../../mm/bank16kfc.o \
	../../tty.o ../../devsys.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_exec16.o ../../syscall_exec.o ../../blk512.o \
	../../usermem.o ../../cpu-6800/usermem_std-6800.o devtty.o /opt/fcc/lib/6800/lib6800.a
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map
	dd if=../../fuzix.bin of=fuzix.img bs=256 skip=1

loader.o: loader.S
	cpp -E $< >$*.s
	$(CROSS_AS) $(ASOPTS) $*.s

loader: loader.o
	$(CROSS_LD) -b -C 0x00 -Z 0xF0 -o loader.bin -m loader.map loader.o

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: loader
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the bootstrap loader
	dd if=loader.bin of=$(IMAGES)/disk.img conv=notrunc
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=fuzix.img of=$(IMAGES)/disk.img bs=512 seek=1 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
