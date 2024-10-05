export CROSS_LD=ia16-elf-ld
export CROSS_CC = ia16-elf-gcc
export CROSS_CCOPTS=-Os -I$(ROOT_DIR)/cpu-8086 -I$(ROOT_DIR)/platform/platform-$(TARGET) -I$(ROOT_DIR)/include -c
export CROSS_AS=$(CROSS_CC) $(CROSS_CCOPTS)
export CROSS_CC_SEG1=
export CROSS_CC_SEG2=
export CROSS_CC_SEG3=
# Fixme: we should split discard off
export CROSS_CC_SEGDISC=
export CROSS_CC_VIDEO=
export CROSS_CC_FONT=
export CROSS_CC_NETWORK=
export ASOPTS=
export ASMEXT = .S
export BINEXT = .o
export BITS=16
export EXECFORMAT=16