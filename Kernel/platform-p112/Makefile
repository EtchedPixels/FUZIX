CSRCS += devices.c main.c devtty.c devfd.c
DSRCS = ../dev/blkdev.c ../dev/devide.c ../dev/mbr.c ../dev/ds1302.c
ASRCS = crt0.s z180.s commonmem.s p112.s ds1302-p112.s devfd2.s monitor.s flopboot.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS) flopboot.bin

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ fuzix.com fuzix-boot.fdd

z180.rel:		z180.s kernel.def ../cpu-z180/z180.s

flopboot.bin:	flopboot.s
	$(CROSS_AS) $(ASOPTS) flopboot.s
	sdldz80 -nmi flopboot.rel
	makebin -s 65536 flopboot.ihx > flopboot.bin

image:
	../cpm-loader/makecpmloader ../cpm-loader/cpmload.bin ../fuzix.bin 0x88 fuzix.com
	./flopboot-mkimage flopboot.bin ../fuzix.bin fuzix-boot.fdd
