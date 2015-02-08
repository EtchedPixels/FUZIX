fuzix.bin: target $(OBJS) tools/decbdragon
	+make -C platform-$(TARGET) image
