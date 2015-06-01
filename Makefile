TOP = .
BUILD = $(TOP)/Build

OBJ = .obj
hide = @

CFLAGS = -g -Os
LDFLAGS = -g

host.cflags = $(CFLAGS)
host.ldflags = $(LDFLAGS)

all:

ifeq ($(PLATFORM),)
$(error You must specify a PLATFORM)
endif

# Export these files from the build system.

chmem.result = bin/chmem
size.result = bin/size
mkfs.result = bin/mkfs
fsck.result = bin/fsck
ucp.result = bin/ucp

filesystem.result = filesystem-$(PLATFORM).img

include $(BUILD)/_head.mk
include $(BUILD)/platforms/$(PLATFORM).mk
include $(BUILD)/standard.rules.mk
include $(TOP)/Standalone/build.mk
include $(TOP)/Library/build.mk
include $(TOP)/Applications/build.mk
include $(TOP)/Applications/V7/cmd/sh/build.mk
include $(TOP)/Applications/levee/build.mk
include $(TOP)/Standalone/filesystem-src/build.mk
include $(BUILD)/_tail.mk

all: standalones filesystem
