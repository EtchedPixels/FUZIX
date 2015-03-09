ifeq ($(PLATFORM),,)
$(error You must specify PLATFORM=something --- look in Build/platforms.)
endif

TOP = .
OBJ = $(TOP)/.obj/$(PLATFORM)
HOSTOBJ = $(TOP)/.obj/host
hide = @

all: $(OBJ)/filesystem.img

include $(TOP)/Build/platforms/$(PLATFORM).mk
include $(TOP)/Library/build.mk
include $(TOP)/Applications/build.mk
include $(TOP)/Standalone/build.mk

clean:
	@echo CLEAN
	$(hide) rm -rf $(OBJ)

