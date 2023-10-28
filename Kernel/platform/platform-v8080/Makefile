# FIXME: paths
LIBC = /opt/cc85/lib/lib8080.a

DSRCS = ../../dev/z80pack/devfd.c

CSRCS += devices.c main.c devtty.c

ASRCS = crt0.S commonmem.S v8080.S tricks.S

AOBJS = $(ASRCS:.S=.o)
COBJS = $(CSRCS:.c=.o)
DOBJS = $(patsubst ../../dev/z80pack/%.c,%.o, $(DSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS)

CROSS_CCOPTS += -I../../dev/z80pack/

JUNK = *.o

all:	$(OBJS)

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $<

$(DOBJS): %.o: ../../dev/z80pack/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

tricks.s: ../../lib/8080fixedbank.s ../../lib/8080fixedbank-core.s

clean:
	rm -f $(OBJS) $(JUNK)  core *~ bootblock.bin bootblock

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

image:
	$(CROSS_LD) -b -C 0x0100 -S 0xE800 -f CLDBbXSs -o fuzix.bin crt0.o devices.o main.o \
	commonmem.o tricks.o v8080.o devtty.o \
	../../start.o ../../version.o ../../cpu-8080/lowlevel-8080.o \
	../../mm/bankfixed.o ../../timer.o ../../kdata.o  ../../mm/memalloc_none.o \
	../../devio.o ../../filesys.o ../../process.o ../../inode.o ../../syscall_fs.o \
	../../syscall_proc.o ../../syscall_other.o ../../mm.o ../../swap.o \
	../../tty.o ../../devsys.o ../../usermem.o ../../syscall_fs2.o \
	../../syscall_fs3.o ../../syscall_exec.o ../../syscall_exec16.o  \
	../../cpu-8080/usermem_std-8080.o ../../blk512.o \
	../../dev/z80pack/devfd.o $(LIBC) -m fuzix.tmpmap
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map
	../../tools/pack85 <../../fuzix.map fuzix.bin ../../fuzix.bin
	cc85 -c bootblock.S
	ld85 -b bootblock.o -o bootblock.bin

diskimage: image
	# Boot floppy
	dd if=bootblock.bin of=$(IMAGES)/boot.dsk bs=256256 conv=sync
	dd if=../../fuzix.bin of=$(IMAGES)/boot.dsk bs=1 skip=256 seek=193024 conv=notrunc
	# Root hard disk. Mostly space so just grow to size
	dd if=/dev/zero of=$(IMAGES)/drivep.dsk bs=1 seek=536870911 count=1
	# Put the data over the start
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/drivep.dsk conv=notrunc
