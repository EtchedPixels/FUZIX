fuzix.bin: target $(OBJS)
	+make -C platform/platform-$(TARGET) image
