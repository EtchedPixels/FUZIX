
CSRCS = devtty.c devhd.c
CSRCS += main.c devices.c

ASRCS = pz1.s crt0.s
ASRCS += tricks.s commonmem.s

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))

OBJS  = $(COBJS) $(AOBJS)

CROSS_CCOPTS += -I../dev/

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*$(BINEXT)

clean:
	rm -f $(OBJS) *.o core *~ fuzix.img fuzix.bin

image:
	$(CROSS_LD) -o ../fuzix.bin --mapfile ../fuzix.map -C ld65.cfg crt0.o commonmem.o \
	pz1.o ../start.o ../version.o ../lowlevel-6502.o \
	tricks.o main.o ../timer.o ../kdata.o devices.o \
	devhd.o ../memalloc_none.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../bankfixed.o \
	../tty.o ../devsys.o ../syscall_fs2.o ../syscall_fs3.o \
	../syscall_exec16.o ../syscall_exec.o ../blk512.o \
	../usermem.o ../usermem_std-6502.o devtty.o
	dd if=../fuzix.bin of=fuzix.img bs=256 skip=4 

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	cp fuzix.img $(IMAGES)/fuzix.rom
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
