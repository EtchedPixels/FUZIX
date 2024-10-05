tools/visualize6502: tools/visualize6502.c

fuzix.bin: target $(OBJS) tools/visualize6502
	+make -C platform/platform-$(TARGET) image
	tools/visualize6502 <fuzix.map
