CROSS_CCOPTS += -I../dev/


CSRCS = devtty.c
CSRCS += devices.c main.c

DISCSRCS = discard.c

ASRCS = z280rc.s crt0.s
ASRCS += tricks.s commonmem.s

DISCARD_DSRCS = ../dev/devide_discard.c
DSRCS = ../dev/blkdev.c ../dev/devide.c ../dev/mbr.c

NSRCS = 

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))
DISCOBJS = $(DISCSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))

OBJS  = $(COBJS) $(AOBJS) $(NOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS)

JUNK =  *.lst *.asm *.sym *.rst *.lst

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(DAOBJS): %.rel: ../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~

image:	boot

boot:	boot.s
	sdasz80 -o boot.s
	sdldz80 -i boot.rel
	makebin -s 65535 boot.ihx boot.tmp
	# Skip to F000, copy the rest
	dd if=boot.tmp of=boot.raw bs=512 skip=126 count=4
