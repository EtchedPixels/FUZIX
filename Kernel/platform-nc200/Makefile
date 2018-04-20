CSRCS = \
	devfd.c \
	../platform-nc100/devlpr.c \
	../platform-nc100/devtty.c \
	../platform-nc100/devrd.c \
	../platform-nc100/devaudio.c \
	../platform-nc100/devgfx.c \
	../platform-nc100/devices.c \
	../platform-nc100/main.c \

ASRCS = \
	crt0.s \
	fdc765.s \
	../platform-nc100/nc100.s \
	../platform-nc100/tricks.s \
	../platform-nc100/commonmem.s \

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DOBJS = $(DSRCS:.c=.rel)
OBJS  = $(COBJS) $(AOBJS) $(DOBJS)

CROSS_CCOPTS += -I../dev/

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

image: floppyskeleton.img autoprg.bin ../fuzix.bin
	cp floppyskeleton.img ../fuzixfloppy.img
	truncate ../fuzixfloppy.img --size 7680
	mcopy -i ../fuzixfloppy.img autoprg.bin ::auto.prg
	dd if=../fuzix.bin bs=16k skip=0 count=1 | mcopy -i ../fuzixfloppy.img - ::load4000.83
	dd if=../fuzix.bin bs=16k skip=1 count=1 | mcopy -i ../fuzixfloppy.img - ::load4000.84
	dd if=../fuzix.bin bs=16k skip=2 count=1 | mcopy -i ../fuzixfloppy.img - ::load4000.85
	echo -n | mcopy -i ../fuzixfloppy.img - ::call4080.83

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
