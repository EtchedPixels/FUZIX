# Build rules for cc65.

CC65 = cl65
CC65CPP = cpp -nostdinc -undef -P
CC65AS = ca65
CC65AR = ar65
CC65LD = ld65
PLATFORM_RULES = cc65.rules

# Find the cc65 installation directory, fairly crudely from the location of
# the binary (it doesn't seem possible to get this from the cc65 binary
# itself).

CC65_DIR = $(dir $(shell which $(CC65)))/../share/cc65
CC65_LIBDIR = $(CC65_DIR)/lib/


# Flags used everywhere.

cc65.cflags += \
	-t none \
	-O -Or \
	-D__STDC__

cc65.includes += \
	-I$(TOP)/Library/include/6502 \
	-I$(CC65_DIR)/include \
	--asm-include-dir $(CC65_DIR)/asminc \
	--asm-include-dir $(TOP)/Kernel/platform-$(PLATFORM)

cc65.asflags += \
	-I$(TOP)/Kernel/platform-$(PLATFORM) \
	-I$(CC65_DIR)/asminc

# Used when linking user mode executables.

target-exe.ldflags += --config $(TOP)/Build/platforms/$(PLATFORM).cfg
target-exe.extradeps += $(libc.result) $(TOP)/Build/platforms/$(PLATFORM).cfg


# Names of source files from Fuzix's libc that we don't want to compile.
libc-functions.omit = \


# Fuzix' libc conflicts with the standard cc65 compiler library, which is a
# shame because there's bits we want in it. So we steal those bits into our
# own addon library.

libc-runtime.ext = $A
$(call build, libc-runtime, nop)
PLATFORM_EXTRA_LIBC = $(libc-runtime.result)

# Names of object files to pull out of the cc65 libc.
libc-runtime.objs = \
        addeqsp.o add.o addysp.o along.o and.o aslax1.o aslax2.o aslax3.o aslax4.o \
        asleax1.o asleax2.o asleax3.o asleax4.o asrax1.o asrax2.o asrax3.o asrax4.o \
        asreax1.o asreax2.o asreax3.o asreax4.o asr.o axlong.o bneg.o bpushbsp.o \
        callirq.o callmain.o call.o compl.o condes.o decax1.o decax2.o decax3.o \
        decax4.o decax5.o decax6.o decax7.o decax8.o decaxy.o decsp1.o decsp2.o \
        decsp3.o decsp4.o decsp5.o decsp6.o decsp7.o decsp8.o div.o enter.o eq.o \
        ge.o gt.o icmp.o idiv32by16r16.o imul16x16r32.o imul8x8r16.o incax1.o \
        incax2.o incax3.o incax5.o incax6.o incax7.o incax8.o incaxy.o incsp1.o \
        incsp2.o incsp3.o incsp4.o incsp5.o incsp6.o incsp7.o incsp8.o jmpvec.o \
        laddeq.o laddeqsp.o ladd.o land.o lasr.o lbneg.o lcmp.o lcompl.o ldai.o \
        ldau0sp.o ldaui.o ldauisp.o ldaxi.o ldaxsp.o ldeaxi.o ldeaxysp.o ldec.o \
        ldiv.o leaaxsp.o leave.o leq.o le.o lge.o lgt.o linc.o lle.o llt.o lmod.o \
        lmul.o lneg.o lne.o lor.o lpop.o lpush.o lrsub.o lsave.o lshelp.o lshl.o \
        lshr.o lsubeq.o lsubeqsp.o lsub.o ltest.o lt.o ludiv.o luge.o lugt.o lule.o \
        lult.o lumod.o lxor.o makebool.o mod.o mul8.o mulax10.o mulax3.o mulax5.o \
        mulax6.o mulax7.o mulax9.o mul.o neg.o ne.o or.o popa.o popsreg.o push1.o \
        push2.o push3.o push4.o push5.o push6.o push7.o pushaff.o pusha.o pushax.o \
        pushb.o pushbsp.o pushc0.o pushc1.o pushc2.o pushlysp.o pushw.o pushwsp.o \
        regswap1.o regswap2.o regswap.o return0.o return1.o rsub.o shelp.o shl.o \
        shrax1.o shrax2.o shrax3.o shrax4.o shreax1.o shreax2.o shreax3.o shreax4.o \
        shr.o staspidx.o staxspi.o staxsp.o steaxspi.o steaxsp.o stkchk.o subeqsp.o \
        sub.o subysp.o swap.o tosint.o toslong.o udiv32by16r16.o udiv.o uge.o ugt.o \
        ule.o ult.o umod.o umul16x16r32.o umul8x16r24.o umul8x8r16.o xor.o \
        zeropage.o \
		strchr.o strrchr.o memmove.o memcpy.o memset.o setjmp.o longjmp.o

# Names of source files from Fuzix's libc that we don't want to compile.
libc-functions.omit = \
	memmove.c memcpy.c memset.c strtod.c strchr.c strrchr.c strstr.c bzero.c

$(libc-runtime.result): $(CC65_LIBDIR)/sim$(ARCH).lib $(MAKEFILE_LIST)
	@echo LIBRUNTIME $@
	@mkdir -p $(libc-runtime.objdir)
	$(hide) rm -f $(libc-runtime.objdir)/*.rel
	$(hide) (cd $(libc-runtime.objdir) \
		&& $(CC65AR) x $(CC65_LIBDIR)/sim$(ARCH).lib $(libc-runtime.objs) \
		&& $(CC65AR) a $(abspath $@) $(libc-runtime.objs))


# This is the macro which is appended to target build classes; it contains all
# the cc65-specific bits of the build rules.

define cc65.rules

# $1 is the current target name (the same as in build classes).
# Variables referenced with $ are evaluated immediately.
# Variables references with $$ are evaluated at when the rules are
# instantiated.

# Invoke standard rules.

$(call standard.rules,$1)

# Builds an ordinary C file.

$$($1.objdir)/%.o: $(TOP)/%.c
	@echo CC $$@
	@mkdir -p $$(dir $$@)
	$(hide) $(CC65) \
		--create-dep $$(basename $$@).d \
		$$(cc65.cflags) $$($$($1.class).cflags) $$($1.cflags) \
		$$(cc65.includes) $$($$($1.class).includes) $$($1.includes) \
		$$(cc65.defines) $$($$($1.class).defines) $$($1.defines) \
		-c -o $$@ $$<

# Builds an ordinary .s file.

$$($1.objdir)/%.o: $(TOP)/%.s
	@echo AS $$@
	@mkdir -p $$(dir $$@)
	$(hide) $(CC65AS) \
		$$(cc65.asflags) $$($$($1.class).asflags) $$($1.asflags) \
		-o $$@ $$<

# Builds a dynamically generated .s file.

$$($1.objdir)/%.o: $$($1.objdir)/%.s
	@echo AS $$@
	@mkdir -p $$(dir $$@)
	$(hide) $(CC65AS) \
		$$(cc65.asflags) $$($$($1.class).asflags) $$($1.asflags) \
		-o $$@ $$<

# Builds a library from object files and other library files. Additional
# libraries are merged in after the object files, from first-to-last order.

ifneq ($$(filter %.$A, $$($1.result)),)

$$($1.result): $$($1.objs) $$($$($1.class).extradeps) $$($1.extradeps)
	@echo AR $$@
	@mkdir -p $$(dir $$@)
	$(hide) rm -f $$(dir $$@)/*.$A $$@
	$(hide) $$(foreach lib, $$(filter %.$A, $$($1.objs)), \
		(cd $$(dir $$@) && $(CC65AR) x $$(abspath $$(lib)) \
			$$$$($(CC65AR) l $$(abspath $$(lib)))) \
		&& ) true
	$$(if $$(filter %.o, $$($1.objs)), $(hide) $(CC65AR) a $$@ $$(filter %.o, $$($1.objs)))
	$$(if $$(filter %.$A, $$($1.objs)), $(hide) $(CC65AR) a $$@ $$(dir $$@)/*.o)

endif

# Builds a target executable.

ifneq ($$(filter %.exe, $$($1.result)),)

$$($1.result): $$($1.objs) $(crt0.result) $(binman.result) \
		$$($$($1.class).extradeps) $$($1.extradeps)
	@echo LINK $$@
	@mkdir -p $$(dir $$@)
	$(hide) $(CC65LD) \
		$$(cc65.ldflags) $$($$($1.class).ldflags) $$($1.ldflags) \
		-o $$@ \
		--start-group \
		$$($1.objs) $(crt0.result) $(libc.result) \
		--end-group

endif

endef


