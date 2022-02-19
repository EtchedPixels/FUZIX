
CSRCS = devtty.c
CSRCS += devices.c main.c libc.c

ASRCS = p68000.S crt0.S
ASRCS += tricks.S

DSRCS = ../dev/devide.c ../dev/mbr.c ../dev/blkdev.c ../dev/devide_discard.c
DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))

LSRCS = ../lib/68000exception.c
LOBJS = $(patsubst ../lib/%.c,%.o, $(LSRCS))

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=.o)
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(LOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = $(CSRCS:.c=.o) $(ASRCS:.S=.o)

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

$(DOBJS): %.o: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(LOBJS): %.o: ../lib/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	$(CROSS_LD) -M -o fuzix.elf -T fuzix.ld \
	crt0.o \
	p68000.o ../start.o ../version.o ../lowlevel-68000.o \
	tricks.o ../simple.o main.o ../timer.o ../kdata.o devices.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o 68000exception.o \
	../syscall_fs3.o ../syscall_exec32.o ../syscall_exec.o blkdev.o \
	devide.o devide_discard.o mbr.o  \
	../usermem_std-68000.o devtty.o libc.o ../malloc.o > ../fuzix.map
	$(CROSS_COMPILE)objcopy fuzix.elf -O binary ../fuzix.bin

loader.bin:
	$(CROSS_CC) -Wl,-Ttext=0x1000 -nostdlib -o loader.elf loader.S
	$(CROSS_COMPILE)objcopy loader.elf -O binary --only-section=.text loader.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image loader.bin
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=10485760 conv=sync
	# Add the loader
	dd if=loader.bin of=$(IMAGES)/disk.img bs=512 conv=notrunc
	# Add the file system
	(cd $(FUZIX_ROOT)/Standalone/filesystem-src; ./build-filesystem $(ENDIANFLAG) $(FUZIX_ROOT)/Images/$(TARGET)/filesys.img 256 30720)
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=10240 conv=notrunc
	# Add the kernel
	dd if=../fuzix.bin of=$(IMAGES)/disk.img bs=512 seek=1 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.20M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img

