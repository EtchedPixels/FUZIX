CSRCS = \
	../platform-nc100/devlpr.c \
	../platform-nc100/devtty.c \
	../platform-nc100/devrd.c \
	../platform-nc100/devaudio.c \
	../platform-nc100/devgfx.c \
	../platform-nc100/devices.c \
	../platform-nc100/main.c \

ASRCS = \
	../platform-nc100/nc100.s \
	../platform-nc100/crt0.s \
	../platform-nc100/tricks.s \
	../platform-nc100/commonmem.s \

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
OBJS  = $(COBJS) $(AOBJS)

JUNK = \
	$(CSRCS:.c=.lst) \
	$(CSRCS:.c=.asm) \
	$(CSRCS:.c=.sym) \
	$(ASRCS:.s=.lst) \
	$(ASRCS:.s=.sym) \
	$(CSRCS:.c=.rst) \
	$(ASRCS:.s=.rst)

all: $(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -o $@ -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) -o $@ $<

clean:
	rm -f $(OBJS) $(JUNK) core *~ 
	rm -f floppyskeleton.img autoprg.bin bootfloppy.img

bootblock.img: ../platform-nc100/bootblock.s
	sdasz80 -fflopzws bootblock.rel ../platform-nc100/bootblock.s
	sdldz80 -nwmx -b BOOTBLOCK=0 -i bootblock.ihx bootblock.rel
	srec_cat -disable-sequence-warning \
                bootblock.ihx -intel \
                -output bootblock.img -binary

image: floppyskeleton.img autoprg.bin bootblock.img ../fuzix.bin
	dd if=bootblock.img of=../fuzix.bin bs=1 conv=notrunc
	cp floppyskeleton.img ../fuzixfloppy.img
	truncate ../fuzixfloppy.img --size 7680
	mcopy -i ../fuzixfloppy.img autoprg.bin ::auto.prg
	dd if=../fuzix.bin bs=16k skip=0 count=1 | mcopy -i ../fuzixfloppy.img - ::load4000.80
	dd if=../fuzix.bin bs=16k skip=1 count=1 | mcopy -i ../fuzixfloppy.img - ::load4000.81
	dd if=../fuzix.bin bs=16k skip=2 count=1 | mcopy -i ../fuzixfloppy.img - ::load4000.82
	echo -n | mcopy -i ../fuzixfloppy.img - ::call4000.80

floppyskeleton.img: floppyskeleton.s
	sdasz80 -fflopzws floppyskeleton.rel floppyskeleton.s
	sdldz80 -nwmx -i floppyskeleton.ihx floppyskeleton.rel
	srec_cat -disable-sequence-warning \
 		floppyskeleton.ihx -intel \
 		-output floppyskeleton.img -binary

autoprg.bin: autoprg.s
	sdasz80 -fflopzws autoprg.rel autoprg.s
	sdldz80 -nwmx -b _CODE=0xa000 -i autoprg.ihx autoprg.rel
	srec_cat -disable-sequence-warning \
 		autoprg.ihx -intel -offset -0xa000 \
 		-output autoprg.bin -binary
