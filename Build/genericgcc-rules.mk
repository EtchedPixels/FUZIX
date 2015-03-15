# Build rules for a generic, well-behaved gcc system.

# Forget default suffix rules.
.SUFFIXES:

# Object file and library extensions (sdcc use non-standard ones so it needs to
# be configurable).
O = o
A = a

# This rule is used as a hook to add behaviour before any source file is built.
# It's mostly used to set up the platform symlink.
.PHONY: paths
paths: ;

# Location of standard libraries.
LIBC = $(OBJ)/Library/libc.$A
CRT0 = $(OBJ)/Library/libs/fuzix/crt0.$(ARCH).$O
LIBGCC = $(shell $(CC) --print-libgcc)
.SECONDARY: $(CRT0) $(LIBC) $(LIBRUNTIME)

# Assembly files which need to be preprocessed.
$(OBJ)/%.$O: $(TOP)/%.S |paths
	@echo AS $@
	@mkdir -p $(dir $@)
	$(hide) $(CC) $(INCLUDES) $(DEFINES) \
		-MM -MF $(basename $@).d -MT $@ $<
	$(hide) $(CC) $(ASFLAGS) $(INCLUDES) $(DEFINES) \
		-c -o $@ $<

# Likewise, for dynamically generated assembly files.
$(OBJ)/%.$O: $(OBJ)/%.S |paths
	@echo AS $@
	@mkdir -p $(dir $@)
	$(hide) $(CC) $(INCLUDES) $(DEFINES) \
		-MM -MF $(basename $@).d -MT $@ $<
	$(hide) $(CC) $(ASFLAGS) $(INCLUDES) $(DEFINES) \
		-c -o $@ $<

# Ordinary C files.
$(OBJ)/%.$O: $(TOP)/%.c |paths
	@echo CC $@
	@mkdir -p $(dir $@)
	$(hide) $(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) \
		-MM -MF $(basename $@).d -MT $@ $<
	$(hide) $(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) \
		-c -o $@ $<

# Dynamically generated C files.
$(OBJ)/%.$O: $(OBJ)/%.c |paths
	@echo CC $@
	@mkdir -p $(dir $@)
	$(hide) $(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) \
		-MM -MF $(basename $@).d -MT $@ $<
	$(hide) $(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) \
		-c -o $@ $<

# Tell make how to generate .d files, to stop it confusing them with
# executables.
$(OBJ)/%.d: $(OBJ)/%.$O ;

# Assembly files which don't need to be preprocessed.
$(OBJ)/%.$O: $(TOP)/%.s |paths
	@echo AS $@
	@mkdir -p $(dir $@)
	$(hide) $(CC) $(ASFLAGS) $(INCLUDES) $(DEFINES) \
		-c -o $@ $<

# Libraries.
$(OBJ)/%.$A:
	@echo AR $@
	@mkdir -p $(dir $@)
	$(hide) rm -f $@
	$(hide) $(AR) qcs $@ $^

# Executables. Add object files by adding prerequisites.
$(OBJ)/Applications/%: $(CRT0) $(LIBC) $(LIBGCC)
	@echo LINK $@
	@mkdir -p $(dir $@)
	$(hide) $(LD) $(LDFLAGS) -o $@.elf --start-group $^ --end-group
	$(hide) $(OBJCOPY) --output-target binary $@.elf $@

# Ensure that various things know where their headers are.
$(OBJ)/Applications/%: INCLUDES += -I$(TOP)/Library/include -I$(TOP)/Library/include/$(ARCH)
$(OBJ)/Library/%: INCLUDES += -I$(TOP)/Library/include -I$(TOP)/Library/include/$(ARCH)
$(OBJ)/Library/%: INCLUDES += -I$(OBJ)/Library/libs/fuzix -I$(TOP)/Library/include/$(ARCH)

