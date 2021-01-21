fuzix.bin: target $(OBJS)
	+make -C platform-$(TARGET) image

