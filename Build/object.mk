ifeq ($(TARGET),)
$(error You must define TARGET before calling object)
endif

ifeq ($(CLASS),)
$(error You must define CLASS before calling object)
endif

ifeq ($(DIR),)
$(error You must define DIR before calling object)
endif

include $(BUILD)/$(CLASS).setup.mk

TARGET :=
CLASS :=

