TOP = .
BUILD = $(TOP)/Build

OBJ = .obj
hide = @

CFLAGS = -g -O
LDFLAGS = -g

host.cflags = $(CFLAGS)
host.ldflags = $(LDFLAGS)

all:

ifeq ($(PLATFORM),)
$(error You must specify a PLATFORM)
endif

include $(BUILD)/_head.mk
include $(BUILD)/platforms/$(PLATFORM).mk
include $(BUILD)/platforms/standard.rules.mk
include $(TOP)/Standalone/build.mk
include $(TOP)/Library/build.mk
include $(TOP)/Applications/build.mk
include $(TOP)/Applications/V7/cmd/sh/build.mk
include $(TOP)/Applications/levee/build.mk
include $(TOP)/Standalone/filesystem-src/build.mk
include $(BUILD)/_tail.mk

all: standalones filesystem
