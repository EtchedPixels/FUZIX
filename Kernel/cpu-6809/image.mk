fuzix.bin: target $(OBJS) tools/decbdragon tools/decb-image
	+make -C platform-$(TARGET) image
