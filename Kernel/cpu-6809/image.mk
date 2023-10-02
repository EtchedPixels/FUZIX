fuzix.bin: target $(OBJS) tools/decbdragon tools/decb-image tools/visualize6809 tools/diskpad
	+make -C platform-$(TARGET) image
	tools/visualize6809 < fuzix.map

