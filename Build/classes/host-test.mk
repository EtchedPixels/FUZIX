# In the macros below:
# $1 is the target name.
# Variables referenced with $ are evaluated immediately.
# Variables references with $$ are evaluated at recipe execution time.

define host-test.rules

# Look! Recursive build objects!

$1-exe.srcs = $$($1.srcs)
$$(call build, $1-exe, host-exe)

$1.objdir ?= $(OBJ)/host/$1
$1.result ?= $$($1.objdir)/$1.passed

$1: $$($1.result)
tests: $1
.PHONY: $1

$$($1.result): $$($1-exe.result)
	@echo TEST $$@
	@mkdir -p $$(dir $$@)
	$(hide) rm -f $$($1.result)
	$(hide) $$($1-exe.result) && touch $$($1.result)

endef


