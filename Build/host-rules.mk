HOSTCFLAGS = \
	-O2 \
	-g \
	-Wall \
	-Wno-char-subscripts \
	-Wno-deprecated-declarations

# Host object files.
$(HOSTOBJ)/%.o: $(TOP)/%.c
	@echo HOSTCC $@
	@mkdir -p $(dir $@)
	$(hide) $(HOSTCC) $(HOSTCFLAGS) \
		-MM -MF $(basename $@).d -MT $(basename $@).o $<
	$(hide) $(HOSTCC) $(HOSTCFLAGS) -c -o $(basename $@).o $<

# Tell make how to generate .d files, to stop it confusing them with
# executables.
$(HOSTOBJ)/%.d: $(HOSTOBJ)/%.o ;

# Host executables. Add object files by adding prerequisites.
$(HOSTOBJ)/%:
	@echo HOSTLINK $@
	@mkdir -p $(dir $@)
	$(hide) $(HOSTCC) $(HOSTCFLAGS) -o $@ $^

