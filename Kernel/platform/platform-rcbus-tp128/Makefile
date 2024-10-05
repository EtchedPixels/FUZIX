CROSS_CCOPTS += -I../../dev/ -I../../dev/net/


CSRCS = devtty.c
CSRCS += devices.c main.c wiznet.c

DISCSRCS = discard.c

ASRCS = tp128.S crt0.S ide.S tricks.S commonmem.S

DISCARD_DSRCS = ../../dev/tinydisk_discard.c ../../dev/ds1302_discard.c ../../dev/tinyide_discard.c
DSRCS = ../../dev/tinydisk.c ../../dev/tinyide.c
DSRCS += ../../dev/ds1302.c
NSRCS = ../../dev/net/net_w5x00.c ../../dev/net/net_w5300.c ../../dev/net/net_native.c

DASRCS = ../../dev/ds1302_rcbusu.S

COBJS = $(CSRCS:.c=.o)
AOBJS = $(ASRCS:.S=.o)
NOBJS = $(patsubst ../../dev/net/%.c,%.o, $(NSRCS))
DISCOBJS = $(DISCSRCS:.c=.o)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.S,%.o, $(DASRCS))

OBJS  = $(COBJS) $(AOBJS) $(NOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS)

JUNK =  *.o

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCOBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.o: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

$(DAOBJS): %.o: ../../dev/%.S
	$(CROSS_AS) $(ASOPTS) $@ $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ *.ihx *.bin *.o *.tmp *.s

#
#	Attach the kernel to the boot block
#
image:	loader.bin
	$(CROSS_LD) -b -S 0x0080 -f SsLDBbXC -o fuzix.bin \
	crt0.o commonmem.o tp128.o ../../start.o ../../version.o \
	../../cpu-z80u/lowlevel-z80u.o ../../usermem.o tricks.o main.o discard.o \
	../../timer.o ../../kdata.o devices.o ../../devio.o ../../filesys.o \
	../../blk512.o ../../process.o ../../inode.o ../../syscall_exec.o \
	../../syscall_exec16.o ../../syscall_fs.o ../../syscall_fs2.o \
	../../syscall_fs3.o ../../syscall_proc.o ../../syscall_other.o \
	../../syscall_net.o ../../network.o ../../tty.o ../../mm.o ../../mm/bankfixed.o \
	../../mm/memalloc_none.o ../../swap.o ../../devsys.o devtty.o \
	../../dev/tinydisk.o ../../dev/tinydisk_discard.o \
	../../dev/tinyide.o ../../dev/tinyide_discard.o ide.o \
	../../dev/ds1302.o ../../dev/ds1302_discard.o ../../dev/ds1302_rcbusu.o \
	../../dev/net/net_native.o ../../dev/net/net_w5x00.o \
	../../dev/net/net_w5300.o wiznet.o \
	/opt/fcc/lib/z80/libz80.a -m fuzix.tmpmap
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map
	cp fuzix.bin ../../fuzix.bin

#
#	Compile up the boot block
#
loader.bin: loader.S
	asz80 loader.S -o loader.o
	ldz80 -b -C 0 loader.o -o loader.tmp
	dd if=loader.tmp bs=256 skip=208 of=loader.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Bootstrap (in two pieces to avoid the partition table)
	dd if=loader.bin of=$(IMAGES)/disk.img bs=446 count=1 conv=notrunc
	dd if=loader.bin of=$(IMAGES)/disk.img bs=512 skip=1 seek=1 conv=notrunc
	# And kernel
	dd if=../../fuzix.bin of=$(IMAGES)/disk.img bs=256 seek=48 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.20M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
