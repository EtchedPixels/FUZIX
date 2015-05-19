create-rules = $(eval include $(BUILD)/$($1.class).mk)
$(foreach TARGET, $(TARGETS), $(call create-rules,$(TARGET)))

