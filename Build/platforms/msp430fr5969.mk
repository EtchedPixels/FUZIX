HOSTCC = gcc
CC = msp430-elf-gcc
AR = msp430-elf-ar
LD = msp430-elf-ld
OBJCOPY = msp430-elf-objcopy

ARCH = msp430x

# The MSP430 user space is just under 32kB, and isn't big enough for the 
# larger apps.
WANT_LARGE_APPLICATIONS = n

# Find what load address the kernel wants.
PROGLOAD = $(shell \
	(cat $(TOP)/Kernel/platform-$(PLATFORM)/config.h && echo PROGLOAD) | \
	cpp -E | tail -n1)

# CFLAGS used everwhere.
CFLAGS = \
	-g \
	-ffunction-sections \
	-fdata-sections \
	-fno-inline \
	--short-enums \
	$(COPTIMISATION)

# Linker flags used everwhere.
LDFLAGS = \
	--gc-sections \
	-g

# Assembler flags used everywhere.
ASFLAGS = \
	-g

COPTIMISATION = -Os

# Used when linking user mode executables (but not the kernel).
$(OBJ)/Applications/%: LDFLAGS = \
	-T $(TOP)/Build/platforms/msp430fr5969.ld \
	--relax

include $(TOP)/Build/genericgcc-rules.mk
include $(TOP)/Build/host-rules.mk

