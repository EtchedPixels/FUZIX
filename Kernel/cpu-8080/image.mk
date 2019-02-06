fuzix.bin: target $(OBJS)
	+$(MAKE) -C platform-$(TARGET) image
