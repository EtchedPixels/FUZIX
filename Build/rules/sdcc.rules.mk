# SDCC setup.

SDCC = sdcc
SDCPP = cpp -nostdinc -undef -P
SDAS = sdasz80
SDAR = sdar
PLATFORM_RULES = sdcc.rules


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


# Find what load address the kernel wants.

PROGLOAD = $(shell \
        (cat $(TOP)/Kernel/platform-$(PLATFORM)/config.h && echo PROGLOAD) | \
        cpp -E | tail -n1)

# CFLAGS used everywhere.

sdcc.cflags = \
	-m$(ARCH) \
	--std-c99 \
	--opt-code-speed --peep-asm \
	-Ddouble=float

# Used when linking user mode executables.

target-exe.ldflags ?=
target-exe.ldflags += \
	-m$(ARCH) \
	--nostdlib \
	--no-std-crt0 \
	--code-loc $(PROGLOAD) \
	--data-loc 0 \
	$(libc.result)

target-exe.extradeps ?=
target-exe.extradeps += $(libc.result)


# Fuzix' libc conflicts with the standard sdcc compiler library, which is a
# shame because there's bits we want in it. So we steal those bits into our
# own addon library.

libc-runtime.ext = $A
$(call build, libc-runtime, nop)
PLATFORM_EXTRA_LIBC = $(libc-runtime.result)

# Names of object files to pull out of the SDCC libc.
libc-runtime.objs = \
	divunsigned.rel divsigned.rel divmixed.rel modunsigned.rel modsigned.rel \
	modmixed.rel mul.rel mulchar.rel heap.rel memmove.rel strcpy.rel strlen.rel \
	abs.rel __sdcc_call_hl.rel crtenter.rel setjmp.rel _atof.rel _schar2fs.rel \
	_sint2fs.rel _slong2fs.rel _uchar2fs.rel _uint2fs.rel _ulong2fs.rel \
	_fs2schar.rel _fs2sint.rel _fs2slong.rel _fs2uchar.rel _fs2uint.rel \
	_fs2ulong.rel _fsadd.rel _fsdiv.rel _fsmul.rel _fssub.rel _fseq.rel \
	_fsgt.rel _fslt.rel _fsneq.rel fabsf.rel frexpf.rel ldexpf.rel expf.rel \
	powf.rel sincosf.rel sinf.rel cosf.rel logf.rel log10f.rel sqrtf.rel \
	tancotf.rel tanf.rel cotf.rel asincosf.rel asinf.rel acosf.rel atanf.rel \
	atan2f.rel sincoshf.rel sinhf.rel coshf.rel tanhf.rel floorf.rel \
	ceilf.rel modff.rel _divslong.rel _modslong.rel _modulong.rel \
	_divulong.rel _mullong.rel _mullonglong.rel _divslonglong.rel \
	_divulonglong.rel _modslonglong.rel _modulonglong.rel _ltoa.rel \
	abs.rel labs.rel _strcat.rel _strchr.rel _strcmp.rel _strcspn.rel \
	_strncat.rel _strncmp.rel strxfrm.rel _strncpy.rel _strpbrk.rel \
	_strrchr.rel _strspn.rel _strstr.rel _strtok.rel _memchr.rel _memcmp.rel \
	_memcpy.rel _memset.rel _itoa.rel _ltoa.rel atoi.rel \

# Names of source files from Fuzix's libc that we don't want to compile.
libc-functions.omit = \
	memcmp.c atoi.c memcpy.c strcat.c memset.c strncat.c strchr.c xitoa.c \
	strrchr.c ltoa.c strcmp.c strncpy.c strtok.c strncmp.c memchr.c strcspn.c

$(libc-runtime.result): $(SDCC_LIBS)/$(ARCH).lib
	@echo LIBRUNTIME $@
	@mkdir -p $(libc-runtime.objdir)
	$(hide) rm -f $(libc-runtime.objdir)/*.rel
	$(hide) (cd $(libc-runtime.objdir) \
		&& $(SDAR) x $< $(libc-runtime) \
		&& $(SDAR) cq $(abspath $@) $(libc-runtime.objs))


# This is the macro which is appended to target build classes; it contains all
# the sdcc-specific bits of the build rules.

define sdcc.rules

# $1 is the current target name (the same as in build classes).
# Variables referenced with $ are evaluated immediately.
# Variables references with $$ are evaluated at when the rules are
# instantiated.

# Invoke standard rules.

$(call standard.rules,$1)

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

