
CSRCS = devnascom.c mm58174.c
CSRCS += devices.c main.c
DISCARD_CSRCS = discard.c

ASRCS = nascom.s crt0.s
ASRCS += tricks.s commonmem.s

NASRCS = ../dev/80bus/nascom-vt.s ../dev/80bus/nascom-pagemode.s
NASRCS += ../dev/80bus/gm8x9.s
NSRCS = ../dev/80bus/devgm8x9.c

DSRCS = ../dev/tinydisk.c ../dev/tinysd.c
DISCARD_DSRCS = ../dev/tinydisk_discard.c ../dev/tinysd_discard.c

DASRCS = ../dev/z80softspi.s ../dev/z80softsd.s

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))
NAOBJS = $(patsubst ../dev/80bus/%.s,%.rel, $(NASRCS))
NOBJS = $(patsubst ../dev/80bus/%.c,%.rel, $(NSRCS))
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
OBJS  = $(COBJS) $(AOBJS) $(DISCARD_COBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS) $(NAOBJS) $(NOBJS)

CROSS_CCOPTS += -I../dev/ -I../dev/80bus/

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(NOBJS): %.rel: ../dev/80bus/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DAOBJS): %.rel: ../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

$(NAOBJS): %.rel: ../dev/80bus/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(OBJS) core *~  *.asm *.rst *.lst *.sym *.rel
	rm -f nasboot.lk nasboot.bin

image:
	sdasz80 -o nasboot.s
	sdldz80 -m -i nasboot.rel
	makebin -s 1024 nasboot.ihx | dd of=nasboot.bin bs=512 skip=1
