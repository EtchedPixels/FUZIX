
tools/visualize6800: tools/visualize6800.c

fuzix.bin: target $(OBJS) tools/visualize6800
	+make -C platform/platform-$(TARGET) image
	tools/visualize6800 <fuzix.map
