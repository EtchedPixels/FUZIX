export CROSS_AS=sdasrab
export CROSS_LD=sdldz80
export CROSS_CC=sdcc
export CROSS_CCOPTS=-c --std-sdcc99 --no-std-crt0 -m$(CPU) -I$(ROOT_DIR)/cpu-$(CPU) -I$(ROOT_DIR)/platform/platform-$(TARGET) -I$(ROOT_DIR)/include --max-allocs-per-node 30000 --opt-code-size --stack-auto --constseg CONST --fverbose-asm
export CROSS_CC_SEG2=--codeseg CODE2
# For now but we are overspilling in a lot of configs so will need a real CODE3
export CROSS_CC_SEG3=--codeseg CODE2
export CROSS_CC_SEGDISC=--codeseg DISCARD --constseg DISCARD
export CROSS_CC_FONT=--constseg FONT
export CROSS_CC_NETWORK=--codeseg CODE2
export CROSS_CC_VIDEO=--codeseg VIDEO
export CROSS_CC_SYS1=--codeseg CODE2
export CROSS_CC_SYS2=--codeseg CODE2
export CROSS_CC_SYS3=--codeseg CODE2
export CROSS_CC_SYS4=--codeseg CODE2
export CROSS_CC_SYS5=--codeseg CODE2
export ASOPTS=-plosff
export ASMEXT = .s
export BINEXT = .rel
export BITS=16
export EXECFORMAT=16
#
export LIBZ80=$(shell tools/findsdcc $(CPU))
