ifeq ($(TARGET),)
$(error You must define TARGET before calling object.)
endif

ifeq ($(CLASS),)
$(error You must define CLASS before calling object.)
endif

ifeq ($(DIR),)
$(error You must define DIR before calling object.)
endif

TARGETS += $(TARGET)

$(TARGET).class := $(CLASS)
$(TARGET).dir := $(DIR)

# No spaces here
T:=$(strip $(TARGET))
include $(BUILD)/$(CLASS).mk

TARGET :=
CLASS :=

