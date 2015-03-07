ifeq ($(PLATFORM),,)
$(error You must specify PLATFORM=something --- look in Build/platforms.)
endif

TOP = .
OBJ = $(TOP)/.obj/$(PLATFORM)
hide = @

LIBC = $(OBJ)/Library/libc.a
all: $(LIBC)

include $(TOP)/Build/platforms/$(PLATFORM).mk
include $(TOP)/Library/build.mk

clean:
	@echo CLEAN
	$(hide) rm -rf $(OBJ)

