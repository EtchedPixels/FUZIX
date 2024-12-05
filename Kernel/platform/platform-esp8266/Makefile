LIBC=

CSRCS += \
	../../dev/blkdev.c \
	../../dev/mbr.c \
	../../dev/devsd_discard.c \
	../../dev/devsd.c \
	../../dev/net/net_w5x00.c \
	../../lib/dhara/error.c \
	../../lib/dhara/journal.c \
	../../lib/dhara/map.c \
	boot.c \
	devices.c \
	devflash.c \
	devsdspi.c \
	devtty.c \
	interrupt.c \
	lib.c \
	main.c \
	misc.c \
	rawflash.c \
	swapper.c \
	syscall_exec.c \

ASRCS = tricks.S

AOBJS = $(ASRCS:.S=.o)
COBJS = $(CSRCS:.c=.o)

OBJS  = $(AOBJS) $(COBJS)

KOBJS = \
	$(OBJS) \
	../../blk512.o \
	../../devio.o \
	../../devsys.o \
	../../filesys.o \
	../../inode.o \
	../../kdata.o \
	../../cpu-esp8266/lowlevel-esp8266.o \
	../../malloc.o \
	../../mm.o \
	../../network.o \
	../../process.o \
	../../start.o \
	../../swap.o \
	../../syscall_exec.o \
	../../syscall_fs.o \
	../../syscall_fs2.o \
	../../syscall_fs3.o \
	../../syscall_net.o \
	../../syscall_other.o \
	../../syscall_proc.o \
	../../timer.o \
	../../tty.o \
	../../usermem.o \
	../../version.o \

JUNK = *.o *.lst *.asm *.sym *.rst

all:	$(OBJS)

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) -I.. $< -o $@

$(OBJS): ../../cpu-esp8266/kernel-esp8266.def

$(COBJS): %.o: %.c
	$(CROSS_CC) -I../../dev/ -I../../dev/net $(CROSS_CCOPTS) $< -o $@

image.elf: kernel.ld addresses.ld $(KOBJS)
	$(CROSS_CC) -T kernel.ld -T addresses.ld -flto -mlongcalls -nostdlib -o image.elf \
		$(KOBJS)
	
filesystem.img:
	./update-flash.sh

filesystem.ftl: filesystem.img
	# -s is the flash size in kilobytes; the kernel and boot loader takes the first 64kB.
	../../../Standalone/mkftl -s 1984 -e 0x1000 -p 512 -g 10 filesystem.img -o filesystem.ftl

filesystem.elf: filesystem.ftl
	xtensa-lx106-elf-objcopy -I binary -O elf32-xtensa-le --change-section-vma .data=0x40220000 filesystem.ftl filesystem.elf
	xtensa-lx106-elf-ld -Tdata 0x40220000 -o filesystem2.elf filesystem.elf
	mv filesystem2.elf filesystem.elf

fburn: filesystem.elf
	esptool elf2image filesystem.elf
	esptool --port /dev/ttyUSB0 write_flash 0x20000 filesystem.elf-0x20000.bin -fs 4MB -ff 80m -fm dio

image: image.elf
	xtensa-lx106-elf-objdump -S image.elf >image.src
	xtensa-lx106-elf-nm image.elf >../../fuzix.map
	esptool elf2image image.elf
	touch image

burn: image
	# You can use qio on a lot of parts but if it doesn't work try dio. For the ESP8285 use dout
	esptool --port /dev/ttyUSB0 write_flash 0x00000 image.elf-0x00000.bin 0x10000 image.elf-0x10000.bin -fs 4MB -ff 80m -fm dio
	#esptool --port /dev/ttyUSB0 write_flash 0x00000 image.elf-0x00000.bin 0x10000 image.elf-0x10000.bin 0x100000 filesystem.ftl -ff 80m -fm dout
	#esptool --port /dev/ttyUSB0 write_flash 0x00000 image.elf-0x00000.bin -ff 80m -fm dout
	
clean:
	rm -f $(OBJS) $(JUNK)  core *~

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
