# On entry, $T is the target name.

define kernel-lib.rules

$1.objdir ?= $(OBJ)/$(PLATFORM)/$1
$1.result ?= $$($1.objdir)/$1.$A

$1: $$($1.result)
.PHONY: $1

$1.cflags += \
	-I$(TOP)/Kernel \
	-I$(TOP)/Kernel/dev \
	-I$(TOP)/Kernel/include \
	-I$(TOP)/Kernel/platform-$(PLATFORM) \
	-I$(TOP)/Kernel/cpu-$(ARCH)

$1.asflags += \
	-I$(TOP)/Kernel \
	-I$(TOP)/Kernel/dev \
	-I$(TOP)/Kernel/include \
	-I$(TOP)/Kernel/platform-$(PLATFORM) \
	-I$(TOP)/Kernel/cpu-$(ARCH)

$(call $(PLATFORM_RULES),$1)
endef
