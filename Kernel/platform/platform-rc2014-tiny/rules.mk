#
# Push the execve code high to make room
#
export CROSS_CC_SEG1=--codeseg CODE
export CROSS_CC_SEG2=--codeseg CODE2
export CROSS_CC_SEG3=--codeseg CODE
export CROSS_CC_VIDEO=--codeseg CODE2
#
export CROSS_CC_SYS1=--codeseg CODE
export CROSS_CC_SYS2=--codeseg CODE
export CROSS_CC_SYS3=--codeseg CODE
export CROSS_CC_SYS4=--codeseg COMMONMEM
export CROSS_CC_SYS5=--codeseg COMMONMEM
export CROSS_CC_SEGDISC=--codeseg DISCARD
#
CROSS_CCOPTS += --peep-file $(FUZIX_ROOT)/Kernel/cpu-z80/rst.peep
