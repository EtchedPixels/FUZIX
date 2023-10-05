export CROSS_AS=m6809-unknown-as
export CROSS_LD=m6809-unknown-ld
export CROSS_CC=m6809-unknown-gcc
#export CROSS_CCOPTS=-Wall -O2 -I$(ROOT_DIR)/cpu-6809 #-I$(ROOT_DIR)/platform/platform-$(TARGET) -I$(ROOT_DIR)/include
export CROSS_CCOPTS=-c -Wall -Os -msoft-reg-count=0 -mfar-stack-param -I$(ROOT_DIR)/cpu-6809 -I$(ROOT_DIR)/platform/platform-$(TARGET) -I$(ROOT_DIR)/include
# Workaround for gcc6809 bug - register copy propagation issue
CROSS_CCOPTS += -fno-cprop-registers
export CROSS_CC_SEG1=-mcode-section=.text -mfar-code-page=1
export CROSS_CC_SEG2=-mcode-section=.text2 -mfar-code-page=2
#Given the compactness we don't need a CODE3 segment really
export CROSS_CC_SEG3=-mcode-section=.text2 -mfar-code-page=2
export CROSS_CC_SEGDISC=-mcode-section=.discard -mdata-section=.discard -mfar-code-page=3
export CROSS_CC_VIDEO=-mcode-section=.video -mdata-section=.videodata -mfar-code-page=4
export CROSS_CC_FONT=-mcode-section=.video -mdata-section=.videodata -mfar-code-page=4
export CROSS_CC_SYS1=-mcode-section=.text2 -mfar-code-page=2
export CROSS_CC_SYS2=-mcode-section=.text2 -mfar-code-page=2
export CROSS_CC_SYS3=-mcode-section=.text2 -mfar-code-page=2
export CROSS_CC_SYS4=-mcode-section=.text2 -mfar-code-page=2
export CROSS_CC_SYS5=-mcode-section=.text2 -mfar-code-page=2
export CROSS_CC_NETWORK=-mcode-section=.text2 -mfar-code-page=2
export ASOPTS=
export ASMEXT = .s
export BINEXT = .o
export BITS=16
export EXECFORMAT=16

tools/decbdragon: tools/decbdragon.c
tools/decb-image: tools/decb-image.c
tools/decb-mooh: tools/decb-mooh.c
tools/diskpad: tools/diskpad.c
