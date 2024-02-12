tools/visualize6800: tools/visualize6800.c

tools/pack85: tools/pack85.c

fuzix.bin: target $(OBJS) tools/pack85 tools/visualize6800
	+$(MAKE) -C platform/platform-$(TARGET) image
	tools/visualize6800 <fuzix.map
