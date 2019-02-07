tools/analysemap: tools/analysemap.c

tools/bihx: tools/bihx.c

tools/binmunge: tools/binmunge.c

tools/memhogs: tools/analysemap
	cp tools/analysemap tools/memhogs

tools/binman: tools/binman.c

fuzix.ihx: target $(OBJS) platform-$(TARGET)/fuzix.lnk
	$(CROSS_LD) -n -k $(LIBZ80) -f platform-$(TARGET)/fuzix.lnk

fuzix.bin: fuzix.ihx tools/bihx tools/analysemap tools/memhogs tools/binman tools/bintomdv tools/binmunge tools/bin2sna tools/bin2z80 cpm-loader/cpmload.bin tools/flat2z80
	-cp hogs.txt hogs.txt.old
	tools/memhogs <fuzix.map |sort -nr >hogs.txt
	head -5 hogs.txt
	tools/bihx fuzix.ihx
	tools/binprep
	+$(MAKE) -C platform-$(TARGET) image

