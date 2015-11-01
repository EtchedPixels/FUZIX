TOP = .
BUILD = $(TOP)/Build

OBJ = .obj
hide = @

CFLAGS = -g -Os
LDFLAGS = -g

VERSION = "0.1"
SUBVERSION = "ac1"

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

include $(BUILD)/bake.mk
include $(BUILD)/platforms/$(PLATFORM).mk
include $(BUILD)/rules/standard.rules.mk
include $(TOP)/Kernel/tools/build.mk
include $(TOP)/Standalone/build.mk
include $(TOP)/Library/build.mk
include $(TOP)/Library/tests/build.mk
include $(TOP)/Applications/build.mk
include $(TOP)/Applications/V7/cmd/sh/build.mk
include $(TOP)/Applications/levee/build.mk
include $(TOP)/Standalone/filesystem-src/build.mk

ifeq ($(wildcard $(TOP)/Kernel/platform-$(PLATFORM)/build.mk),)
$(warning (building the kernel for $(PLATFORM) isn't set up from here yet))
else
include $(TOP)/Kernel/platform-$(PLATFORM)/build.mk
all: kernel
endif

all: tests standalones filesystem
