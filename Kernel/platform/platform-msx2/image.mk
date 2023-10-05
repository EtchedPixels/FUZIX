fuzix.ihx: target $(OBJS) platform/platform-$(TARGET)/fuzix.lnk tools/bankld/sdldz80
	$(CROSS_LD) -n -k $(LIBZ80) -f platform/platform-$(TARGET)/fuzix.lnk
	$(CROSS_LD) -n -k $(LIBZ80) -f platform/platform-$(TARGET)/fuzix_boot.lnk
	$(CROSS_LD) -n -k $(LIBZ80) -f platform/platform-$(TARGET)/fuzix_disk.lnk

fuzix.bin: fuzix.ihx tools/bihx tools/analysemap tools/memhogs tools/binman tools/bintomdv cpm-loader/cpmload.bin
	-cp hogs.txt hogs.txt.old
	tools/memhogs <fuzix.map |sort -nr >hogs.txt
	head -5 hogs.txt
	tools/bihx fuzix.ihx
	tools/analysemap <fuzix.map
	tools/binman common.bin fuzix.map fuzix.bin
	tools/bihx fuzix_boot.ihx
	-cp common.bin fuzix_boot.bin
	tools/bihx fuzix_disk.ihx
	-cp common.bin fuzix_disk.bin
	+$(MAKE) -C platform/platform-$(TARGET) image
	tools/visualize < fuzix.map
