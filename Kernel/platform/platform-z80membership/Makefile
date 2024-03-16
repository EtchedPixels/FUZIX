CROSS_CCOPTS += -I../../dev/

CSRCS = devtty.c devices.c main.c sdcard.c

DISCSRCS = discard.c

ASRCS = z80membership.S crt0.S
ASRCS += tricks.S commonmem.S

DISCARD_DSRCS = ../../dev/tinydisk_discard.c ../../dev/tinysd_discard.c
DSRCS = ../../dev/tinydisk.c ../../dev/tinysd.c
NSRCS =

DASRCS = 

COBJS = $(CSRCS:.c=.o)
AOBJS = $(ASRCS:.S=.o)
NOBJS = $(patsubst ../../dev/net/%.c,%.o, $(NSRCS))
DISCOBJS = $(DISCSRCS:.c=.o)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.S,%.o, $(DASRCS))

OBJS  = $(COBJS) $(AOBJS) $(NOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS)

JUNK =  *.lst *.asm *.Sym *.rst *.lst *.o *.tmp *.ihx

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
	rm -f $(OBJS) $(JUNK)  core *~ *.tmp

bin2ihx:
	cc -Wall -pedantic ../../tools/bin2ihx.c -o ../../tools/bin2ihx

loader.ihx: loader.S bin2ihx
	asz80 loader.S
	ldz80  -b loader.o -o loader.bin
	../../tools/bin2ihx loader.bin 0xF800 0x0100 >loader.ihx

	# FIXME: need to convert to IHX
	#objcopy --input-target=binary --output-target=ihx loader.bin loader.ihx

image: loader.ihx
	ldz80 -b -C 0x0100 -S 0xF400 -f CLDBbXSs -o fuzix.bin \
	crt0.o commonmem.o z80membership.o tricks.o main.o discard.o devices.o \
	devtty.o sdcard.o \
	../../start.o ../../version.o ../../usermem.o ../../timer.o \
	../../kdata.o ../../devio.o ../../filesys.o ../../blk512.o \
	../../process.o ../../inode.o ../../syscall_exec.o \
	../../syscall_exec16.o ../../syscall_fs.o ../../syscall_fs2.o \
	../../syscall_fs3.o ../../syscall_proc.o ../../syscall_other.o \
	../../syscall_net.o ../../tty.o ../../mm.o ../../mm/memalloc_none.o \
	../../swap.o ../../mm/bankfixed.o ../../devsys.o \
	../../dev/tinydisk.o ../../dev/tinydisk_discard.o \
	../../dev/tinysd.o ../../dev/tinysd_discard.o \
	../../cpu-z80u/lowlevel-z80u.o \
	/opt/fcc/lib/z80/libz80.a -m fuzix.tmpmap
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map
	../../tools/pack85 <../../fuzix.map fuzix.bin ../../fuzix.bin


IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition for FAT and FUZIX
	dd if=parttab of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Land the Fuzix file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=12288 conv=notrunc
	# Build the MSDOS partition
	dd if=/dev/zero of=dos.part bs=512 count=10240
	mkdosfs dos.part
	mkdir -p tmpdir
	sudo mount -o loop,umask=0 dos.part tmpdir
	cp loader.ihx  tmpdir/FUZIX.HEX
	sudo umount tmpdir
	# Attach the MSDOS partition to the disk
	dd if=dos.part of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=../../fuzix.bin of=$(IMAGES)/disk.img bs=256 skip=1 seek=6 conv=notrunc
	# Clean up
	rm dos.part
	rmdir tmpdir
