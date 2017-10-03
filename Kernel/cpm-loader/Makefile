all:	makecpmloader cpmload.bin fuzixload.bin

makecpmloader: makecpmloader.c

cpmload.bin:	cpmload.s
	$(CROSS_AS) $(ASOPTS) cpmload.s
	sdldz80 -nmi cpmload.rel
	makebin -p cpmload.ihx > cpmload.bin

fuzixload.bin:	fuzixload.s
	$(CROSS_AS) $(ASOPTS) fuzixload.s
	sdldz80 -nmi fuzixload.rel
	makebin -p fuzixload.ihx > fuzixload.bin

clean:
	rm -f *~ *.rst *.lst *.asm *.bin *.sym *.rel *.map *.ihx makecpmloader
