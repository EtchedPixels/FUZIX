KERNEL = $(TOP)/fuzix-$(PLATFORM).com

$o/%.$O: EXTRA += $o/platform
$o/%.$O: INCLUDES += -I$s/dev

KERNEL_OBJS += \
	$o/$p/commonmem.$O \
	$o/$p/crt0.$O \
	$o/$p/devfd.$O \
	$o/$p/devhd.$O \
	$o/$p/devices.$O \
	$o/$p/devlpr.$O \
	$o/$p/devmegasd.$O \
	$o/$p/devrtc.$O \
	$o/$p/devtty.$O \
	$o/$p/discard.$O \
	$o/$p/main.$O \
	$o/$p/msx2.$O \
	$o/$p/tricks.$O \
	$o/$p/vdp.$O \
	$o/bank16k.$O \
	$o/dev/blkdev.$O \
	$o/dev/devsd.$O \
	$o/dev/devsd_discard.$O \
	$o/dev/mbr.$O \
	$o/dev/rp5c01.$O \
	$o/lowlevel-z80.$O \
	$o/syscall_exec16.$O \
	$o/usermem.$O \
	$o/usermem_std-z80.$O \
	$o/vt.$O \

# Set segments of platform-specific files.

$o/$p/devrtc.$O:    SEGMENT = $(CFLAGS_SEG2)
$o/$p/devfd.$O:     SEGMENT = $(CFLAGS_SEG2)
$o/$p/devhd.$O:     SEGMENT = $(CFLAGS_SEG2)
$o/$p/devlpr.$O:    SEGMENT = $(CFLAGS_SEG2)
$o/$p/devices.$O:   SEGMENT = $(CFLAGS_SEG2)
$o/$p/main.$O:      SEGMENT = $(CFLAGS_SEG2)
$o/$p/devtty.$O:    SEGMENT = $(CFLAGS_SEG2)
$o/$p/devmegasd.$O: SEGMENT = --codeseg COMMONMEM
$o/$p/discard.$O:   SEGMENT = $(CFLAGS_SEGDISC)
$o/dev/blkdev.$O:   SEGMENT = $(CFLAGS_SEG2)
$o/dev/devsd.$O:    SEGMENT = $(CFLAGS_SEG2)
$o/dev/mbr.$O:      SEGMENT = $(CFLAGS_SEG2)
$o/dev/rp5c01.$O:   SEGMENT = $(CFLAGS_SEG2)

.DELETE_ON_ERROR: $o/fuzix.bin
$o/fuzix.bin: $t/bankld/sdldz80 $(KERNEL_OBJS) \
		|$t/bihx $t/memhogs $t/binman
	@echo LINK $@
	@mkdir -p $(dir $@)/kernel
	$(hide) $< \
		-n \
		-k $(SDCC_LIBS) \
		-mwxuy \
		-i $(dir $@)/kernel/image.ihx \
		-b _CODE=0x0000 \
		-b _COMMONMEM=0xf000 \
		-b _DISCARD=0xe000 \
		-l z80 \
		$(wordlist 2, 999, $^)
	$(hide) (cd $(dir $@)/kernel && $(abspath $(HOSTOBJ)/Kernel/tools/bihx) image.ihx)
	$(hide) mv $(dir $@)/hogs.txt $(dir $@)/hogs.txt.old 2> /dev/null || true
	$(hide) $(HOSTOBJ)/Kernel/tools/memhogs \
		< $(dir $@)/kernel/image.map | sort -nr > $(dir $@)/hogs.txt
	$(hide) $(HOSTOBJ)/Kernel/tools/binman \
		$(dir $@)/kernel/common.bin \
		$(dir $@)/kernel/image.map \
		$@

.DELETE_ON_ERROR: $o/fuzix-boot.bin
$o/fuzix-boot.bin: $t/bankld/sdldz80 $o/$p/bootrom.$O |$t/bihx
	@echo LINK $@
	@mkdir -p $(dir $@)/boot
	$(hide) $< \
		-n \
		-k $(SDCC_LIBS) \
		-mwxuy \
		-i $(dir $@)/boot/image.ihx \
		-b _BOOT=0x4000 \
		-l z80 \
		$(wordlist 2, 999, $^) \
		-e
	$(hide) (cd $(dir $@)/boot && $(abspath $(HOSTOBJ)/Kernel/tools/bihx) image.ihx)
	$(hide) mv $(dir $@)/boot/common.bin $@

$(KERNEL): $o/fuzix.bin $o/fuzix-boot.bin
	@echo KERNEL $@
	@mkdir -p $(dir $@)
	$(hide) dd if=$(word 1, $^) of=$@ \
		bs=256 skip=1 2> /dev/null
	$(hide) dd if=$(word 2, $^) of=$(@:.com=.ascii8.rom) \
		skip=1 bs=16384 conv=sync 2> /dev/null
	$(hide) dd if=$(word 1, $^) of=$(@:.com=.ascii8.rom) \
		seek=1 bs=16384 conv=sync 2> /dev/null

all:: $(KERNEL)

