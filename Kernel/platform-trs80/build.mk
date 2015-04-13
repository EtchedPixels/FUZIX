KERNEL = $(TOP)/fuzix-$(PLATFORM).bin

kerneltool = $(HOSTOBJ)/Kernel/tools
platform = platform-$(PLATFORM)

$(OBJ)/Kernel/%.$O: private INCLUDES += -I$(TOP)/Kernel/dev

KERNEL_OBJS += \
	$(OBJ)/Kernel/$(platform)/commonmem.$O \
	$(OBJ)/Kernel/$(platform)/crt0.$O \
	$(OBJ)/Kernel/$(platform)/devfd.$O \
	$(OBJ)/Kernel/$(platform)/devhd.$O \
	$(OBJ)/Kernel/$(platform)/devhd_discard.$O \
	$(OBJ)/Kernel/$(platform)/devices.$O \
	$(OBJ)/Kernel/$(platform)/devlpr.$O \
	$(OBJ)/Kernel/$(platform)/devtty.$O \
	$(OBJ)/Kernel/$(platform)/floppy.$O \
	$(OBJ)/Kernel/$(platform)/main.$O \
	$(OBJ)/Kernel/$(platform)/tricks.$O \
	$(OBJ)/Kernel/$(platform)/trs80.$O \
	$(OBJ)/Kernel/bankfixed.$O \
	$(OBJ)/Kernel/lowlevel-z80.$O \
	$(OBJ)/Kernel/syscall_exec16.$O \
	$(OBJ)/Kernel/usermem_std-z80.$O \
	$(OBJ)/Kernel/vt.$O \

$(OBJ)/Kernel/%.$O: SEGMENT =

$(KERNEL): $(KERNEL_OBJS) \
		$(kerneltool)/bihx \
		$(kerneltool)/memhogs \
		$(kerneltool)/binman \
		$(kerneltool)/bankld/sdldz80
	@echo LINK $@
	@mkdir -p $(OBJ)/kernel
	$(hide) $(kerneltool)/bankld/sdldz80 \
		-n \
		-k $(SDCC_LIBS) \
		-mwxuy \
		-i $(OBJ)/kernel/image.ihx \
		-b _CODE=0x0100 \
		-b _COMMONMEM=0xd000 \
		-b _DISCARD=0xe000 \
		-l z80 \
		$(KERNEL_OBJS)
	$(hide) (cd $(OBJ)/kernel && $(abspath $(kerneltool)/bihx) image.ihx)
	$(hide) mv $(OBJ)/Kernel/$(platform)/hogs.txt $(dir $@)/hogs.txt.old \
		2> /dev/null || true
	$(hide) $(kerneltool)/memhogs \
		< $(OBJ)/kernel/image.map | sort -nr > $(OBJ)/Kernel/$(platform)/hogs.txt
	$(hide) $(kerneltool)/binman \
		$(OBJ)/kernel/common.bin \
		$(OBJ)/kernel/image.map \
		$@

all:: $(KERNEL)

