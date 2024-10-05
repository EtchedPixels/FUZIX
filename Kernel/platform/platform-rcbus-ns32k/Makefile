
CSRCS = devtty.c
CSRCS += devices.c main.c libc.c

ASRCS = ns32k.S crt0.S
ASRCS += tricks.S

DSRCS =  ../../dev/blkdev.c ../../dev/mbr.c ../../dev/devide.c ../../dev/devide_discard.c
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))

LSRCS = ../../lib/ns32krelocate.c
LOBJS = $(patsubst ../../lib/%.c,%.o, $(LSRCS))

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=.o)
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(LOBJS)

CROSS_CCOPTS += -I../../dev/

JUNK = $(CSRCS:.c=.o) $(ASRCS:.S=.o)

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

$(DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(LOBJS): %.o: ../../lib/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f *.o $(JUNK)  core *~  fuzix.aout

image:
	$(CROSS_LD) -M -o fuzix.aout -N -T fuzix.ld \
	crt0.o \
	ns32k.o ../../start.o ../../version.o ../../cpu-ns32k/lowlevel-ns32k.o \
	tricks.o ../../simple.o main.o ../../timer.o ../../kdata.o devices.o \
	../../devio.o ../../filesys.o ../../process.o ../../inode.o ../../syscall_fs.o \
	../../syscall_proc.o ../../syscall_other.o ../../mm.o ../../blk512.o \
	../../tty.o ../../devsys.o ../../usermem.o ../../syscall_fs2.o \
	../../syscall_fs3.o ../../syscall_exec32.o blkdev.o mbr.o \
	devide.o devide_discard.o ../../flat.o ../../syscall_exec.o \
	ns32krelocate.o ../../cpu-ns32k/usermem_std-ns32k.o devtty.o \
	libc.o ../../malloc.o > ../../fuzix.map
	ns32k-pc532-netbsd-objcopy fuzix.aout -O binary ../../fuzix.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=../../fuzix.bin of=$(IMAGES)/disk.img bs=512 seek=1 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
