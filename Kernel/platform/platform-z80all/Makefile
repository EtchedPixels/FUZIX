CROSS_CCOPTS += -I../../dev/

CSRCS = devtty.c devinput.o devices.c main.c

DISCSRCS = discard.c

ASRCS = crt0.S z80all.S video.S
ASRCS += tricks.S commonmem.S

DISCARD_DSRCS = ../../dev/tinydisk_discard.c ../../dev/tinyide_discard.c
DSRCS = ../../dev/tinydisk.c ../../dev/tinyide.c ../../dev/ps2kbd.o

COBJS = $(CSRCS:.c=.o)
AOBJS = $(ASRCS:.S=.o)
DISCOBJS = $(DISCSRCS:.c=.o)
DISCARD_DOBJS = $(DISCARD_DSRCS:.c=.o)
DOBJS = $(DSRCS:.c=.o)
DAOBJS = $(DASRCS:.S=.o)

OBJS  = $(COBJS) $(AOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS)

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

$(DAOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $@ $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ boot.ihx boot.o boot

#
#	Attach the kernel to the boot block
#
image:	boot
	$(CROSS_LD) -b -C 0x0100 -S 0xF400 -f CLDBbXSs -o fuzix.bin \
	crt0.o commonmem.o z80all.o ../../start.o \
	../../version.o ../../cpu-z80u/lowlevel-z80u.o \
	../../usermem.o tricks.o main.o discard.o \
	../../timer.o ../../kdata.o devices.o ../../devio.o \
	../../filesys.o ../../blk512.o ../../process.o \
	../../inode.o ../../syscall_exec.o ../../syscall_exec16.o \
	../../syscall_fs.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_proc.o ../../syscall_other.o ../../syscall_net.o \
	../../network.o ../../tty.o ../../mm.o ../../mm/memalloc_none.o \
	../../swap.o ../../mm/bankfixed.o ../../devsys.o \
	../../font/font8x8.o ../../vt.o video.o \
	devtty.o ../../dev/tinydisk.o ../../dev/tinydisk_discard.o \
	../../dev/tinyide.o ../../dev/tinyide_discard.o \
	../../dev/ps2kbd.o ../../devinput.o devinput.o \
	/opt/fcc/lib/z80/libz80.a -m fuzix.tmpmap
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map
	../../tools/pack85 <../../fuzix.map fuzix.bin ../../fuzix.bin
	
#
#	Compile up the boot block
#
boot: boot.s
	asz80 boot.s -o boot.o
	ldz80 -b -C 0 boot.o -o boot.bin
	../../tools/doubleup <boot.bin >boot

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=../../fuzix.bin of=$(IMAGES)/disk.img bs=256 seek=4 skip=1 conv=notrunc
	# Add the z80all bootstrap
	dd if=boot of=$(IMAGES)/disk.img bs=446 count=1 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.20M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
