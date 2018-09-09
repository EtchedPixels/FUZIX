ASRCS = crt0.s tricks.s commonmem.s rc2014.s monitor.s vfdterm.s
ASRCS += devrd_zeta2_hw.s
CSRCS = devices.c main.c devtty.c devrd_zeta2.c vfd-debug.c vfd-term.c
CSRCS += devinput.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../dev/devide_discard.c ../dev/ds1302_discard.c
DSRCS = ../dev/devfd.c ../dev/devide.c ../dev/mbr.c ../dev/blkdev.c
DSRCS +=  ../dev/devrd.c ../dev/ds1302.c
DASRCS = ../dev/devfd_hw.s ../dev/devrd_hw.s ../dev/ds1302_rc2014.s
NSRCS = ../dev/net/net_native.c

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.rel)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.rel, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.rel, $(DASRCS))
NOBJS = $(patsubst ../dev/net/%.c,%.rel, $(NSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DAOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(NOBJS)

CROSS_CCOPTS += -I../dev/ -I../dev/net/

JUNK = *.rel *.lst *.asm *.sym *.rst *.map *.ihx *.bin

all:	$(OBJS) diskboot.bin

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DAOBJS): %.rel: ../dev/%.s
	$(CROSS_AS) $(ASOPTS) $@ $<

$(DISCARD_COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.rel: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ bootrom.ihx bootrom.bin diskboot.bin fuzix.com fuzix.rom

diskboot.bin:	diskboot.s
	$(CROSS_AS) $(ASOPTS) diskboot.s
	sdldz80 -nmi diskboot.rel
	makebin -s 65536 diskboot.ihx | dd bs=512 skip=125 count=1 of=diskboot.bin

image:
	sdasz80 -o bootrom.s
	sdldz80 -m -i bootrom.rel
	makebin -s 136 bootrom.ihx > bootrom.bin
#	cat bootrom.bin ../fuzix.bin | dd conv=sync bs=65536 count=1 of=fuzix.rom
	cat bootrom.bin ../fuzix.bin > tmp.rom
	cat tmp.rom | dd conv=sync bs=65536 count=1 of=fuzix.rom
	-cat fuzix.rom ../rc2014_root_fs > fuzix_rc2014_sio_bootfs.rom
	../cpm-loader/makecpmloader ../cpm-loader/cpmload.bin ../fuzix.bin 0x88 fuzix.com
	../cpm-loader/makecpmloader ../cpm-loader/fuzixload.bin ../fuzix.bin 0x88 fuzix

