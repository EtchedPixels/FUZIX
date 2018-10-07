
CSRCS = ../devlpr.c ../devtty.c ../devrd.c ../devaudio.c ../devgfx.c
CSRCS += ../devices.c ../main.c ../tc8521.c

ASRCS = ../nc100.s ../crt0.s
ASRCS += ../tricks.s ../commonmem.s

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
OBJS  = $(COBJS) $(AOBJS)

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -o $(dir $@) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.lk  core *~ 

bootblock.img: ../bootblock.s
	sdasz80 -fflopzws bootblock.rel ../bootblock.s
	sdldz80 -nwmx -b BOOTBLOCK=0 -i bootblock.ihx bootblock.rel
	srec_cat -disable-sequence-warning \
                bootblock.ihx -intel \
                -output bootblock.img -binary

image: bootblock.img
	dd if=bootblock.img of=../../fuzix.bin bs=1 conv=notrunc

nc100emu.bin: nc100emu.s
	sdasz80 -fflopzws nc100emu.rel $<
	sdldz80 -nwmx -b _CODE=0x100 -i nc100emu.ihx nc100emu.rel
	srec_cat -disable-sequence-warning \
		nc100emu.ihx -intel -offset -0x100 \
		-output nc100emu.bin -binary
