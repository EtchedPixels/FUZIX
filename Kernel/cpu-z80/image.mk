tools/analysemap: tools/analysemap.c

tools/bihx: tools/bihx.c

tools/binmunge: tools/binmunge.c

tools/memhogs: tools/analysemap
	cp tools/analysemap tools/memhogs

tools/binman: tools/binman.c

tools/bintomdv: tools/bintomdv.c

tools/bin2sna: tools/bin2sna.c

tools/bin2z80: tools/bin2z80.c

tools/bankld/sdldz80:
	+(cd tools/bankld; make)

cpm-loader/cpmload.bin:	cpm-loader/cpmload.s cpm-loader/fuzixload.s cpm-loader/makecpmloader.c
	+make -C cpm-loader

tools/makejv3: tools/makejv3.c

fuzix.ihx: target $(OBJS) platform-$(TARGET)/fuzix.lnk tools/bankld/sdldz80
	$(CROSS_LD) -n -k $(LIBZ80) -f platform-$(TARGET)/fuzix.lnk

fuzix.bin: fuzix.ihx tools/bihx tools/analysemap tools/memhogs tools/binman tools/bintomdv tools/binmunge tools/bin2sna tools/bin2z80 cpm-loader/cpmload.bin
	-cp hogs.txt hogs.txt.old
	tools/memhogs <fuzix.map |sort -nr >hogs.txt
	head -5 hogs.txt
	tools/bihx fuzix.ihx
	tools/binprep
	+make -C platform-$(TARGET) image

