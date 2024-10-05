fuzix.bin: target $(OBJS)
	+make -C platform/platform-$(TARGET) image
	tools/visualize6800 <fuzix.map
