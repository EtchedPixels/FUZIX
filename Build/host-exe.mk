# On entry, $T is the target name.

$T.objdir ?= $(OBJ)/host/$T
$T.exe ?= $($T.objdir)/$T

$T: $($T.exe)
.PHONY: $T

$T.abssrcs := $(call absify, $($T.dir), $($T.srcs))
$T.depsrcs := $(filter %.c, $($T.abssrcs))
$T.deps := $(patsubst %.c, $($T.objdir)/%.d, $($T.depsrcs))
$T.objs := $(patsubst %, $($T.objdir)/%.o, $(basename $($T.abssrcs)))
.SECONDARY: $($T.objs)

-include $($T.deps)

# Variables referenced with $ are evaluated immediately.
# Variables references with $$ are evaluated at recipe execution time.
define patterns

$($T.exe): $($T.objs)
	@echo HOSTLINK $T $$@
	@mkdir -p $$(dir $$@)
	$(hide) gcc \
		$(LDFLAGS) $($T.ldflags) \
		-o $$@ $$^

$($T.objdir)/%.o: $(TOP)/%.c
	@echo HOSTCC $T $$@
	@mkdir -p $$(dir $$@)
	$(hide) gcc \
		$(host.cflags) $($($T.class).cflags) $($T.cflags) \
		$(host.includes) $($($T.class).includes) $($T.includes) \
		$(host.defines) $($($T.class).defines) $($T.defines) \
		-MM -MF $$(basename $$@).d -MT $$(basename $$@).o $$<
	$(hide) gcc \
		$(host.cflags) $($($T.class).cflags) $($T.cflags) \
		$(host.includes) $($($T.class).includes) $($T.includes) \
		$(host.defines) $($($T.class).defines) $($T.defines) \
		-c -o $$@ $$<

endef
$(eval $(patterns))


