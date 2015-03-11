export CROSS_AS=ca65
export CROSS_LD=cl65
export CROSS_CC=cl65
export CROSS_CCOPTS=--all-fastcall -c -O -t none -I$(ROOT_DIR)/cpu-6502 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
#
# The 6502 compiler produces what is mostly threadcode and is quite determined
# that the runtime lives in the code segment. As we want the runtime in common
# memory we use SEG1/SEG2 names for all the kernel code.
#
export CROSS_CC_SEG1=--code-name SEG1
export CROSS_CC_SEG2=--code-name SEG2
# 6502 we need a real SEG3 to make it fit
export CROSS_CC_SEG3=--code-name SEG3
export CROSS_CC_VIDEO=--code-name SEG3
export CROSS_CC_SEGDISC=--code-name DISCARD --rodata-name DISCARDDATA
export ASMEXT = .s
export BINEXT = .o
export BITS=16
