# On entry, $T is the target name.

ifeq ($($T.objdir),)
$T.objdir := $(OBJ)/$(PLATFORM)/$T
endif

ifeq ($($T.ext),)
$(error You must define $T.ext to use nop)
endif

ifeq ($($T.exe),)
$T.exe := $($T.objdir)/$T.$($T.ext)
endif

$T: $($T.exe)
.PHONY: $T

