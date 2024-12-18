tools/visualize_splitid: tools/visualize_splitid.c

fuzix.bin: target $(OBJS) tools/visualize_splitid
	+$(MAKE) -C platform/platform-$(TARGET) image
	tools/visualize_splitid <fuzix.map
#	tools/visualize <fuzix.map
