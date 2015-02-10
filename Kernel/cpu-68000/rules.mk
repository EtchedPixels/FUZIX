export CROSS_LD=m68k-atari-mint-ld
export CROSS_CC = m68k-atari-mint-gcc
export CROSS_CCOPTS=-c -fno-builtin -Wall -Os -m68000 -mshort -I$(ROOT_DIR)/cpu-68000 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
export CROSS_AS=$(CROSS_CC) $(CROSS_CCOPTS) #-Wa,-M
export CROSS_CC_SEG1=
export CROSS_CC_SEG2=
export CROSS_CC_SEG3=
# Fixme: we should split discard off
export CROSS_CC_SEGDISC=
export CROSS_CC_VIDEO=
export ASOPTS=
export ASMEXT = .S
export BINEXT = .o
export BITS=32
