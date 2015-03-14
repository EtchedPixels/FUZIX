# Build rules for cc65.

# Find the cc65 installation directory, fairly crudely from the location of
# the binary (it doesn't seem possible to get this from the cc65 binary
# itself).
CC65_DIR = $(dir $(shell which $(CC)))/../share/cc65
CC65_LIBDIR = $(CC65_DIR)/lib/

# Libc configuration.
HAS_FLOAT = n

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
LIBRUNTIME = $(OBJ)/Library/libclean.$A
.SECONDARY: $(CRT0) $(LIBC) $(LIBRUNTIME)

# The cleaned up version of the cc65 runtime library includes these objects
# only.
libruntime_objects = \
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

# We also included selected parts of the cc65 libc.
libruntime_objects += \
	strrchr.o strlen.o strcmp.o strcmp.o strchr.o strcat.o memcpy.o strcpy.o \
	memmove.o memset.o memcmp.o strncpy.o memchr.o strncmp.o strcspn.o \
	strncat.o strtok.o

$(LIBRUNTIME): $(CC65_LIBDIR)/sim$(ARCH).lib Build/cc65-rules.mk
	@echo LIBCLEAN $@
	$(hide) rm -rf $@.tmp
	$(hide) mkdir -p $@.tmp
	$(hide) (cd $@.tmp && $(AR) x $< $(libruntime_objects))
	$(hide) $(AR) a $@ $(patsubst %, $@.tmp/%, $(libruntime_objects))

# Assembly files which need to be preprocessed --- run through cpp first.
$(OBJ)/%.$O: $(TOP)/%.S |paths
	@echo AS $@
	@mkdir -p $(dir $@)
	$(hide) $(CPP) $(INCLUDES) $(DEFINES) \
		-MM -MF $(basename $@).d -MT $@ $<
	$(hide) $(CPP) $(INCLUDES) $(DEFINES) \
		-o $(basename $@).s $<
	$(hide) $(AS) $(ASFLAGS) $(INCLUDES) $(DEFINES) -o $@ $(basename $@).s

# Likewise, for dynamically generated assembly files.
$(OBJ)/%.$O: $(OBJ)/%.S |paths
	@echo AS $@
	@mkdir -p $(dir $@)
	$(hide) $(CPP) $(INCLUDES) $(SDCC_INCLUDE_PATH) $(DEFINES) \
		-MM -MF $(basename $@).d -MT $@ $<
	$(hide) $(CPP) $(INCLUDES) $(SDCC_INCLUDE_PATH) $(DEFINES) \
		-o $(basename $@).s $<
	$(hide) $(AS) $(ASFLAGS) $(INCLUDES) $(DEFINES) -o $@ $(basename $@).s

# Ordinary C files.
$(OBJ)/%.$O: $(TOP)/%.c |paths
	@echo CC $@
	@mkdir -p $(dir $@)
	$(hide) $(CC) --create-dep $(basename $@).d \
		$(CFLAGS) $(INCLUDES) $(DEFINES) -c -o $@ $<

# Dynamically generated C files.
$(OBJ)/%.$O: $(OBJ)/%.c |paths
	@echo CC $@
	@mkdir -p $(dir $@)
	$(hide) $(CC) --create-dep $(basename $@).d \
		$(CFLAGS) $(INCLUDES) $(DEFINES) -c -o $@ $<

# Tell make how to generate .d files, to stop it confusing them with
# executables.
$(OBJ)/%.d: $(OBJ)/%.$O ;

# Assembly files which don't need to be preprocessed.
$(OBJ)/%.$O: $(TOP)/%.s |paths
	@echo AS $@
	@mkdir -p $(dir $@)
	$(hide) $(AS) $(ASFLAGS) $(INCLUDES) $(DEFINES) -o $@ $<

# Libraries.
$(OBJ)/%.$A:
	@echo AR $@
	@mkdir -p $(dir $@)
	$(hide) rm -f $@
	$(hide) $(AR) a $@ $^

# Executables. Add object files by adding prerequisites.
$(OBJ)/Applications/%: $(CRT0) $(LIBC) $(LIBRUNTIME)
	@echo LINK $@
	@mkdir -p $(dir $@)
	$(hide) $(LD) $(LDFLAGS) -o $@ --start-group $^ --end-group

# Default PROGBASE assumes CP/M.
PROGBASE = 0x0100

# Ensure that various things know where their headers are.
$(OBJ)/Applications/%: INCLUDES += -I$(TOP)/Library/include -I$(TOP)/Library/include/6502
$(OBJ)/Library/%: INCLUDES += -I$(TOP)/Library/include -I$(TOP)/Library/include/6502
$(OBJ)/Library/%: INCLUDES += -I$(OBJ)/Library/libs/fuzix -I$(TOP)/Library/include/6502

# Z80 binaries (which we're assuming here) require massaging before they're
# valid Fuzix binaries. This tool does that.
$(HOSTOBJ)/Library/tools/binman: $(HOSTOBJ)/Library/tools/binman.o

