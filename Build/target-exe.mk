# On entry, $T is the target name.

ifeq ($($T.objdir),)
$T.objdir := $(OBJ)/$(PLATFORM)/$T
endif

ifeq ($($T.exe),)
$T.exe := $($T.objdir)/$T.exe
endif

$T: $($T.exe)
.PHONY: $T

include $(PLATFORM_RULES)

