HOSTCFLAGS = \
	-O2 \
	-g \
	-Wall \
	-Wno-char-subscripts \
	-Wno-deprecated-declarations

# Host object files, living in Standalone.
$(HOSTOBJ)/%.o: $(TOP)/%.c
	@echo HOSTCC $@
	@mkdir -p $(dir $@)
	$(hide) $(HOSTCC) $(HOSTCFLAGS) -MM -MF $(basename $@).d -MT $@ $<
	$(hide) $(HOSTCC) $(HOSTCFLAGS) -c -o $@ $<

# Host executables. Add object files by adding prerequisites.
$(HOSTOBJ)/%:
	@echo HOSTLINK $@
	@mkdir -p $(dir $@)
	$(hide) $(HOSTCC) $(HOSTCFLAGS) -o $@ $^

