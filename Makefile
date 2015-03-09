hide = @

ifeq ($(PLATFORM),,)
$(error You must specify PLATFORM=something --- look in Build/platforms.)
endif

TOP = .
OBJ = $(TOP)/.obj/$(PLATFORM)
HOSTOBJ = $(TOP)/.obj/host
PREFIX = /usr/local

FILESYSTEM = filesystem-$(PLATFORM).img

all: $(FILESYSTEM)

include $(TOP)/Build/platforms/$(PLATFORM).mk
include $(TOP)/Library/build.mk
include $(TOP)/Applications/build.mk
include $(TOP)/Standalone/build.mk
-include $(shell find $(OBJ) $(HOSTOBJ) -name "*.d")

clean:
	@echo CLEAN
	$(hide) rm -rf $(OBJ)

