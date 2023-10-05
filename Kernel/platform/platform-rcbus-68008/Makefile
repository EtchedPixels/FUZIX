
CSRCS = devtty.c devinput.c devgpio.c
CSRCS += devices.c main.c libc.c ppide.c tms9918.c 

ASRCS = p68000.S crt0.S
ASRCS += tricks.S spi.S ds1302io.S

DSRCS = ../dev/devide.c ../dev/mbr.c ../dev/blkdev.c ../dev/devide_discard.c
DSRCS += ../dev/devsd.c ../dev/devsd_discard.c
DSRCS += ../dev/ds1302.c ../dev/ds1302_discard.c
DSRCS += ../dev/ps2bitbang.c ../dev/ps2bitbang_rcbus.c ../dev/ps2kbd.c
DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))

LSRCS = ../lib/68000exception.c ../lib/68000usercopy.c ../lib/68000relocate.c
LOBJS = $(patsubst ../lib/%.c,%.o, $(LSRCS))

RSRCS = ../dev/rcbus/zxkey.c ../dev/rcbus/zxkeyboard.c
ROBJS = $(patsubst ../dev/rcbus/%.c,%.o, $(RSRCS))

NSRCS = ../dev/net/net_native.c
NOBJS = $(patsubst ../dev/net/%.c,%.o, $(NSRCS))

COBJS = $(CSRCS:.c=.o)
AOBJS = $(ASRCS:.S=.o)
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(LOBJS) $(NOBJS) $(ROBJS)

CROSS_CCOPTS += -I../dev/ -I../dev/rcbus/ -I../dev/net

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

$(DOBJS): %.o: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(NOBJS): %.o: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(ROBJS): %.o: ../dev/rcbus/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(LOBJS): %.o: ../lib/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

tricks.S: ../lib/68000flat.S

clean:
	rm -f *.o fuzix.elf loader.elf loader.map loader.bin core *~ 

image:
	$(CROSS_LD) -M -o fuzix.elf -T fuzix.ld \
	p68000.o ../start.o ../version.o ../lowlevel-68000.o \
	tricks.o main.o ../timer.o ../kdata.o 68000exception.o devices.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../flat.o \
	../tty.o ../vt.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_fs3.o ../syscall_exec32.o ../syscall_exec.o \
	../blk512.o ../font6x8.o 68000usercopy.o 68000relocate.o \
	blkdev.o devide.o devide_discard.o mbr.o zxkey.o zxkeyboard.o \
	../usermem_std-68000.o devtty.o libc.o ../malloc.o ppide.o \
	devsd.o devsd_discard.o spi.o ds1302.o ds1302_discard.o ds1302io.o \
	devinput.o ../devinput.o devgpio.o tms9918.o \
	ps2kbd.o ps2bitbang.o ps2bitbang_rcbus.o \
	../level2.o ../syscall_level2.o ../select.o \
	../syscall_net.o ../network.o net_native.o > ../fuzix.map
	$(CROSS_COMPILE)objcopy fuzix.elf -O binary ../fuzix.bin
	$(CROSS_CC) -c loader.S
	$(CROSS_LD) -M -o loader.elf -T loader.ld loader.o >loader.map
	$(CROSS_COMPILE)objcopy loader.elf -O binary loader.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the bootstrap loader
	dd if=loader.bin of=$(IMAGES)/disk.img conv=notrunc
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=../fuzix.bin of=$(IMAGES)/disk.img bs=512 seek=1 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
