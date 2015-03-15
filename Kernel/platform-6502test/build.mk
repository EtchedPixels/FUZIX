KERNEL = $(TOP)/fuzix-$(PLATFORM).bin

platform = platform-$(PLATFORM)

KERNEL_OBJS += \
	$(OBJ)/Kernel/$(platform)/devices.$O \
	$(OBJ)/Kernel/$(platform)/devlpr.$O \
	$(OBJ)/Kernel/$(platform)/devrd.$O \
	$(OBJ)/Kernel/$(platform)/devtty.$O \
	$(OBJ)/Kernel/$(platform)/libc.$O \
	$(OBJ)/Kernel/$(platform)/main.$O \
	$(OBJ)/Kernel/$(platform)/p6502.$O \
	$(OBJ)/Kernel/$(platform)/commonmem.$O \
	$(OBJ)/Kernel/$(platform)/tricks.$O \
	$(OBJ)/Kernel/bank16k.$O \
	$(OBJ)/Kernel/lowlevel-6502.$O \
	$(OBJ)/Kernel/syscall_exec16.$O \
	$(OBJ)/Kernel/usermem.$O \

$(OBJ)/Kernel/$(platform)/%.$O: private SEGMENT =

$(KERNEL): $(TOP)/Kernel/$(platform)/ld65.cfg $(KERNEL_OBJS)
	@echo KERNEL $@
	@mkdir -p $(dir $@)
	$(hide) $(LD) -o $(KERNEL) --mapfile $(KERNEL:.bin=.map) \
		-C $^ $(LIBRUNTIME)

all:: $(KERNEL)


