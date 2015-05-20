# $T is the current target name.

ifeq ($($T.abssrcs),)
$T.abssrcs := $(call absify, $($T.dir), $($T.srcs))
endif

ifeq ($($T.depsrcs),)
$T.depsrcs := $(filter %.c, $($T.abssrcs))
endif

ifeq ($($T.deps),)
$T.deps := $(patsubst %.c, $($T.objdir)/%.d, $($T.depsrcs))
endif

ifeq ($($T.objs),)
$T.objs := $(patsubst %, $($T.objdir)/%.rel, $(basename $($T.abssrcs)))
endif

.SECONDARY: $($T.objs)

-include $($T.deps)

# Variables referenced with $ are evaluated immediately.
# Variables references with $$ are evaluated at recipe execution time.
define patterns

$($T.objdir)/%.rel: $(TOP)/%.c
	@echo CC $$@
	@mkdir -p $$(dir $$@)
	$(hide) $(SDCC) \
		$(sdcc.cflags) $($($T.class).cflags) $($T.cflags) \
		$(sdcc.includes) $($($T.class).includes) $($T.includes) \
		$(sdcc.defines) $($($T.class).defines) $($T.defines) \
		-M $$< | sed -e '1s!^[^:]*!$$@!' \
		> $$(basename $$@).d
	$(hide) $(SDCC) \
		$(sdcc.cflags) $($($T.class).cflags) $($T.cflags) \
		$(sdcc.includes) $($($T.class).includes) $($T.includes) \
		$(sdcc.defines) $($($T.class).defines) $($T.defines) \
		-c -o $$@ $$<

$($T.objdir)/%.lib: $($T.objs)
	@echo AR $$@
	@mkdir -p $$(dir $$@)
	$(hide) rm -f $$@
	$(hide) $(SDAR) -rc $$@ $$^

$($T.objdir)/%.exe: $($T.objs) $($($T.class).extradeps) $($T.extradeps)
	@echo LINK $$@
	@mkdir -p $$(dir $$@)
	$(hide) $(SDCC) \
		$(sdcc.ldflags) $($($T.class).ldflags) $($T.ldflags) \
		-o $$(@:.exe=.ihx) $($T.objs)

endef
$(eval $(patterns))

