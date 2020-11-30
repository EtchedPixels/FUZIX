ASRCS = crt0.s ti84pse.s display.s
ASRCS += tricks.s commonmem.s

CSRCS += main.c stub.c
CSRCS += devices.c

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)

OBJS  = $(AOBJS) $(COBJS)

CROSS_CCOPTS += -I../dev/ti84pse/

JUNK = *.rel *.lst *.asm *.sym *.rst *.8xu *.bin *.ihx *.rom

all:	$(OBJS)

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

flash.bin: flash.s
	$(CROSS_AS) $(ASOPTS) flash.s
	sdldz80 -nmi -b _FLASH=0x8000 flash.rel
	makebin -s 65536 flash.ihx | dd bs=16384 skip=2 count=1 of=flash.bin

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image: flash.bin
	mkrom fuzix.rom 0x200000 ../fuzix.bin:0x00 flash.bin:0x1F0000
	mktiupgrade -p -k keys/0A.key -d TI84pSE \
		fuzix.rom fuzix.8xu \
		00 01 02 03 7C
