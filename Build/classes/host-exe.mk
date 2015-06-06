# In the macros below:
# $1 is the target name.
# Variables referenced with $ are evaluated immediately.
# Variables references with $$ are evaluated at recipe execution time.

define host-exe.rules

$1.objdir ?= $(OBJ)/host/$1
$1.result ?= $$($1.objdir)/$1

$1: $$($1.result)
.PHONY: $1

$1.abssrcs ?= $$(call absify, $$($1.dir), $$($1.srcs))
$1.depsrcs ?= $$(filter %.c, $$($1.abssrcs))
$1.deps ?= $$(patsubst %.c, $$($1.objdir)/%.d, $$($1.depsrcs))
$1.objs ?= $$(patsubst %, $$($1.objdir)/%.o, $$(basename $$($1.abssrcs)))

-include $$($1.deps)

$$($1.result): $$($1.objs)
	@echo HOSTLINK $$@
	@mkdir -p $$(dir $$@)
	$$(hide) gcc \
		$$(LDFLAGS) $$($1.ldflags) \
		-o $$@ $$^

$$($1.objdir)/%.o: $$(TOP)/%.c
	@echo HOSTCC $$@
	@mkdir -p $$(dir $$@)
	$$(hide) gcc \
		$$(host.cflags) $$($$($1.class).cflags) $$($1.cflags) \
		$$(host.includes) $$($$($1.class).includes) $$($1.includes) \
		$$(host.defines) $$($$($1.class).defines) $$($1.defines) \
		-MM -MF $$(basename $$@).d -MT $$(basename $$@).o $$<
	$$(hide) gcc \
		$$(host.cflags) $$($$($1.class).cflags) $$($1.cflags) \
		$$(host.includes) $$($$($1.class).includes) $$($1.includes) \
		$$(host.defines) $$($$($1.class).defines) $$($1.defines) \
		-c -o $$@ $$<

endef

