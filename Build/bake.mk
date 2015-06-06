# Load all the build class macros.

include $(wildcard $(BUILD)/classes/*)

find-makefile = \
	$(eval DIR := $(dir $(word $(words $(MAKEFILE_LIST)), $(MAKEFILE_LIST))))

# Abusing foreach here gives us local variables.

build = \
	$(foreach TARGET,$(strip $1), \
		$(foreach CLASS,$(strip $2), \
			$(eval TARGETS += $(TARGET)) \
			$(eval $(TARGET).class = $(CLASS)) \
			$(eval $(TARGET).dir = $(DIR)) \
			$(eval $(call $(CLASS).rules,$(TARGET)))))

# Given a path $1 and a list of filenames $2, prepends the path to any relative
# filename in $2 (but leaves absolute paths or $(OBJ)-relative paths alone).

absify = \
	$(filter /%, $2) \
	$(filter $(OBJ)/%, $2) \
	$(addprefix $1, \
		$(filter-out /%, \
		$(filter-out $(OBJ)/%, $2)))

# Standard targets.

tests:
clean:
.PHONY: tests clean
.DELETE_ON_ERROR:
