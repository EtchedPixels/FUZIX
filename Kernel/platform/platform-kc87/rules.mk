#
#	KC87 uses banked kernel images
#
CROSS_CCOPTS += --external-banker
#
# Tell the core code we are using the banked helpers
#
export BANKED=-banked
#
export CROSS_CC_SEG1=--codeseg CODE
export CROSS_CC_SEG3=--codeseg CODE1
export CROSS_CC_SEG4=--codeseg CODE3
export CROSS_CC_SYS1=--codeseg CODE4
export CROSS_CC_SYS2=--codeseg CODE4
export CROSS_CC_SYS3=--codeseg CODE2
export CROSS_CC_SYS4=--codeseg CODE
export CROSS_CC_SYS5=--codeseg CODE1
export CROSS_CC_SEGDISC=--codeseg DISCARD --constseg DISCARD
