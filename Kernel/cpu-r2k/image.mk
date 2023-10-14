tools/analysemap: tools/analysemap.c

tools/binmunge: tools/binmunge.c

tools/memhogs: tools/analysemap
	cp tools/analysemap tools/memhogs

tools/visualize: tools/visualize.c

tools/binman: tools/binman.c

fuzix.ihx: target $(OBJS) platform/platform-$(TARGET)/fuzix.lnk
	$(CROSS_LD) -n -k $(LIBZ80) -f platform/platform-$(TARGET)/fuzix.lnk

fuzix.bin: fuzix.ihx tools/bihx tools/analysemap tools/memhogs tools/binman tools/binmunge tools/visualize
	-cp hogs.txt hogs.txt.old
	tools/memhogs <fuzix.map |sort -nr >hogs.txt
	head -5 hogs.txt

	tools/analysemap <fuzix.map
	makebin -s 65536 -p fuzix.ihx >fuzix.tmp
	tools/binman fuzix.tmp fuzix.map fuzix.bin

	+$(MAKE) -C platform/platform-$(TARGET) image
	tools/visualize < fuzix.map
