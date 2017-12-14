export CROSS_AS=ca65
export CROSS_LD=cl65
export CROSS_CC=cl65
export CROSS_CCOPTS=--cpu 65c02 -c -O -Os -Or -t none -I$(ROOT_DIR)/cpu-65c816 -I$(ROOT_DIR)/cpu-6502 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
#
#	It really doesn't matter how we map the segments as it's one binary
#	with no banking or tricks. The only exception is the discard area
#	so we can turn it into buffers
#
export CROSS_CC_SEG1=--code-name CODE
export CROSS_CC_SEG2=--code-name CODE
export CROSS_CC_SEG3=--code-name CODE
export CROSS_CC_SYS1=--code-name CODE
export CROSS_CC_SYS2=--code-name CODE
export CROSS_CC_SYS3=--code-name CODE
export CROSS_CC_SYS4=--code-name CODE
export CROSS_CC_SYS5=--code-name CODE
export CROSS_CC_VIDEO=--code-name CODE
export CROSS_CC_SEGDISC=--code-name DISCARD --rodata-name DISCARDDATA
export ASMEXT = .s
export BINEXT = .o
export BITS=16
