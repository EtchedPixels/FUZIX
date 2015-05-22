# On entry, $T is the target name.

define nop.rules

$1.objdir = $$(OBJ)/$$(PLATFORM)/$1

ifeq ($$($1.ext),)
$$(error You must define $1.ext to use nop)
endif

$1.exe += $$($1.objdir)/$1.$$($1.ext)

$1: $$($1.exe)
.PHONY: $1

endef
