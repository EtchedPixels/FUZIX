
CSRCS = devtty.c ppide.c
CSRCS += devices.c main.c libc.c ns202.c ds1302_ops.c

ASRCS = p68000.S crt0.S
ASRCS += tricks.S

DSRCS = ../dev/devide.c ../dev/mbr.c ../dev/blkdev.c ../dev/devide_discard.c
DSRCS += ../dev/ds1302.c ../dev/ds1302_discard.c
DSRCS += ../dev/devsd.c ../dev/devsd_discard.c
DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))

LSRCS = ../lib/68000exception.c ../lib/68000usercopy.c ../lib/68000relocate.c
LOBJS = $(patsubst ../lib/%.c,%.o, $(LSRCS))

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=.o)
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(LOBJS)

CROSS_CCOPTS += -I../dev/ -Wa,-l -DCPU=68000
ASOPTS += -Wa,-alhs=$*.lst -I..

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

$(DOBJS): %.o: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(LOBJS): %.o: ../lib/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

tricks.S: ../lib/68000flat.S


clean:
	rm -f *.o fuzix.elf fuzix.map \
		../../foo core *.lst #  *~   keep backups
	rm -f dos.part
	if [ -e tmpdir ] ; then rmdir tmpdir; fi
		
tidy:
	rm -f *~

image:
	$(CROSS_LD) -M -o fuzix.elf -T fuzix.ld \
	ns202.o ../start.o ../version.o \
	../lowlevel-68000.o 68000exception.o devtty.o \
	ds1302.o ds1302_discard.o ds1302_ops.o \
	ppide.o devide.o devide_discard.o devsd.o devsd_discard.o \
	tricks.o p68000.o main.o ../timer.o ../kdata.o devices.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o blkdev.o mbr.o ../syscall_other.o ../mm.o ../flat.o \
	../blk512.o ../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_fs3.o ../syscall_exec32.o ../syscall_exec.o \
	68000usercopy.o ../usermem_std-68000.o 68000relocate.o \
	libc.o ../malloc.o ../level2.o ../syscall_level2.o ../select.o \
	>../fuzix.map
	cp -a ../fuzix.map .
	$(CROSS_COMPILE)objcopy fuzix.elf -O binary ../fuzix.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition for FAT and FUZIX
	dd if=parttab of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Land the Fuzix file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=12288 conv=notrunc
	# Build the MSDOS partition
	dd if=/dev/zero of=dos.part bs=512 count=10240
	mkdosfs dos.part
	mkdir -p tmpdir
	sudo mount -o loop,umask=0 dos.part tmpdir
	mkdir tmpdir/fuzix
	cp fuzix.elf tmpdir/fuzix/
	cp fuz.cmd tmpdir
	cp boot.cmd tmpdir
	cp dfuz.cmd tmpdir
	sudo umount tmpdir
	# Attach the MSDOS partition to the disk
	dd if=dos.part of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
	# Clean up
	rm dos.part
	rmdir tmpdir
