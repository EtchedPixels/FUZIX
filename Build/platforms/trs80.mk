O = rel
A = lib

# SDCC setup.

SDCC = sdcc
SDCPP = cpp -nostdinc -undef -P
SDAS = sdasz80
SDAR = sdar

# Which file contains the target rules for this platform.

PLATFORM_RULES = $(BUILD)/platforms/sdcc.rules.mk

# Find what load address the kernel wants.

PROGLOAD = $(shell \
        (cat $(TOP)/Kernel/platform-$(PLATFORM)/config.h && echo PROGLOAD) | \
        cpp -E | tail -n1)

# CFLAGS used everywhere.

PLATFORM_CFLAGS = \
	-mz80 \
	--std-c99 \
	--opt-code-size \
	-Ddouble=float

# Used when linking user mode executables (but not the kernel).

PLATFORM_TARGET_LDFLAGS = \
	-mz80 \
	--nostdlib \
	--no-std-crt0 \
	--code-loc $(PROGLOAD) \
	--data-loc 0

