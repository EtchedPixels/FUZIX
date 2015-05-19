# On entry, $1 is the target name.

$1.objdir ?= $(OBJ)/$(PLATFORM)/$1
$1.exe ?= $($1.objdir)/$1.$A

$1: $($1.exe)
.PHONY: $1

$(eval $1.cflags += -I$(TOP)/Library/include)

include $(PLATFORM_RULES)

