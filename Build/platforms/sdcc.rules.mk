# Utility functions to extract a named section from the data returned by
# sdcc --print-search-dirs; this is chunked into sections, each starting
# with a keyword followed by a colon, everything separated by whitespace.

# Truncates a list immediately after a named section header.
trim_before_named_section = \
	$(if $(filter $2, $(firstword $1)), \
		$(wordlist 2, 999, $1), \
			$(call trim_before_named_section, $(wordlist 2, 999, $1), $2))

# Truncates a list immediately *before* any section header.
trim_after_section = \
	$(if $(filter %:, $(firstword $1)), \
		$2, \
		$(if $(strip $1), \
			$(call trim_after_section, $(wordlist 2, 999, $1), $(firstword $1) $2), \
			$2))

# Return the contents of a named section.
find_section = \
        $(call trim_after_section, $(call trim_before_named_section, $1, $2))

# Fetch information about the SDCC installation: we'll need to know
# where SDCC's libraries and headers are later.

search_dirs = $(shell $(SDCC) --print-search-dirs -m$(ARCH))
SDCC_INCLUDES = $(patsubst %, -I%, \
                $(call find_section, $(search_dirs), includedir:))
SDCC_LIBS = $(firstword $(call find_section, $(search_dirs), libdir:))
SDCC_INCLUDE_PATH = $(patsubst %, -I%, $(SDCC_INCLUDES))

WANT_FUZIX_STRINGLIB = n

# This is the macro which is appended to target build classes; it contains all
# the sdcc-specific bits of the build rules.

define sdcc.rules

# $1 is the current target name (the same as in build classes).
# Variables referenced with $ are evaluated immediately.
# Variables references with $$ are evaluated at when the rules are
# instantiated.

$1.abssrcs ?= $$(call absify, $$($1.dir), $$($1.srcs))
$1.depsrcs ?= $$(filter %.c, $$($1.abssrcs))
$1.deps ?= $$(patsubst %.c, $$($1.objdir)/%.d, $$($1.depsrcs))
$1.objs ?= \
	$$(patsubst %.c, $$($1.objdir)/%.rel, \
	$$(patsubst %.s, $$($1.objdir)/%.rel, \
		$$($1.abssrcs)))

.SECONDARY: $$($1.objs)

-include $$($1.deps)

# Builds an ordinary C file.

$$($1.objdir)/%.rel: $(TOP)/%.c
	@echo CC $$@
	@mkdir -p $$(dir $$@)
	$$(hide) $(SDCC) \
		$$(sdcc.cflags) $$($$($1.class).cflags) $$($1.cflags) \
		$$(sdcc.includes) $$($$($1.class).includes) $$($1.includes) \
		$$(sdcc.defines) $$($$($1.class).defines) $$($1.defines) \
		-M $$< | sed -e '1s!^[^:]*!$$@!' \
		> $$(basename $$@).d
	$$(hide) $(SDCC) \
		$$(sdcc.cflags) $$($$($1.class).cflags) $$($1.cflags) \
		$$(sdcc.includes) $$($$($1.class).includes) $$($1.includes) \
		$$(sdcc.defines) $$($$($1.class).defines) $$($1.defines) \
		-c -o $$@ $$<

# Builds an ordinary .s file.

$$($1.objdir)/%.rel: $(TOP)/%.s
	@echo AS $$@
	@mkdir -p $$(dir $$@)
	$(hide) $(SDAS) \
		$$(sdcc.asflags) $$($$($1.class).asflags) $$($1.asflags) \
		-c -o $$@ $$<

# Builds a dynamically generated .s file.

$$($1.objdir)/%.rel: $$($1.objdir)/%.s
	@echo AS $$@
	@mkdir -p $$(dir $$@)
	$(hide) $(SDAS) \
		$$(sdcc.asflags) $$($$($1.class).asflags) $$($1.asflags) \
		-c -o $$@ $$<

# Builds a library from object files and other library files. Additional
# libraries are merged in after the object files, from first-to-last order.

ifneq ($$(filter %.lib, $$($1.result)),)

$$($1.result): $$($1.objs) $$($$($1.class).extradeps) $$($1.extradeps)
	@echo AR $$@
	@mkdir -p $$(dir $$@)
	$(hide) rm -f $$(dir $$@)/*.rel $$@
	$(hide) $$(foreach lib, $$(filter %.lib, $$($1.objs)), \
		(cd $$(dir $$@) && $(SDAR) -x $$(abspath $$(lib))) && ) true
	$(hide) $(SDAR) -rc $$@ $$(filter %.rel, $$($1.objs))
	$$(if $$(filter %.lib, $$($1.objs)), $(hide) $(SDAR) -r $$@ $$(dir $$@)/*.rel)

endif

# Builds a target executable.

ifneq ($$(filter %.exe, $$($1.result)),)

$$($1.result): $$($1.objs) $(crt0.result) $(binman.result) \
		$$($$($1.class).extradeps) $$($1.extradeps)
	@echo LINK $$@
	@mkdir -p $$(dir $$@)
	$$(hide) $(SDCC) \
		$$(sdcc.ldflags) $$($$($1.class).ldflags) $$($1.ldflags) \
		-o $$(@:.exe=.ihx) $(crt0.result) $$($1.objs)
	$$(hide) makebin -p -s 65535 $$(@:.exe=.ihx) $$(@:.exe=.bin)
	$$(hide) $(binman.result) $(PROGLOAD) $$(@:.exe=.bin) $$(@:.exe=.map) $$@ > /dev/null

endif

endef

