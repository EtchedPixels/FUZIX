
CSRCS = devtty.c ds3234.c
CSRCS += devices.c main.c libc.c

ASRCS = 68knano.S crt0.S
ASRCS += tricks.S

DSRCS = ../dev/devide.c ../dev/mbr.c ../dev/blkdev.c ../dev/devide_discard.c
DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))

LSRCS = ../lib/68000exception.c ../lib/68000usercopy.c ../lib/68000relocate.c
LOBJS = $(patsubst ../lib/%.c,%.o, $(LSRCS))

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=.o)
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(LOBJS)

CROSS_CCOPTS += -I../dev/

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
	rm -f *.o fuzix.elf loader.elf loader.map loader.bin core *~ 
	rm -f fuzix.rom

image:
	$(CROSS_LD) -M -o fuzix.elf -T fuzix.ld \
	68knano.o ../start.o ../version.o ../lowlevel-68000.o \
	tricks.o main.o ../timer.o ../kdata.o 68000exception.o devices.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../flat.o \
	../blk512.o ../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_fs3.o ../syscall_exec32.o ../syscall_exec.o \
	ds3234.o blkdev.o devide.o devide_discard.o mbr.o \
	68000usercopy.o ../usermem_std-68000.o devtty.o libc.o ../malloc.o \
	68000relocate.o \
	../level2.o ../syscall_level2.o ../select.o > ../fuzix.map
	$(CROSS_COMPILE)objcopy fuzix.elf -O binary ../fuzix.bin
	dd if=../fuzix.bin of=fuzix.rom bs=65536 conv=sync

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
	cp fuzix.rom $(IMAGES)
