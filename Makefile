hide = @

need := 3.82
rightversion := $(filter $(need), \
	$(firstword $(sort $(MAKE_VERSION) $(need))))
ifeq ($(rightversion),)
$(error You must have make version $(need) or above)
endif

ifeq ($(PLATFORM),,)
$(error You must specify PLATFORM=something --- look in Build/platforms.)
endif

VERSION = "0.1"
SUBVERSION = "ac1"

TOP = .
OBJ = $(TOP)/.obj/$(PLATFORM)
HOSTOBJ = $(TOP)/.obj/host
PREFIX = /usr/local

# Where you want the filesystem image to go
FILESYSTEM = filesystem-$(PLATFORM).bin
all:: $(FILESYSTEM)

include $(TOP)/Build/platforms/$(PLATFORM).mk
include $(TOP)/Library/build.mk
include $(TOP)/Applications/build.mk
include $(TOP)/Standalone/build.mk
include $(TOP)/Kernel/build.mk
-include $(shell find $(OBJ) $(HOSTOBJ) -name "*.d" 2>/dev/null)

clean:
	@echo CLEAN
	$(hide) rm -rf $(OBJ)

