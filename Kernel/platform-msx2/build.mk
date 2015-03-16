KERNEL = $(TOP)/fuzix-$(PLATFORM).com

kerneltool = $(HOSTOBJ)/Kernel/tools
platform = platform-$(PLATFORM)

$(OBJ)/Kernel/%.$O: private INCLUDES += -I$(TOP)/Kernel/dev

KERNEL_OBJS += \
	$(OBJ)/Kernel/$(platform)/commonmem.$O \
	$(OBJ)/Kernel/$(platform)/crt0.$O \
	$(OBJ)/Kernel/$(platform)/devfd.$O \
	$(OBJ)/Kernel/$(platform)/devhd.$O \
	$(OBJ)/Kernel/$(platform)/devices.$O \
	$(OBJ)/Kernel/$(platform)/devlpr.$O \
	$(OBJ)/Kernel/$(platform)/devmegasd.$O \
	$(OBJ)/Kernel/$(platform)/devrtc.$O \
	$(OBJ)/Kernel/$(platform)/devtty.$O \
	$(OBJ)/Kernel/$(platform)/discard.$O \
	$(OBJ)/Kernel/$(platform)/main.$O \
	$(OBJ)/Kernel/$(platform)/msx2.$O \
	$(OBJ)/Kernel/$(platform)/tricks.$O \
	$(OBJ)/Kernel/$(platform)/vdp.$O \
	$(OBJ)/Kernel/bank16k.$O \
	$(OBJ)/Kernel/dev/blkdev.$O \
	$(OBJ)/Kernel/dev/devsd.$O \
	$(OBJ)/Kernel/dev/devsd_discard.$O \
	$(OBJ)/Kernel/dev/mbr.$O \
	$(OBJ)/Kernel/dev/rp5c01.$O \
	$(OBJ)/Kernel/lowlevel-z80.$O \
	$(OBJ)/Kernel/syscall_exec16.$O \
	$(OBJ)/Kernel/usermem_std-z80.$O \
	$(OBJ)/Kernel/vt.$O \

# Set segments of platform-specific files.

$(OBJ)/Kernel/$(platform)/devrtc.$O:    SEGMENT = $(CFLAGS_SEG2)
$(OBJ)/Kernel/$(platform)/devfd.$O:     SEGMENT = $(CFLAGS_SEG2)
$(OBJ)/Kernel/$(platform)/devhd.$O:     SEGMENT = $(CFLAGS_SEG2)
$(OBJ)/Kernel/$(platform)/devlpr.$O:    SEGMENT = $(CFLAGS_SEG2)
$(OBJ)/Kernel/$(platform)/devices.$O:   SEGMENT = $(CFLAGS_SEG2)
$(OBJ)/Kernel/$(platform)/main.$O:      SEGMENT = $(CFLAGS_SEG2)
$(OBJ)/Kernel/$(platform)/devtty.$O:    SEGMENT = $(CFLAGS_SEG2)
$(OBJ)/Kernel/$(platform)/devmegasd.$O: SEGMENT = --codeseg COMMONMEM
$(OBJ)/Kernel/$(platform)/discard.$O:   SEGMENT = $(CFLAGS_SEGDISC)
$(OBJ)/Kernel/dev/blkdev.$O:            SEGMENT = $(CFLAGS_SEG2)
$(OBJ)/Kernel/dev/devsd.$O:             SEGMENT = $(CFLAGS_SEG2)
$(OBJ)/Kernel/dev/mbr.$O:               SEGMENT = $(CFLAGS_SEG2)
$(OBJ)/Kernel/dev/rp5c01.$O:            SEGMENT = $(CFLAGS_SEG2)

.DELETE_ON_ERROR: $(OBJ)/Kernel/fuzix.bin
$(OBJ)/Kernel/fuzix.bin: $(KERNEL_OBJS) \
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
		-b _CODE=0x0000 \
		-b _COMMONMEM=0xf000 \
		-b _DISCARD=0xe000 \
		-l z80 \
		$(KERNEL_OBJS)
	$(hide) (cd $(OBJ)/kernel && $(abspath $(kerneltool)/bihx) image.ihx)
	$(hide) mv $(KERNEL:.com=.hogs.txt) $(KERNEL:.com=.hogs.txt.old) 2> /dev/null || true
	$(hide) $(kerneltool)/memhogs \
		< $(OBJ)/kernel/image.map | sort -nr > $(KERNEL:.com=.hogs.txt)
	$(hide) $(kerneltool)/binman \
		$(OBJ)/kernel/common.bin \
		$(OBJ)/kernel/image.map \
		$@

.DELETE_ON_ERROR: $(OBJ)/Kernel/fuzix-boot.bin
$(OBJ)/Kernel/fuzix-boot.bin: $(OBJ)/Kernel/$(platform)/bootrom.$O \
		$(kerneltool)/bihx \
		$(kerneltool)/bankld/sdldz80
	@echo LINK $@
	@mkdir -p $(OBJ)/boot
	$(hide) $(kerneltool)/bankld/sdldz80 \
		-n \
		-k $(SDCC_LIBS) \
		-mwxuy \
		-i $(OBJ)/boot/image.ihx \
		-b _BOOT=0x4000 \
		-l z80 \
		$<
	$(hide) (cd $(OBJ)/boot && $(abspath $(HOSTOBJ)/Kernel/tools/bihx) image.ihx)
	$(hide) mv $(OBJ)/boot/common.bin $@

$(KERNEL): $(OBJ)/Kernel/fuzix.bin $(OBJ)/Kernel/fuzix-boot.bin
	@echo KERNEL $@
	@mkdir -p $(dir $@)
	$(hide) dd if=$(word 1, $^) of=$@ \
		bs=256 skip=1 2> /dev/null
	$(hide) dd if=$(word 2, $^) of=$(@:.com=.ascii8.rom) \
		skip=1 bs=16384 conv=sync 2> /dev/null
	$(hide) dd if=$(word 1, $^) of=$(@:.com=.ascii8.rom) \
		seek=1 bs=16384 conv=sync 2> /dev/null

all:: $(KERNEL)

