CROSS_CCOPTS += -I../../dev/

CSRCS = devtty.c
CSRCS += devices.c main.c devsasi.c devinput.c devlpr.c 

DISCSRCS = discard.c

ASRCS = lobo.S crt0.S
ASRCS += tricks.S commonmem.S

DISCARD_DSRCS = ../../dev/tinydisk_discard.c ../../dev/tinyscsi_discard.c
DSRCS = ../../dev/tinydisk.c ../../dev/tinyscsi.c

COBJS = $(CSRCS:.c=.o)
AOBJS = $(ASRCS:.S=.o)
DISCOBJS = $(DISCSRCS:.c=.o)
DISCARD_DOBJS = $(DISCARD_DSRCS:.c=.o)
DOBJS = $(DSRCS:.c=.o)

OBJS  = $(COBJS) $(AOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS)

JUNK =  *.o

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCOBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_DOBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~

#
#	Attach the kernel to the boot block
#
image:	boot
	$(CROSS_LD) -b -S 0x1003 -f SsLDBbXC -o fuzix.bin \
	crt0.o commonmem.o lobo.o ../../start.o \
	../../version.o ../../cpu-z80u/lowlevel-z80u.o \
	../../usermem.o tricks.o main.o discard.o \
	../../timer.o ../../kdata.o devices.o ../../devio.o \
	../../filesys.o ../../blk512.o ../../process.o \
	../../inode.o ../../syscall_exec.o ../../syscall_exec16.o \
	../../syscall_fs.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_proc.o ../../syscall_other.o ../../syscall_net.o \
	../../network.o ../../tty.o ../../mm.o ../../mm/memalloc_none.o \
	../../swap.o ../../mm/bankfixed.o ../../devsys.o ../../font/font8x8.o \
	../../devinput.o devinput.o ../../vt.o devsasi.o devlpr.o \
	devtty.o ../../dev/tinydisk.o ../../dev/tinydisk_discard.o \
	../../dev/tinyscsi.o ../../dev/tinyscsi_discard.o \
	/opt/fcc/lib/z80/libz80.a -m fuzix.tmpmap
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map
	
#
#	Compile up the boot block
#
boot: boot.s
	asz80 boot.s -o boot.o
	ldz80 -b -C 0 boot.o -o boot.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Now make the boot floppy
	dd if=/dev/zero of=$(IMAGES)/boot.5 bs=512 count=400
	dd if=boot.bin of=$(IMAGES)/boot.5 bs=512 skip=2 count=1 conv=notrunc
	# Place after bootblock starting from 0x1000
	dd if=fuzix.bin of=$(IMAGES)/boot.5 bs=512 seek=1 skip=8 conv=notrunc
