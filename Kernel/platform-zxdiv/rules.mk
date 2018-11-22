#
#	ZX128 uses banked kernel images
#
CROSS_CCOPTS += --external-banker
#
# Tell the core code we are using the banked helpers
#
export BANKED=-banked
#
export CROSS_CC_SEG1=--codeseg CODE2
export CROSS_CC_SEG3=--codeseg CODE1
export CROSS_CC_SEG4=--codeseg CODE1
export CROSS_CC_SYS1=--codeseg CODE1
export CROSS_CC_SYS2=--codeseg CODE1
export CROSS_CC_SYS3=--codeseg CODE1
export CROSS_CC_SYS4=--codeseg CODE3
export CROSS_CC_SYS5=--codeseg CODE3
# The banking default is to put fonts in bank3. We don't want this so build
# our font as CONST so it lands where we want it
export CROSS_CC_FONT=--constseg CONST