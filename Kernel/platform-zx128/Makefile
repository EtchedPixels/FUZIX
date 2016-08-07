CSRCS = devtty.c devices.c main.c devmdv.c devfd.c bank128.c betadisk.c
DSRCS = ../dev/devide.c ../dev/mbr.c ../dev/blkdev.c
DDSRCS = ../dev/devide_discard.c
ASRCS = crt0.s zx128.s zxvideo.s microdrive.s disciple.s betadisk_internal.s
ASRCS += tricks.s commonmem.s

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
DOBJS = $(patsubst ../dev/%.c,%.rel, $(DSRCS))
DDOBJS = $(patsubst ../dev/%.c,%.rel, $(DDSRCS))
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(DDOBJS)

CROSS_CCOPTS += -I../dev/

CROSS_CC_SEG3 = --codeseg CODE3

JUNK = $(CSRCS:.c=.lst) $(CSRCS:.c=.asm) $(CSRCS:.c=.sym) $(ASRCS:.s=.lst) $(ASRCS:.s=.sym) $(CSRCS:.c=.rst) $(ASRCS:.s=.rst)

all:	$(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) -c $<

$(DOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) -c $<

$(DDOBJS): %.rel: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	( cd ..; tools/bin2sna fuzix.sna )
	( cd ..; tools/bin2z80 fuzix.z80 )
