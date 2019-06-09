CROSS_CCOPTS += -I../dev/

CSRCS = devtty.c devices.c main.c devdisk.c devlpr.c devrtc.c

DISCSRCS = discard.c

ASRCS = system.s crt0.s tricks.s commonmem.s bioshooks.s

DISCARD_DSRCS = 
DSRCS = ../dev/blkdev.c ../dev/mbr.c
NSRCS =

DASRCS =

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))
DISCOBJS = $(DISCSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))

OBJS  = $(COBJS) $(AOBJS) $(NOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS)

JUNK =  *.lst *.asm *.sym *.rst *.lst *.rel

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
	rm -f $(JUNK)  core *~

reloctool: reloctool.c

image: reloctool
	mv ../fuzix.bin ./fuzix.1
	mv ../fuzix.map ./fuzix.1.map
	(cd ..; $(CROSS_LD) -n -k $(LIBZ80) -f platform-$(TARGET)/fuzix2.lnk)
	(cd ..; tools/bihx fuzix.ihx)
	(cd ..; tools/binprep)
	dd if=../fuzix.bin of=./fuzix.2 bs=256 skip=1
	./reloctool fuzix.1 fuzix.2 ./fuzix.1.map FUZIX.REL
	mv ./fuzix.1.map ../fuzix.map
