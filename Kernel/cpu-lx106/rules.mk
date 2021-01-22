export CROSS_LD = xtensa-lx106-elf-ld
export CROSS_CC = xtensa-lx106-elf-gcc
# Do not use the Fedora gcc 5.3.1. It miscompiles stuff badly.
export CROSS_CCOPTS=-c -g -Os -fno-strict-aliasing -fomit-frame-pointer -fno-builtin -Wall -I$(ROOT_DIR)/cpu-lx106 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include -mlongcalls -mforce-l32
export CROSS_AS=$(CROSS_CC) $(CROSS_CCOPTS)
export CROSS_CC_SEG1=
export CROSS_CC_SEG2=
export CROSS_CC_SEG3=
# Fixme: we should split discard off
export CROSS_CC_SEGDISC=
export CROSS_CC_VIDEO=
export ASOPTS=
export ASMEXT = .S
export BINEXT = .o
export BITS=lx106
