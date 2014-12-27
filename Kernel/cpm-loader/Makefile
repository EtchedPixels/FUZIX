all:	makecpmloader cpmload.bin

makecpmloader: makecpmloader.c

cpmload.bin:	cpmload.s
	$(CROSS_AS) $(ASOPTS) cpmload.s
	sdldz80 -nmi cpmload.rel
	makebin -p cpmload.ihx > cpmload.bin

clean:
	rm -f *~ *.rst *.lst *.asm *.bin *.sym *.rel *.map *.ihx makecpmloader
