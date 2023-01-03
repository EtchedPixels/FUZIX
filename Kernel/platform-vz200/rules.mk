#
#	Banked kernel images
#
CROSS_CCOPTS += --external-banker
#
# Tell the core code we are using the banked helpers
#
export BANKED=-banked
#
export CROSS_CC_SEG1=--codeseg CODE
export CROSS_CC_SEG3=--codeseg CODE1
export CROSS_CC_SEG4=--codeseg CODE
export CROSS_CC_SYS1=--codeseg CODE1
export CROSS_CC_SYS2=--codeseg CODE1
export CROSS_CC_SYS3=--codeseg CODE1
export CROSS_CC_SYS4=--codeseg CODE2
export CROSS_CC_SYS5=--codeseg CODE
export CROSS_CC_SEGDISC=--codeseg DISCARD --constseg DISCARD
export CROSS_CC_VIDEO=--codeseg CODE1
