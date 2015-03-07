CC = sdcc
CPP = cpp -nostdinc -undef -P
AS = sdasz80
AR = sdcclib

ARCH = z80

# CFLAGS used everwhere.
CFLAGS = \
	-mz80 \
	--std-c99 \
	--opt-code-size

# Extra CFLAGS used only for user space code.
USERCFLAGS = \
	-I$(TOP)/Library/include

include $(TOP)/Build/standard-rules.mk

