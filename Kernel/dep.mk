# Automatic header dependencies with GCC

DEP_CFLAGS += -Wp,-MD,$(@D)/$(@F).d -MT $(@D)/$(@F)

ifneq "$(MAKECMDGOALS)" "clean"
    DEP_FILES := $(shell find . -name '*'.d -printf '%P\n' | sort)
    $(if $(DEP_FILES),$(eval include $(DEP_FILES)))
endif
