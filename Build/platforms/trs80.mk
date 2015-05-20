$(call find-makefile)

O = rel
A = lib

# SDCC setup.

SDCC = sdcc
SDCPP = cpp -nostdinc -undef -P
SDAS = sdasz80
SDAR = sdar
ARCH = z80

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

# Which file contains the target rules for this platform.

PLATFORM_RULES = $(BUILD)/platforms/sdcc.rules.mk

# Find what load address the kernel wants.

PROGLOAD = $(shell \
        (cat $(TOP)/Kernel/platform-$(PLATFORM)/config.h && echo PROGLOAD) | \
        cpp -E | tail -n1)

# CFLAGS used everywhere.

sdcc.cflags = \
	-m$(ARCH) \
	--std-c99 \
	--opt-code-size \
	-Ddouble=float

# User-mode code can see the standard library.

target-lib.includes += -ILibrary/include
target-exe.includes += -ILibrary/include

# Used when linking user mode executables.

target-exe.ldflags += \
	-mz80 \
	--nostdlib \
	--no-std-crt0 \
	--code-loc $(PROGLOAD) \
	--data-loc 0 \
	$(libc.exe)

target-exe.extradeps += $(libc.exe)

# Fuzix' libc conflicts with the standard sdcc compiler library, which is a
# shame because there's bits we want in it. So we steal those bits into our
# own addon library.

libruntime.ext := $A
$(call build, libruntime, nop)
target-exe.ldflags += $(libruntime.exe)
target-exe.extradeps += $(libruntime.exe)

libruntime.objs := \
	divunsigned.rel divsigned.rel divmixed.rel modunsigned.rel modsigned.rel \
	modmixed.rel mul.rel mulchar.rel heap.rel memmove.rel strcpy.rel strlen.rel \
	abs.rel crtcall.rel crtenter.rel setjmp.rel _atof.rel _schar2fs.rel \
	_sint2fs.rel _slong2fs.rel _uchar2fs.rel _uint2fs.rel _ulong2fs.rel \
	_fs2schar.rel _fs2sint.rel _fs2slong.rel _fs2uchar.rel _fs2uint.rel \
	_fs2ulong.rel _fsadd.rel _fsdiv.rel _fsmul.rel _fssub.rel _fseq.rel \
	_fsgt.rel _fslt.rel _fsneq.rel fabsf.rel frexpf.rel ldexpf.rel expf.rel \
	powf.rel sincosf.rel sinf.rel cosf.rel logf.rel log10f.rel sqrtf.rel \
	tancotf.rel tanf.rel cotf.rel asincosf.rel asinf.rel acosf.rel atanf.rel \
	atan2f.rel sincoshf.rel sinhf.rel coshf.rel tanhf.rel floorf.rel \
	ceilf.rel modff.rel _divslong.rel _modslong.rel _modulong.rel \
	_divulong.rel _mullong.rel _mullonglong.rel _divslonglong.rel \
	_divulonglong.rel _modslonglong.rel _modulonglong.rel _ltoa.rel _atoi.rel \
	abs.rel labs.rel _strcat.rel _strchr.rel _strcmp.rel _strcspn.rel \
	_strncat.rel _strncmp.rel strxfrm.rel _strncpy.rel _strpbrk.rel \
	_strrchr.rel _strspn.rel _strstr.rel _strtok.rel _memchr.rel _memcmp.rel \
	_memcpy.rel _memset.rel _itoa.rel _ltoa.rel \


$(libruntime.exe): $(SDCC_LIBS)/$(ARCH).lib $(MAKEFILE)
	@echo LIBRUNTIME $@
	@mkdir -p $(libruntime.objdir)
	$(hide) rm -f $(libruntime.objdir)/*.rel
	$(hide) (cd $(libruntime.objdir) \
		&& $(AR) x $< $(libruntime_objects) \
		&& $(AR) q $(abspath $@) $(libruntime.objs))

