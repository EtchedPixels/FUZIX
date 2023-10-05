
CROSS_CCOPTS += -Iplatform/platform-dragon-nx32/

vt.o: vt.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_VIDEO) $<

bank8k.o: CROSS_CCOPTS += -O0
