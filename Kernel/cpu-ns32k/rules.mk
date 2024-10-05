export CROSS_LD=ns32k-pc532-netbsd-ld
export CROSS_CC = ns32k-pc532-netbsd-gcc
export CROSS_CCOPTS=-c -Os -fno-strict-aliasing -fomit-frame-pointer -fno-builtin -Wall -I$(ROOT_DIR)/cpu-ns32k -I$(ROOT_DIR)/platform/platform-$(TARGET) -I$(ROOT_DIR)/include
export CROSS_AS=$(CROSS_CC) $(CROSS_CCOPTS)
export CROSS_CC_SEG1=
export CROSS_CC_SEG2=
export CROSS_CC_SEG3=
# Fixme: we should split discard off
export CROSS_CC_SEGDISC=
export CROSS_CC_VIDEO=
export CROSS_CC_NETWORK=
export ASOPTS=
export ASMEXT = .S
export BINEXT = .o
export BITS=32
export EXECFORMAT=32
