export CROSS_AS=as68
export CROSS_LD= ld68
export CROSS_CC = cc68
export CROSS_CCOPTS= -c -I$(ROOT_DIR)/cpu-6303 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
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

