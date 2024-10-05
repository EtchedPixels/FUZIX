
CSRCS = devtty.c
CSRCS += devices.c main.c

ASRCS = rcbus-65c816.s crt0.s
ASRCS += tricks.s commonmem.s

DSRCS = ../../dev/devide.c ../../dev/blkdev.c
DISCARD_DSRCS = ../../dev/mbr.c ../../dev/devide_discard.c

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))

OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(DISCARD_DOBJS)

CROSS_CCOPTS += -I../../dev/

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*$(BINEXT)

$(DOBJS): %$(BINEXT): ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<
	mv ../../dev/$*.o .

$(DISCARD_DOBJS): %$(BINEXT): ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<
	mv ../../dev/$*.o .

clean:
	rm -f $(OBJS) *.o core *~ fuzix.img fuzix.bin

image:
	$(CROSS_LD) -b -C 0x200 -S 0xF000 -X 0xD000 -o ../../fuzix.bin -m ../../fuzix.map \
	crt0.o commonmem.o rcbus-65c816.o ../../start.o ../../version.o \
	../../cpu-65c816/lowlevel-65c816.o tricks.o main.o ../../timer.o \
	../../kdata.o devices.o blkdev.o mbr.o devide.o devide_discard.o \
	../../mm/memalloc_none.o ../../devio.o ../../filesys.o ../../process.o \
	../../inode.o ../../syscall_fs.o ../../syscall_proc.o \
	../../syscall_other.o ../../mm.o ../../mm/bankfixed.o \
	../../tty.o ../../devsys.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_exec16.o ../../syscall_exec.o ../../blk512.o \
	../../usermem.o ../../cpu-65c816/usermem_std-65c816.o devtty.o \
	/opt/cc65c816/lib/lib65c816.a
	dd if=../../fuzix.bin of=fuzix.img bs=256 skip=4 

loader: loader.o
	# Turn the loader into just the bytes for the bootstrap
	ld65 -C loader.cfg loader.o -o loader.tmp
	dd if=loader.tmp bs=256 skip=2 | dd of=loader.bin bs=350 count=1

loader.o: loader.s
	ca65 loader.s
	
IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image loader
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
