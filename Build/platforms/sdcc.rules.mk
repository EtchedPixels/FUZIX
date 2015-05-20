# $1 is the current target name.

$1.abssrcs ?= $(call absify, $($1.dir), $($1.srcs))
$1.depsrcs ?= $(filter %.c, $($1.abssrcs))
$1.deps ?= $(patsubst %.c, $($1.objdir)/%.d, $($1.depsrcs))
$1.objs ?= $(patsubst %, $($1.objdir)/%.rel, $(basename $($1.abssrcs)))
.SECONDARY: $($1.objs)

-include $($1.deps)

# Variables referenced with $ are evaluated immediately.
# Variables references with $$ are evaluated at recipe execution time.
define patterns

$($1.objdir)/%.rel: $(TOP)/%.c
	@echo CC $$@
	@mkdir -p $$(dir $$@)
	$(hide) $(SDCC) \
		$(SDCCCFLAGS) \
		$(PLATFORM_CFLAGS) \
		$($1.cflags) \
		$(INCLUDES) \
		$($1.includes) \
		$(DEFINES) \
		$($1.defines) \
		-M $$< | sed -e '1s!^[^:]*!$$@!' \
		> $$(basename $$@).d
	$(hide) $(SDCC) \
		$(SDCCCFLAGS) \
		$(PLATFORM_CFLAGS) \
		$($1.cflags) \
		$(INCLUDES) \
		$($1.includes) \
		$(DEFINES) \
		$($1.defines) \
		-c -o $$@ $$<

$($1.objdir)/%.lib: $($1.objs)
	@echo AR $$@
	@mkdir -p $$(dir $$@)
	$(hide) rm -f $$@
	$(hide) $(SDAR) -rc $$@ $$^

endef
$(eval $(patterns))

