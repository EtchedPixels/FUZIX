export CROSS_PATH=$(HOME)/tm4c129x/toolchain
export CROSS_PATH_BIN=$(CROSS_PATH)/bin
export CROSS_LD=$(CROSS_PATH_BIN)/arm-none-eabi-ld
export CROSS_CC=$(CROSS_PATH_BIN)/arm-none-eabi-gcc
export CROSS_CCOPTS=-g -c -Os -fno-strict-aliasing -fno-builtin -Wall -mcpu=cortex-m4 -mtune=cortex-m4 -march=armv7e-m+nofp -mthumb -I$(ROOT_DIR)/cpu-armm4 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
export CROSS_AS=$(CROSS_CC)
export CROSS_CC_SEG1=
export CROSS_CC_SEG2=
export CROSS_CC_SEG3=
# Fixme: we should split discard off
export CROSS_CC_SEGDISC=
export CROSS_CC_VIDEO=
export ASOPTS=-xassembler-with-cpp $(CROSS_CCOPTS)
export ASMEXT=.S
export BINEXT=.o
# export BITS=elf32
export BITS=32
