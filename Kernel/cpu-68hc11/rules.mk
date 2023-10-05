export CROSS_AS=m6811-elf-as
export CROSS_LD= m6811-elf-ld
export CROSS_CC = m6811-elf-gcc
export CROSS_OBJCOPY = m6811-elf-objcopy
export CROSS_CCOPTS=-fomit-frame-pointer -mrelax -mshort -c -Wall -Os -I$(ROOT_DIR)/cpu-68hc11 -I$(ROOT_DIR)/platform/platform-$(TARGET) -I$(ROOT_DIR)/include
export CROSS_CC_SEG1=
export CROSS_CC_SEG2=
export CROSS_CC_SEG3=
# gcc expects this to be done by linker script
export CROSS_CC_SEGDISC=
export CROSS_CC_VIDEO=
export CROSS_CC_FONT=
export CROSS_CC_NETWORK=
export ASOPTS=
export ASMEXT = .s
export BINEXT = .o
export BITS=16
export EXECFORMAT=16
