export CROSS_LD=pdp11-aout-ld
export CROSS_CC = pdp11-aout-gcc
# Do not use the Fedora gcc 5.3.1. It miscompiles stuff badly.
export CROSS_CCOPTS=-c -Os -fno-strict-aliasing -fomit-frame-pointer -fno-builtin -Wall -I$(ROOT_DIR)/cpu-pdp11 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
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
export BITS=16
export EXECFORMAT=16
