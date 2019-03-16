tools/atariboot: tools/atariboot.c

fuzix.bin: target tools/atariboot $(OBJS)
	+make -C platform-$(TARGET) image
