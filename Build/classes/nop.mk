# On entry, $T is the target name.

define nop.rules

$1.objdir ?= $$(OBJ)/$$(PLATFORM)/$1
$1.abssrcs ?= $$(call absify, $$($1.dir), $$($1.srcs))

ifeq ($$($1.ext),)
$$(error You must define $1.ext to use nop)
endif

$1.result ?= $$($1.objdir)/$1.$$($1.ext)

$1: $$($1.result)
.PHONY: $1

$$($1.result): $$($1.abssrcs)

endef
