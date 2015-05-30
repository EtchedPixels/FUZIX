# Build rules for gcc-based targets.

# Locate the libgcc used by this target.

libgcc = $(shell $(TARGETCC) --print-libgcc)

# Flags used everywhere.

targetgcc.cflags += \
	-g \
	-ffunction-sections \
	-fdata-sections \
	-fno-inline \
	--short-enums \
	-Os

targetgcc.ldflags += \
	--gc-sections \
	-g

targetgcc.asflags += \
	-g


# Used when linking user mode executables.

target-exe.extradeps += $(libc.result) $(libgcc) $(TOP)/Build/platforms/$(PLATFORM).ld
target-exe.ldflags += \
	-T $(TOP)/Build/platforms/$(PLATFORM).ld \
	--relax

# This is the macro which is appended to target build classes; it contains all
# the cc65-specific bits of the build rules.

define targetgcc.rules

# $1 is the current target name (the same as in build classes).
# Variables referenced with $ are evaluated immediately.
# Variables references with $$ are evaluated at when the rules are
# instantiated.

$1.abssrcs ?= $$(call absify, $$($1.dir), $$($1.srcs))
$1.depsrcs ?= $$(filter %.c, $$($1.abssrcs))
$1.deps ?= $$(patsubst %.c, $$($1.objdir)/%.d, $$($1.depsrcs))
$1.extradeps ?=
$1.objs ?= \
	$$(patsubst %.c, $$($1.objdir)/%.o, \
	$$(patsubst %.s, $$($1.objdir)/%.o, \
		$$($1.abssrcs)))

.SECONDARY: $$($1.objs)

-include $$($1.deps)

# Builds an ordinary C file.

$$($1.objdir)/%.o: $(TOP)/%.c
	@echo CC $$@
	@mkdir -p $$(dir $$@)
	$(hide) $(TARGETCC) \
		$$(targetgcc.cflags) $$($$($1.class).cflags) $$($1.cflags) \
		$$(targetgcc.includes) $$($$($1.class).includes) $$($1.includes) \
		$$(targetgcc.defines) $$($$($1.class).defines) $$($1.defines) \
                -MM -MF $$(basename $$@).d -MT $$@ $$<
	$(hide) $(TARGETCC) \
		$$(targetgcc.cflags) $$($$($1.class).cflags) $$($1.cflags) \
		$$(targetgcc.includes) $$($$($1.class).includes) $$($1.includes) \
		$$(targetgcc.defines) $$($$($1.class).defines) $$($1.defines) \
                -c -o $$@ $$<

# Builds an ordinary .s file.

$$($1.objdir)/%.o: $(TOP)/%.s
	@echo AS $$@
	@mkdir -p $$(dir $$@)
	$(hide) $(TARGETCC) \
		$$(targetgcc.asflags) $$($$($1.class).asflags) $$($1.asflags) \
                -c -o $$@ $$<

# Builds a dynamically generated .s file.

$$($1.objdir)/%.o: $$($1.objdir)/%.s
	@echo AS $$@
	@mkdir -p $$(dir $$@)
	$(hide) $(TARGETCC) \
		$$(targetgcc.asflags) $$($$($1.class).asflags) $$($1.asflags) \
                -c -o $$@ $$<

# Builds a library from object files and other library files. Additional
# libraries are merged in after the object files, from first-to-last order.

ifneq ($$(filter %.$A, $$($1.result)),)

$$($1.result): $$($1.objs) $$($$($1.class).extradeps) $$($1.extradeps)
	@echo AR $$@
	@mkdir -p $$(dir $$@)
	$(hide) rm -f $$(dir $$@)/*.$A $$@
	$(hide) $$(foreach lib, $$(filter %.$A, $$($1.objs)), \
		(cd $$(dir $$@) && $(TARGETAR) -x $$(abspath $$(lib))) && ) true
	$(hide) $(TARGETAR) -rc $$@ $$(filter %.$O, $$($1.objs))
	$$(if $$(filter %.$A, $$($1.objs)), $(hide) $(TARGETAR) -r $$@ $$(dir $$@)/*.$O)

endif

# Builds a target executable.

ifneq ($$(filter %.exe, $$($1.result)),)

$$($1.result): $$($1.objs) $(crt0.result) $(binman.result) \
		$$($$($1.class).extradeps) $$($1.extradeps)
	@echo LINK $$@
	@mkdir -p $$(dir $$@)
	$(hide) $(TARGETLD) \
		$$(cc65.ldflags) $$($$($1.class).ldflags) $$($1.ldflags) \
		-o $$@.elf \
		--start-group \
		$$($1.objs) $(crt0.result) $(libc.result) $(libgcc) \
		--end-group
	$(hide) $(TARGETOBJCOPY) \
		--output-target binary $$@.elf $$@

endif

endef


