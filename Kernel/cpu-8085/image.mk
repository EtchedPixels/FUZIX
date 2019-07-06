
tools/8080map: tools/8080map.c

tools/ack2kernel: tools/ack2kernel.c

fuzix.bin: target $(OBJS) tools/8080map tools/ack2kernel
	+$(MAKE) -C platform-$(TARGET) image
