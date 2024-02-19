
CSRCS = devtty.c devhd.c
CSRCS += main.c devices.c

CDSRCS = ../../dev/tinydisk_discard.c

DSRCS = ../../dev/tinydisk.c

ASRCS = pz1.s crt0.s
ASRCS += tricks.s commonmem.s

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
CDOBJS = $(CDSRCS:.c=$(BINEXT))

OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(CDOBJS)

CROSS_CCOPTS += -I../../dev/

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) $<

$(CDOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %$(BINEXT): ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(HIGH) -c $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*$(BINEXT)

clean:
	rm -f $(OBJS) *.o core *~ fuzix.img fuzix.bin

image:
	$(CROSS_LD) -o ../../fuzix.bin --mapfile ../../fuzix.map -C ld65.cfg crt0.o commonmem.o \
	pz1.o ../../start.o ../../version.o ../../cpu-6502/lowlevel-6502.o \
	tricks.o main.o ../../timer.o ../../kdata.o devices.o \
	devhd.o ../../mm/memalloc_none.o ../../dev/tinydisk.o ../../dev/tinydisk_discard.o \
	../../devio.o ../../filesys.o ../../process.o ../../inode.o ../../syscall_fs.o \
	../../syscall_proc.o ../../syscall_other.o ../../mm.o ../../mm/bankfixed.o \
	../../tty.o ../../devsys.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_exec16.o ../../syscall_exec.o ../../blk512.o \
	../../usermem.o ../../cpu-6502/usermem_std-6502.o devtty.o

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	cp ../../fuzix.bin $(IMAGES)/fuzix.rom
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
