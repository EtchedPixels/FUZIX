# 'Bare metal' ESP32 application Makefile
# Use the xtensa-esp32-elf toolchain.
TOOLCHAIN = xtensa-lx106-elf-

CFLAGS_PLATFORM  = -mlongcalls -mtext-section-literals -fstrict-volatile-bitfields
ASFLAGS_PLATFORM = $(CFLAGS_PLATFORM)
LDFLAGS_PLATFORM = $(CFLAGS_PLATFORM)

###
# General project build
###
CC = $(TOOLCHAIN)gcc
LD = $(TOOLCHAIN)ld
OC = $(TOOLCHAIN)objcopy
OS = $(TOOLCHAIN)size

# Linker script location.
LDSCRIPT       = ./fuzix.ld
# Set C/LD/AS flags.
CFLAGS += $(INC) -Wall -std=gnu11 -nostdlib $(CFLAGS_PLATFORM) $(COPT) \
	-I $(HOME)/src/esp-idf/components/soc/esp32/include
# (Allow access to the same memory location w/ different data widths.)
CFLAGS += -fno-strict-aliasing
CFLAGS += -fdata-sections -ffunction-sections
#CFLAGS += -Os
CFLAGS += -Os -g
LDFLAGS += -nostdlib -T$(LDSCRIPT) -Wl,-Map=$@.map -Wl,--cref -Wl,--gc-sections
LDFLAGS += $(LDFLAGS_PLATFORM)
LDFLAGS += -lm -lc -lgcc
ASFLAGS += -c -O0 -Wall -fmessage-length=0 \
	-I $(HOME)/src/esp-idf/components/soc/esp32/include \
	-I $(HOME)/src/esp-idf/components/esp_common/include
ASFLAGS += $(ASFLAGS_PLATFORM)

OBJS = \
	./boot.o \
	./main.o

# Set the first rule in the file to 'make all'
.PHONY: all
all: main.bin

# Rules to build files.
%.o: %.S
	$(CC) -x assembler-with-cpp $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

main.elf: $(OBJS) $(LDSCRIPT)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

main.bin: main.elf
	esptool --chip esp32 elf2image -o $@ $<

flash::
	esptool --chip esp32 --port /dev/ttyUSB0 --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 main.bin

# Target to clean build artifacts.
.PHONY: clean
clean:
	rm -f $(OBJS)
	rm -f ./main.bin
	rm -f ./main.elf ./main.elf.map
