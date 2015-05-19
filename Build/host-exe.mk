# On entry, $1 is the target name.

$1.objdir ?= $(OBJ)/host/$1
$1.exe ?= $($1.objdir)/$1

$1: $($1.exe)
.PHONY: $1

$1.abssrcs ?= $(call absify, $($1.dir), $($1.srcs))
$1.objs ?= $(patsubst %, $($1.objdir)/%.o, $(basename $($1.abssrcs)))
.SECONDARY: $($1.objs)

# Variables referenced with $ are evaluated immediately.
# Variables references with $$ are evaluated at recipe execution time.
define patterns

$($1.exe): $($1.objs)
	@echo HOSTLINK $1 $$@
	@mkdir -p $$(dir $$@)
	$(hide) gcc -o $$@ $$^ $($1.ldflags) $(LDFLAGS)

$($1.objdir)/%.o: $(TOP)/%.c
	@echo HOSTCC $1 $$@
	@mkdir -p $$(dir $$@)
	$(hide) gcc -c -o $$@ $$< $($1.cflags) $(CFLAGS)

endef
$(eval $(patterns))


