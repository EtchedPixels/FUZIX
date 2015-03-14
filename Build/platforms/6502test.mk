HOSTCC = gcc
CC = cl65
CPP = cpp -nostdinc -undef -P
AS = ca65
AR = ar65
LD = ld65

ARCH = 6502

# Find what load address the kernel wants.
PROGLOAD = $(shell \
	(cat $(TOP)/Kernel/platform-$(PLATFORM)/config.h && echo PROGLOAD) | \
	cpp -E | tail -n1)

# CFLAGS used everwhere.
CFLAGS = \
	-t none \
	-D__STDC__ \
	$(COPTIMISATION)

COPTIMISATION = -O -Or

# Used when linking user mode executables (but not the kernel).
$(OBJ)/Applications/%: LDFLAGS = \
	--config $(TOP)/Build/platforms/$(PLATFORM).cfg

include $(TOP)/Build/cc65-rules.mk
include $(TOP)/Build/host-rules.mk

