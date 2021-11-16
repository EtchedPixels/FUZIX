LIBC=

CROSS_CCOPTS += \
	-I $(HOME)/src/esp-idf/components/soc/esp32/include \

CSRCS += \
	main.c

ASRCS = \
	boot.S

AOBJS = $(ASRCS:.S=.o)
COBJS = $(CSRCS:.c=.o)

OBJS = $(AOBJS) $(COBJS)

KOBJS = \
	$(OBJS)

JUNK = *.o *.lst *.asm *.sym *.rst

all:    $(OBJS)

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) -I.. $< -o $@

$(OBJS): ../kernel-esp32.def

$(COBJS): %.o: %.c
	$(CROSS_CC) -I../dev/ -I../dev/net $(CROSS_CCOPTS) $< -o $@

image.elf: kernel.ld addresses.ld $(KOBJS)
	$(CROSS_CC) -T kernel.ld -T addresses.ld -flto -mlongcalls -nostdlib -o image.elf \
		$(KOBJS)

image: image.elf
	xtensa-lx106-elf-objdump -S image.elf >image.src
	xtensa-lx106-elf-nm image.elf >../fuzix.map
	esptool --chip esp32 elf2image -o $@ $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~

flash::
	esptool --chip esp32 --port /dev/ttyUSB0 --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 image

