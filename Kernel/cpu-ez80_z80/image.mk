tools/analysemap: tools/analysemap.c

tools/visualize: tools/visualize.c

tools/bihx: tools/bihx.c

tools/binmunge: tools/binmunge.c

tools/memhogs: tools/analysemap
	cp tools/analysemap tools/memhogs

tools/binman: tools/binman.c

tools/bankld/sdldz80:
	+(cd tools/bankld; $(MAKE))

cpm-loader/cpmload.bin:	cpm-loader/cpmload.s cpm-loader/fuzixload.s cpm-loader/makecpmloader.c
	+$(MAKE) -C cpm-loader

tools/makejv3: tools/makejv3.c

tools/trslabel: tools/trslabel.c

fuzix.ihx: target $(OBJS) platform/platform-$(TARGET)/fuzix.lnk tools/bankld/sdldz80
	$(CROSS_LD) -n -k $(LIBZ80) -f platform/platform-$(TARGET)/fuzix.lnk

fuzix.bin: fuzix.ihx tools/bihx tools/analysemap tools/memhogs tools/binman tools/bintomdv tools/binmunge tools/bin2sna tools/bin2z80 cpm-loader/cpmload.bin tools/visualize
	-cp hogs.txt hogs.txt.old
	tools/memhogs <fuzix.map |sort -nr >hogs.txt
	head -5 hogs.txt
	tools/bihx fuzix.ihx
	tools/binprep
	+$(MAKE) -C platform/platform-$(TARGET) image
	tools/visualize < fuzix.map

