# Build rules for SDCC.

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

search_dirs = $(shell $(CC) --print-search-dirs -m$(ARCH))
SDCC_INCLUDES = $(patsubst %, -I%, \
		$(call find_section, $(search_dirs), includedir:))
SDCC_LIBS = $(firstword $(call find_section, $(search_dirs), libdir:))
SDCC_INCLUDE_PATH = $(patsubst %, -I%, $(SDCC_INCLUDES))

# Forget default suffix rules.
.SUFFIXES:

# Libc configuration.
WANT_FUZIX_FLOAT = n
WANT_FUZIX_STRINGLIB = n
WANT_FUZIX_NUMBERSLIB = n

# Object file and library extensions (sdcc use non-standard ones so it needs to
# be configurable).
O = rel
A = lib

# This rule is used as a hook to add behaviour before any source file is built.
# It's mostly used to set up the platform symlink.

.PHONY: paths
paths: ;

# Location of standard libraries.
LIBC = $(OBJ)/Library/libc.$A
CRT0 = $(OBJ)/Library/libs/fuzix/crt0.$(ARCH).$O
LIBRUNTIME = $(OBJ)/Library/libruntime.$A
.SECONDARY: $(CRT0) $(LIBC) $(LIBRUNTIME)

# The cleaned up version of the sdcc runtime library includes these objects
# only.
libruntime_objects = \
divunsigned.rel divsigned.rel divmixed.rel modunsigned.rel modsigned.rel \
modmixed.rel mul.rel mulchar.rel heap.rel memmove.rel strcpy.rel strlen.rel \
abs.rel crtcall.rel crtenter.rel setjmp.rel _atof.rel _schar2fs.rel \
_sint2fs.rel _slong2fs.rel _uchar2fs.rel _uint2fs.rel _ulong2fs.rel \
_fs2schar.rel _fs2sint.rel _fs2slong.rel _fs2uchar.rel _fs2uint.rel \
_fs2ulong.rel _fsadd.rel _fsdiv.rel _fsmul.rel _fssub.rel _fseq.rel _fsgt.rel \
_fslt.rel _fsneq.rel fabsf.rel frexpf.rel ldexpf.rel expf.rel powf.rel \
sincosf.rel sinf.rel cosf.rel logf.rel log10f.rel sqrtf.rel tancotf.rel \
tanf.rel cotf.rel asincosf.rel asinf.rel acosf.rel atanf.rel atan2f.rel \
sincoshf.rel sinhf.rel coshf.rel tanhf.rel floorf.rel ceilf.rel modff.rel \
_divslong.rel _modslong.rel _modulong.rel _divulong.rel _mullong.rel \
_mullonglong.rel _divslonglong.rel _divulonglong.rel _modslonglong.rel \
_modulonglong.rel _ltoa.rel _atoi.rel \
abs.rel labs.rel \
_strcat.rel _strchr.rel _strcmp.rel _strcspn.rel _strncat.rel _strncmp.rel \
strxfrm.rel _strncpy.rel _strpbrk.rel _strrchr.rel _strspn.rel _strstr.rel \
_strtok.rel _memchr.rel _memcmp.rel _memcpy.rel _memset.rel _itoa.rel _ltoa.rel \


# Create libclean by taking sdcc's standard library and chopping out
# everything which is duplicated in the Fuzix standard library.
$(LIBRUNTIME): $(SDCC_LIBS)/$(ARCH).lib $(MAKEFILE)
	@echo LIBRUNTIME $@
	$(hide) rm -rf $@.tmp
	$(hide) mkdir -p $@.tmp
	$(hide) (cd $@.tmp && $(AR) x $< $(libruntime_objects))
	$(hide) $(AR) q $@ $(patsubst %, $@.tmp/%, $(libruntime_objects))

# Assembly files which need to be preprocessed --- run through cpp first.
$(OBJ)/%.$O: $(TOP)/%.S |paths
	@echo AS $@
	@mkdir -p $(dir $@)
	$(hide) $(CPP) $(INCLUDES) $(SDCC_INCLUDE_PATH) $(DEFINES) \
		-MM -MF $(basename $@).d -MT $@ $<
	$(hide) $(CPP) $(INCLUDES) $(SDCC_INCLUDE_PATH) $(DEFINES) \
		-o $(basename $@).s $<
	$(hide) $(AS) $(ASFLAGS) $(INCLUDES) $(DEFINES) -c -o $@ $(basename $@).s

# Likewise, for dynamically generated assembly files.
$(OBJ)/%.$O: $(OBJ)/%.S |paths
	@echo AS $@
	@mkdir -p $(dir $@)
	$(hide) $(CPP) $(INCLUDES) $(SDCC_INCLUDE_PATH) $(DEFINES) \
		-MM -MF $(basename $@).d -MT $@ $<
	$(hide) $(CPP) $(INCLUDES) $(SDCC_INCLUDE_PATH) $(DEFINES) \
		-o $(basename $@).s $<
	$(hide) $(AS) $(ASFLAGS) $(INCLUDES) $(DEFINES) -c -o $@ $(basename $@).s

# Ordinary C files.
$(OBJ)/%.$O: $(TOP)/%.c |paths
	@echo CC $@
	@mkdir -p $(dir $@)
	$(hide) $(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) \
		-M $< | sed -e '1s!^[^:]*!$@!' > $(basename $@).d
	$(hide) $(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c -o $@ $<

# Dynamically generated C files.
$(OBJ)/%.$O: $(OBJ)/%.c |paths
	@echo CC $@
	@mkdir -p $(dir $@)
	$(hide) $(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) \
		-M $< | sed -e '1s!^[^:]*!$@!' > $(basename $@).d
	$(hide) $(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c -o $@ $<

# Tell make how to generate .d files, to stop it confusing them with
# executables.
$(OBJ)/%.d: $(OBJ)/%.$O ;

# Assembly files which don't need to be preprocessed.
$(OBJ)/%.$O: $(TOP)/%.s |paths
	@echo AS $@
	@mkdir -p $(dir $@)
	$(hide) $(AS) $(ASFLAGS) $(INCLUDES) $(DEFINES) -c -o $@ $<

# Libraries.
$(OBJ)/%.$A:
	@echo AR $@
	@mkdir -p $(dir $@)
	$(hide) rm -f $@
	$(hide) $(AR) -rc $@ $^

# Executables. Add object files by adding prerequisites.
$(OBJ)/Applications/%: $(HOSTOBJ)/Library/tools/binman $(CRT0) $(LIBC) $(LIBRUNTIME)
	@echo LINK $@
	@mkdir -p $(dir $@)
	$(hide) $(CC) $(LDFLAGS) $(INCLUDES) $(DEFINES) \
		-o $@.ihx $(wordlist 2, 999, $^)
	$(hide) makebin -p -s 65535 $@.ihx $@.bin
	$(hide) $< $(PROGBASE) $@.bin $@.map $@ > /dev/null

# Default PROGBASE assumes CP/M.
PROGBASE = 0x0100

# Ensure that various things know where their headers are.
$(OBJ)/Applications/%: INCLUDES += -I$(TOP)/Library/include
$(OBJ)/Library/%: INCLUDES += -I$(TOP)/Library/include
$(OBJ)/Library/%: INCLUDES += -I$(OBJ)/Library/libs/fuzix

# Z80 binaries (which we're assuming here) require massaging before they're
# valid Fuzix binaries. This tool does that.
$(HOSTOBJ)/Library/tools/binman: $(HOSTOBJ)/Library/tools/binman.o

