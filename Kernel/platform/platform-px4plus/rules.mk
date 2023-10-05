#
#	PX4 uses banked kernel images
#
CROSS_CCOPTS += --external-banker
#
# Tell the core code we are using the banked helpers
#
export BANKED=-banked
#
# We have two ROM images and we want to fold them together. The tool will
# put VIDEO etc in CODE3, so redefine SEG2 to be CODE3 so we only have
# 0 - common
# 1 - code/code1
# 3 - code2/code3/video/etc
#
export CROSS_CC_SEG2=--codeseg CODE3
export CROSS_CC_SEG3=--codeseg CODE3
