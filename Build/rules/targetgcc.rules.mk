# Build rules for gcc-based targets.
# This requires you to have set TARGETCC, TARGETLD etc.

PLATFORM_RULES = targetgcc.rules


libc.ld = -L$(dir $(libc.result)) -lc

# Flags used everywhere.

targetgcc.cflags += \
	-g \
	-Wall \
	-Werror=implicit-function-declaration \
	--short-enums \
	-Os

targetgcc.ldflags += \
	--gc-sections \
	-g

targetgcc.asflags += \
	-g


# Used when linking user mode executables.

target-exe.extradeps += \
	$(libc.result) \
	$(TOP)/Build/platforms/$(PLATFORM).ld

target-exe.ldflags += \
	-T $(TOP)/Build/platforms/$(PLATFORM).ld \
	--relax

# Used when linking kernel images.

kernel-elf.extradeps += \
	$(TOP)/Kernel/platform-$(PLATFORM)/$(PLATFORM).ld

kernel-elf.ldflags += \
	-T $(TOP)/Kernel/platform-$(PLATFORM)/$(PLATFORM).ld \
	--relax

# This is the macro which is appended to target build classes; it contains all
# the targetgcc-specific bits of the build rules.

define targetgcc.rules

# $1 is the current target name (the same as in build classes).
# Variables referenced with $ are evaluated immediately.
# Variables references with $$ are evaluated at when the rules are
# instantiated.

# Invoke standard rules.

$(call standard.rules,$1)

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

# Builds an ordinary .S file.

$$($1.objdir)/%.o: $(TOP)/%.S
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
	$$(if $$(filter %.$O, $$($1.objs)),$(hide) $(TARGETAR) -rc $$@ $$(filter %.$O, $$($1.objs)))
	$$(if $$(filter %.$A, $$($1.objs)), $(hide) $(TARGETAR) -rc $$@ $$(dir $$@)/*.$O)

endif

# Builds a target executable.

ifneq ($$(filter %.exe, $$($1.result)),)

# Locate the libgcc used by this target.

$1.libgcc ?= $(shell $(TARGETCC) --print-libgcc)

$$($1.result): $$($1.objs) $(crt0.result) \
		$$($$($1.class).extradeps) $$($1.extradeps) $$($1.libgcc)
	@echo LINK $$@
	@mkdir -p $$(dir $$@)
	$(hide) $(TARGETLD) \
		$$(targetgcc.ldflags) $$($$($1.class).ldflags) $$($1.ldflags) \
		-o $$@.elf \
		--start-group \
		$$($1.objs) $(crt0.result) $(libc.ld) \
		-L$$(dir $$($1.libgcc)) -lgcc \
		--end-group
	$(hide) $(TARGETOBJCOPY) \
		--output-target binary $$@.elf $$@

endif

# Builds a kernel image.

ifneq ($$(filter %.elf, $$($1.result)),)

$1.libgcc ?= $(shell $(TARGETCC) --print-libgcc)

$$($1.result): $$($1.objs) \
		$$($$($1.class).extradeps) $$($1.extradeps) $$($1.libgcc)
	@echo KERNEL $$@
	@mkdir -p $$(dir $$@)
	$(hide) $(TARGETLD) \
		$$(targetgcc.ldflags) $$($$($1.class).ldflags) $$($1.ldflags) \
		-o $$@ \
		-Map $$(@:elf=map) \
		--start-group \
		$$($1.objs) \
		-L$$(dir $$($1.libgcc)) -lgcc \
		--end-group

endif

endef


