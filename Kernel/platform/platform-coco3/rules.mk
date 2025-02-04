# Override flags for certain objects

# Note: mm/bank16k.o doesn't end up in mm/ as no -o option is used.  I think
# this is because some archs' compilers don't work with it?  This also means
# the dependency is wrong, and they'll get rebuilt every time.

mm/bank16k.o: CROSS_CC_SEG2 = -O0
timer.o: CROSS_CC_SEG3 = -O0
usermem.o: CROSS_CC_SEG3 = $(CROSS_CC_VIDEO) -O0
