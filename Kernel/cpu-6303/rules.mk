export CROSS_AS=as6303
export CROSS_LD= ld6303
export CROSS_CC = cc6303
export CROSS_CCOPTS= -c -O -I$(ROOT_DIR)/cpu-6303 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
export CROSS_CC_SEG1=
export CROSS_CC_SEG2=
export CROSS_CC_SEG3=
# FIXME: discard
export CROSS_CC_SEGDISC=
export CROSS_CC_VIDEO=
export ASOPTS=
export ASMEXT = .s
export BINEXT = .o
export BITS=16

