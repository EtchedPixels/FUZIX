ifeq ($(PLATFORM),,)
$(error You must specify PLATFORM=something --- look in Build/platforms.)
endif

TOP = .
OBJ = $(TOP)/.obj/$(PLATFORM)
hide = @

all: $(LIBC)

include $(TOP)/Build/platforms/$(PLATFORM).mk
include $(TOP)/Library/build.mk
include $(TOP)/Applications/build.mk

clean:
	@echo CLEAN
	$(hide) rm -rf $(OBJ)

