LIBC=

CSRCS += \
	../dev/blkdev.c \
	../dev/mbr.c \
	../dev/devsd_discard.c \
	../dev/devsd.c \
	../lib/dhara/error.c \
	../lib/dhara/journal.c \
	../lib/dhara/map.c \
	boot.c \
	devices.c \
	devflash.c \
	devsdspi.c \
	devtty.c \
	lib.c \
	main.c \
	rawflash.c \
	swapper.c \
	syscall_exec.c \

ASRCS = tricks.S

AOBJS = $(ASRCS:.S=.o)
COBJS = $(CSRCS:.c=.o)

OBJS  = $(AOBJS) $(COBJS)

KOBJS = \
	$(OBJS) \
	../devio.o \
	../devsys.o \
	../filesys.o \
	../inode.o \
	../kdata.o \
	../lowlevel-esp8266.o \
	../malloc.o \
	../mm.o \
	../process.o \
	../start.o \
	../swap.o \
	../syscall_exec.o \
	../syscall_fs.o \
	../syscall_fs2.o \
	../syscall_fs3.o \
	../syscall_other.o \
	../syscall_proc.o \
	../timer.o \
	../tty.o \
	../usermem.o \
	../version.o \

JUNK = *.o *.lst *.asm *.sym *.rst

all:	$(OBJS)

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) -I.. $< -o $@

$(OBJS): ../kernel-esp8266.def

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -I../dev $< -o $@

image.elf: kernel.ld addresses.ld $(KOBJS)
	$(CROSS_CC) -T kernel.ld -T addresses.ld -flto -mlongcalls -nostdlib -o image.elf \
		$(KOBJS)
	
image: image.elf
	esptool elf2image image.elf
	touch image

burn:
	esptool --port /dev/ttyUSB0 write_flash 0x00000 image.elf-0x00000.bin 0x10000 image.elf-0x10000.bin -ff 80m -fm dout
	#esptool --port /dev/ttyUSB0 write_flash 0x00000 image.elf-0x00000.bin 0x10000 image.elf-0x10000.bin 0x100000 ../filesystem.ftl -ff 80m -fm dout
	#esptool --port /dev/ttyUSB0 write_flash 0x00000 image.elf-0x00000.bin -ff 80m -fm dout
	
clean:
	rm -f $(OBJS) $(JUNK)  core *~

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
