
CROSS_CCOPTS += -Iplatform/platform-dragon-nx32/

vt.o: vt.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_VIDEO) $<

mm/bank8k.o: CROSS_CCOPTS += -O0

ifneq ($(CI_TESTING),)
CROSS_CCOPTS += -DCRT9128SIM=1 -DBOOT_TTY=514
endif
