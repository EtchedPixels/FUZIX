export CROSS_LD=ld86
export CROSS_CC = bcc
export CROSS_CCOPTS=-c -ansi -0 -O -I$(ROOT_DIR)/cpu-8086 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
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
