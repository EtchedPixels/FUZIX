fuzix.bin: target $(OBJS) tools/decbdragon tools/decb-image tools/visualize6809
	+make -C platform-$(TARGET) image
	tools/visualize6809 < fuzix.map

