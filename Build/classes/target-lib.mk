# On entry, $T is the target name.

define target-lib.rules

$1.objdir ?= $(OBJ)/$(PLATFORM)/$1
$1.result ?= $$($1.objdir)/$1.$A

$1: $$($1.result)
.PHONY: $1

$1.cflags += -I$(TOP)/Library/include

$(call $(PLATFORM_RULES),$1)
endef
