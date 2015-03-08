HOSTCC = gcc
CC = sdcc
CPP = cpp -nostdinc -undef -P
AS = sdasz80
AR = sdar

ARCH = z80

# Find what load address the kernel wants.
PROGLOAD = $(shell \
	(cat $(TOP)/Kernel/platform-$(PLATFORM)/config.h && echo PROGLOAD) | \
	cpp -E | tail -n1)

# CFLAGS used everwhere.
CFLAGS = \
	-mz80 \
	--std-c99 \
	--opt-code-size \
	$(COPTIMISATION)

COPTIMISATION = --max-allocs-per-node 10000
$(OBJ)/Library/libs/regexp.$O: COPTIMISATION = --max-allocs-per-node 10000

# Used when linking user mode executables (but not the kernel).
$(OBJ)/Applications/%: LDFLAGS = \
	-mz80 \
	--nostdlib \
	--no-std-crt0 \
	--code-loc $(PROGLOAD) \
	--data-loc 0

include $(TOP)/Build/sdcc-rules.mk

