LIBC=

ESP_INCLUDES = \
	-I $(HOME)/src/esp-idf/components/esp_common/include \
	-I $(HOME)/src/esp-idf/components/soc/include \
	-I $(HOME)/src/esp-idf/components/soc/esp32/include \
	-I $(HOME)/src/esp-idf/components/hal/include \
	-I $(HOME)/src/esp-idf/components/hal/esp32/include \
	-I $(HOME)/src/esp-idf/components/hal/platform_port/include \

ASOPTS += $(ESP_INCLUDES)
CROSS_CCOPTS += $(ESP_INCLUDES)

CSRCS += \
	../dev/blkdev.c \
	../dev/mbr.c \
	../lib/hexdump.c \
	devices.c \
	devtty.c \
	interrupt.c \
	lib.c \
	main.c \
	procore.c \
	swapper.c \
	syscall_exec.c \

ASRCS += \
	boot.S \
	tricks.S \

AOBJS = $(ASRCS:.S=.o)
COBJS = $(CSRCS:.c=.o)

OBJS = $(AOBJS) $(COBJS)

KOBJS = \
	$(OBJS) \
	../devio.o \
	../devsys.o \
	../filesys.o \
	../inode.o \
	../kdata.o \
	../lowlevel-esp32.o \
	../malloc.o \
	../mm.o \
	../network.o \
	../process.o \
	../start.o \
	../swap.o \
	../syscall_exec.o \
	../syscall_fs.o \
	../syscall_fs2.o \
	../syscall_fs3.o \
	../syscall_net.o \
	../syscall_other.o \
	../syscall_proc.o \
	../timer.o \
	../tty.o \
	../usermem.o \
	../version.o \

JUNK = *.o *.lst *.asm *.sym *.rst

all: $(OBJS)

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) -I.. $< -o $@

$(OBJS): ../kernel-esp32.def

$(COBJS): %.o: %.c
	$(CROSS_CC) -I../dev/ -I../dev/net $(CROSS_CCOPTS) $< -o $@

image.elf: kernel.ld addresses.ld $(KOBJS)
	$(CROSS_CC) -T kernel.ld -T addresses.ld -flto -mlongcalls -nostdlib -o image.elf \
		$(KOBJS)

image: image.elf Makefile
	xtensa-esp32-elf-objdump -Sr image.elf >image.src
	xtensa-esp32-elf-nm image.elf >../fuzix.map
	xtensa-esp32-elf-objcopy image.elf -O binary image.bin
	xtensa-esp32-elf-objcopy -I binary image.bin -O elf32-xtensa-le --change-addresses 0x40080000 image-absolute.elf
	esptool --chip esp32 elf2image -o $@ image-absolute.elf

clean:
	rm -f $(OBJS) $(JUNK)  core *~

flash::
	esptool --chip esp32 --port /dev/ttyUSB0 --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 image

