#
#	ZX128 uses banked kernel images
#
export CPU = z80
CROSS_CCOPTS += --external-banker
#
# Tell the core code we are using the banked helpers
#
export BANKED=-banked
#
export CROSS_CC_SEG3=--codeseg CODE3
