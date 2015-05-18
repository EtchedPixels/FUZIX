$(foreach TARGET, $(TARGETS), $(eval CLASS:=$($(TARGET).class)) $(eval include $(BUILD)/$(CLASS).build.mk))

