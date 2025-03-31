
CSRCS = devtty.c
CSRCS += devices.c main.c

ASRCS = rcbus-8070.S crt0.S
ASRCS += tricks.S commonmem.S

CDSRCS = discard.c

DSRCS = ../../dev/tinydisk.c ../../dev/tinyide.c
DISCARD_DSRCS = ../../dev/tinydisk_discard.c ../../dev/tinyide_discard.c

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
	rm -f commonmem.s crt0.s rcbus-8070.s tricks.s

image:
	$(CROSS_LD) -b -C 0x0200 -S 0xF000 -Z 0xFFC0 -o ../../fuzix.bin -m fuzix.tmpmap crt0.o commonmem.o \
	rcbus-8070.o ../../start.o ../../version.o ../../cpu-8070/lowlevel-8070.o \
	tricks.o main.o discard.o ../../timer.o ../../kdata.o devices.o \
	tinydisk.o tinydisk_discard.o tinyide.o tinyide_discard.o \
	../../mm/memalloc_none.o \
	../../devio.o ../../filesys.o ../../process.o ../../inode.o ../../syscall_fs.o \
	../../syscall_proc.o ../../syscall_other.o ../../mm.o ../../swap.o ../../mm/bank16kfc.o \
	../../tty.o ../../devsys.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_exec.o ../../syscall_exec16.o ../../blk512.o \
	../../usermem.o ../../cpu-8070/usermem_std-8070.o devtty.o /opt/fcc/lib/8070/lib8070.a
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map
	dd if=../../fuzix.bin of=fuzix.img bs=256 skip=4

loader.o: loader.S
	cpp -E $< >$*.s
	$(CROSS_AS) $(ASOPTS) $*.s

loader: loader.o
	$(CROSS_LD) -b -C 0x200 -Z 0x20 -o loader.bin loader.o

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: loader
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the bootstrap loader
	dd if=loader.bin bs=512 skip=1 of=$(IMAGES)/disk.img conv=notrunc
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=fuzix.img of=$(IMAGES)/disk.img bs=512 seek=1 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
