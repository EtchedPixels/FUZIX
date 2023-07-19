export CROSS_LD=riscv-unknown-elf-ld
export CROSS_CC=riscv-unknown-elf-gcc
export CROSS_CCOPTS=-g -c -Os -fno-strict-aliasing -fno-builtin -fno-stack-protector -Wall
CROSS_CCOPTS+= -march=rv32ima_zicsr -mabi=ilp32 -fdata-sections -ffunction-sections
CROSS_CCOPTS += -static-libgcc -I$(ROOT_DIR)/cpu-riscv32 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include -I$(FUZIX_ROOT)/Library/include/riscv32
export CROSS_AS=$(CROSS_CC)
export CROSS_CC_SEG1=
export CROSS_CC_SEG2=
export CROSS_CC_SEG3=
# Fixme: we should split discard off
export CROSS_CC_SEGDISC=
export CROSS_CC_VIDEO=
export CROSS_CC_FONT=
export CROSS_CC_NETWORK=
export ASOPTS=-xassembler-with-cpp $(CROSS_CCOPTS)
export ASMEXT=.S
export BINEXT=.o
# export BITS=elf32
export BITS=32
export EXECFORMAT=32
