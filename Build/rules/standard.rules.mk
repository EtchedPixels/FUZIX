# Rules used by all platforms. This contains the standard boilerplate for
# creating object file lists etc.

define standard.rules

# $1 is the current target name (the same as in build classes).
# Variables referenced with $ are evaluated immediately.
# Variables references with $$ are evaluated at when the rules are
# instantiated.

$1.abssrcs ?= $$(call absify, $$($1.dir), $$($1.srcs))
$1.depsrcs ?= $$(filter %.c, $$($1.abssrcs))
$1.deps ?= $$(patsubst %.c, $$($1.objdir)/%.d, $$($1.depsrcs))
$1.extradeps ?=
$1.objs ?= \
	$$(patsubst %.c, $$($1.objdir)/%.$O, \
	$$(patsubst %.s, $$($1.objdir)/%.$O, \
	$$(patsubst %.S, $$($1.objdir)/%.$O, \
		$$($1.abssrcs))))

-include $$($1.deps)

endef

