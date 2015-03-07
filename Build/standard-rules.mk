# Standard build rules for a sane system.

$(OBJ)/%.s: %.S
	@echo CPP $@
	@mkdir -p $(dir $@)
	$(hide) $(CPP) $(INCLUDES) $(DEFINES) -o $@ $<

.PRECIOUS: $(OBJ)/%.s
$(OBJ)/%.s: $(OBJ)/%.S
	@echo CPP $@
	@mkdir -p $(dir $@)
	$(hide) $(CPP) $(INCLUDES) $(DEFINES) -o $@ $<

$(OBJ)/%.o: $(OBJ)/%.s
	@echo AS $@
	@mkdir -p $(dir $@)
	$(hide) $(AS) $(INCLUDES) $(DEFINES) -c -o $@ $<

$(OBJ)/%.o: %.c
	@echo CC $@
	@mkdir -p $(dir $@)
	$(hide) $(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c -o $@ $<

$(OBJ)/%.o: %.s
	@echo AS $@
	@mkdir -p $(dir $@)
	$(hide) $(AS) $(INCLUDES) $(DEFINES) -c -o $@ $<

# We add files ten at a time here because sdcc's librarian has a bug which
# can cause it to crash if you use command lines which are too long.
$(OBJ)/%.a:
	@echo AR $@
	@mkdir -p $(dir $@)
	$(hide) rm -f $@
	$(hide) echo $^ | xargs -n10 $(AR) -a $@

$(OBJ)/Library/%: INCLUDES += -I$(TOP)/Library/include
$(OBJ)/Library/%: INCLUDES += -I$(OBJ)/Library/libs/fuzix

