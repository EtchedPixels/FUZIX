
CSRCS = devtty.c
CSRCS += devices.c main.c libc.c

ASRCS = p68000.S crt0.S
ASRCS += tricks.S

DSRCS = ../../dev/devide.c ../../dev/mbr.c ../../dev/blkdev.c ../../dev/devide_discard.c
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))

LSRCS = ../../lib/68000exception.c ../../lib/68000usercopy.c ../../lib/68000relocate.c
LOBJS = $(patsubst ../../lib/%.c,%.o, $(LSRCS))

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=.o)
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(LOBJS)

CROSS_CCOPTS += -I../../dev/

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

$(DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(LOBJS): %.o: ../../lib/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

tricks.S: ../../lib/68000flat.S

clean:
	rm -f *.o fuzix.elf loader.elf loader.map loader.bin core *~ 

image:
	$(CROSS_LD) -M -o fuzix.elf -T fuzix.ld \
	p68000.o ../../start.o ../../version.o ../../cpu-68000/lowlevel-68000.o \
	tricks.o main.o ../../timer.o ../../kdata.o 68000exception.o devices.o \
	../../devio.o ../../filesys.o ../../process.o ../../inode.o ../../syscall_fs.o \
	../../syscall_proc.o ../../syscall_other.o ../../mm.o ../../flat.o \
	../../blk512.o ../../tty.o ../../devsys.o ../../usermem.o ../../syscall_fs2.o \
	../../syscall_fs3.o ../../syscall_exec32.o ../../syscall_exec.o \
	blkdev.o devide.o devide_discard.o mbr.o 68000relocate.o \
	68000usercopy.o ../../cpu-68000/usermem_std-68000.o devtty.o libc.o ../../malloc.o \
	../../level2.o ../../syscall_level2.o ../../select.o > ../../fuzix.map
	$(CROSS_COMPILE)objcopy fuzix.elf -O binary ../../fuzix.bin

	$(CROSS_CC) -c loader.S
	$(CROSS_LD) -M -o loader.elf -T loader.ld loader.o >loader.map
	$(CROSS_COMPILE)objcopy loader.elf -O binary loader.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync,swab
	# Add the bootstrap loader
	dd if=loader.bin of=$(IMAGES)/disk.img bs=512 seek=1 conv=notrunc,swab
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc,swab
	# Add the kernel
	dd if=../../fuzix.bin of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc,swab
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
