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

search_dirs = $(shell $(CC) --print-search-dirs -m$(ARCH))
SDCC_INCLUDES = $(call find_section, $(search_dirs), includedir:)
SDCC_LIBS = $(firstword $(call find_section, $(search_dirs), libdir:))

# Forget default suffix rules.
.SUFFIXES:

# Object file and library extensions (sdcc use non-standard ones so it needs to
# be configurable).
O = rel
A = lib

# Location of standard libraries.
LIBC = $(OBJ)/Library/libc.$A
CRT0 = $(OBJ)/Library/libs/fuzix/crt0.$(ARCH).$O
LIBCLEAN = $(OBJ)/Library/libclean.$A
.SECONDARY: $(CRT0) $(LIBC) $(LIBCLEAN)

# Create libclean by taking sdcc's standard library and chopping out
# everything which is duplicated in the Fuzix standard library.
$(LIBCLEAN): $(SDCC_LIBS)/$(ARCH).lib
	@echo LIBCLEAN $@
	@mkdir -p $(dir $@)
	$(hide) cp $< $@
	$(hide) $(AR) d $@ \
		putchar.rel heap.rel fstubs.rel setjmp.rel errno.rel \
		rand.rel _calloc.rel _malloc.rel _realloc.rel _free.rel \
		printf_large.rel puts.rel gets.rel assert.rel time.rel \
		tolower.rel toupper.rel _ltoa.rel _itoa.rel abs.rel \
		vprintf.rel vfprintf.rel sprintf.rel

# Assembly files which need to be preprocessed --- run through cpp first.
$(OBJ)/%.s: $(TOP)/%.S
	@echo CPP $@
	@mkdir -p $(dir $@)
	$(hide) $(CPP) $(INCLUDES) $(DEFINES) -o $@ $<

# Likewise, for dynamically generated assembly files.
.PRECIOUS: $(OBJ)/%.s
$(OBJ)/%.s: $(OBJ)/%.S
	@echo CPP $@
	@mkdir -p $(dir $@)
	$(hide) $(CPP) $(INCLUDES) $(DEFINES) -o $@ $<

# Dynamically generated preprocessed assembly files.
$(OBJ)/%.$O: $(OBJ)/%.s
	@echo AS $@
	@mkdir -p $(dir $@)
	$(hide) $(AS) $(INCLUDES) $(DEFINES) -c -o $@ $<

# Ordinary C files.
$(OBJ)/%.$O: $(TOP)/%.c
	@echo CC $@
	@mkdir -p $(dir $@)
	$(hide) $(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c -o $@ $<

# Assembly files which don't need to be preprocessed.
$(OBJ)/%.$O: $(TOP)/%.s
	@echo AS $@
	@mkdir -p $(dir $@)
	$(hide) $(AS) $(INCLUDES) $(DEFINES) -c -o $@ $<

# Libraries.
$(OBJ)/%.$A:
	@echo AR $@
	@mkdir -p $(dir $@)
	$(hide) rm -f $@
	$(hide) $(AR) -rc $@ $^

# Executables. Add object files by adding prerequisites.
$(OBJ)/Applications/%: $(HOSTOBJ)/Library/tools/binman $(CRT0) $(LIBC) $(LIBCLEAN)
	@echo LINK $@
	@mkdir -p $(dir $@)
	$(hide) $(CC) $(LDFLAGS) $(INCLUDES) $(DEFINES) \
		-o $@.ihx $(wordlist 2, 999, $^)
	$(hide) makebin -p -s 65535 $@.ihx $@.bin
	$(hide) $< $@.bin $@.map $@

$(OBJ)/Applications/%: INCLUDES += -I$(TOP)/Library/include
$(OBJ)/Library/%: INCLUDES += -I$(TOP)/Library/include
$(OBJ)/Library/%: INCLUDES += -I$(OBJ)/Library/libs/fuzix

$(HOSTOBJ)/Library/tools/binman: $(HOSTOBJ)/Library/tools/binman.o

