tools/visualize6800: tools/visualize6800.c

tools/pack85: tools/pack85.c

tools/doubleup: tools/doubleup.c

fuzix.bin: target $(OBJS) tools/pack85 tools/visualize6800 tools/doubleup
	+$(MAKE) -C platform/platform-$(TARGET) image
	(cd platform/platform-$(TARGET); ../../tools/visualize6800) <fuzix.map
