
fuzix.bin: target $(OBJS)
	+$(MAKE) -C platform-$(TARGET) image
	tools/visualize6800 <fuzix.map
