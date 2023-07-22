
CSRCS = devtty.c
CSRCS += devices.c main.c libc.c

ASRCS = pico.S crt0.S
ASRCS += tricks.S

DSRCS = ../dev/tinydisk.c ../dev/tinysd.c ../dev/tinydisk_discard.c ../dev/tinysd_discard.c
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
	rm -f *.o fuzix.elf core *~ 
	rm -f bootrom.bin bootrom.elf bootrom.rom bootrom.map

image: bootrom
	$(CROSS_LD) -M -o fuzix.elf -T fuzix.ld \
	pico.o ../start.o ../version.o ../lowlevel-68000.o \
	tricks.o main.o ../timer.o ../kdata.o 68000exception.o devices.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../flat_small.o \
	../blk512.o ../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_fs3.o ../syscall_exec32.o ../syscall_exec.o \
	tinydisk.o tinysd.o tinydisk_discard.o tinysd_discard.o \
	68000usercopy.o ../usermem_std-68000.o devtty.o libc.o \
	68000relocate.o ../page.o \
	../level2.o ../syscall_level2.o ../select.o > ../fuzix.map
	$(CROSS_COMPILE)objcopy fuzix.elf -O binary ../fuzix.bin

bootrom: bootrom.S
	$(CROSS_CC) -c bootrom.S
	$(CROSS_LD) -M -o bootrom.elf bootrom.o -T rom.ld >bootrom.map
	$(CROSS_COMPILE)objcopy bootrom.elf -O binary bootrom.bin
	dd if=bootrom.bin of=bootrom.rom bs=8192 conv=sync

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the kernel
	dd if=../fuzix.bin of=$(IMAGES)/disk.img bs=512 seek=1 conv=notrunc
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	cp bootrom.rom $(IMAGES)
