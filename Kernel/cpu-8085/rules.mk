export ACK_ROOT=$(shell tools/findack)

export CROSS_AS=ack -mfuzix
export CROSS_LD=$(ACK_ROOT)/lib/ack/em_led
export CROSS_CC=ack
export CROSS_CCOPTS= -mfuzix -c -O4 -S -I$(ROOT_DIR)/cpu-$(CPU) -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
export CROSS_CC_SEG2=
export CROSS_CC_SEG3=
export CROSS_CC_SEG4=
export CROSS_CC_SEGDISC=-Ras=$(ROOT_DIR)/tools/discard85
export CROSS_CC_FONT=
export CROSS_CC_VIDEO=
export CROSS_CC_SYS1=
export CROSS_CC_SYS2=
export CROSS_CC_SYS3=
export CROSS_CC_SYS4=
export CROSS_CC_SYS5=
export ASOPTS=-c
export ASMEXT = .s
export BINEXT = .o
export BITS=16
