TARGET_LIST = platform-nc100 platform-micropack platform-pcw8256 platform-socz80 platform-spec3 platform-trs80 platform-z80pack platform-z80pack-lite 

export TARGET= nc100
export CPU = z80
#export TARGET = 6809test
#export CPU = 6809
#export TARGET = 6502test
#export CPU = 6502
export VERSION = "0.1"
export SUBVERSION = "ac1"

ifeq ($(CPU),z80)
export CROSS_AS=sdasz80
export CROSS_LD=sdldz80
export CROSS_CC=sdcc
#export CROSS_CCOPTS=-c --std-sdcc99 --no-std-crt0 -mz80 -I$(ROOT_DIR)/cpu-z80 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include --max-allocs-per-node 1000000 --opt-code-size --Werror --stack-auto --constseg CONST
#export CROSS_CCOPTS=-c --std-sdcc99 --no-std-crt0 -mz80 -I$(ROOT_DIR)/cpu-z80 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include --max-allocs-per-node 200000 --opt-code-size --Werror --stack-auto --constseg CONST
export CROSS_CCOPTS=-c --std-sdcc99 --no-std-crt0 -mz80 -I$(ROOT_DIR)/cpu-z80 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include --max-allocs-per-node 30000 --opt-code-size --Werror --stack-auto --constseg CONST
#export CROSS_CCOPTS+=--nostdlib --nostdinc -Isdcclib/include 
export CROSS_CC_SEG2=--codeseg CODE2
export CROSS_CC_SEGDISC=--codeseg DISCARD
export CROSS_CC_FONT=--constseg FONT
export CROSS_CC_VIDEO=--codeseg VIDEO
export ASOPTS=-plosff
export BINEXT = .rel
else ifeq ($(CPU),6502)
export CROSS_AS=ca65
export CROSS_LD=cl65
export CROSS_CC=cl65
export CROSS_CCOPTS=-c -Or -t c128 -I$(ROOT_DIR)/cpu-6502 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
export BINEXT = .o
else
export CROSS_AS=m6809-unknown-as
export CROSS_LD=m6809-unknown-ld
export CROSS_CC = m6809-unknown-gcc
#export CROSS_CCOPTS=-Wall -O2 -I$(ROOT_DIR)/cpu-6809 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
export CROSS_CCOPTS=-c -Wall -O2 -I$(ROOT_DIR)/cpu-6809 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
export ASOPTS=
endif

ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))


#
#	We put these into two code segments so that the caller can plan to
#	bank them with a ROM banking tool. We pull out const data because on
#	such a system you want to the constants unbanked otherwise badness
#	happens on stuff like printf("Hello world\n"); when you bank switch
#	the code.
#
#	This is also useful if you've got an annoying box where the kernel can't
#	be a linear chunk eg if you need to put the kernel in two non adjacent 16K
#	chunks or compile it around a hole for videomemory or similar
#
#	VT and FONT are handled specially because they can be mapped more
#	freely and also because you often want to overlay part of the kernel
#	with the framebuffer when doing video output
#

CDSRCS = start.c
C1SRCS =  version.c kdata.c filesys.c
C1SRCS += inode.c syscall_fs.c process.c usermem.c timer.c
C2SRCS =  devio.c syscall_proc.c
C2SRCS += syscall_fs2.c syscall_other.c
C2SRCS += bank16k.c bank32k.c bankfixed.c single.c simple.c
C2SRCS += tty.c devsys.c
C2SRCS += mm.c swap.c
CVIDEOSRCS = vt.c
CFONTSRCS = font4x6.c font8x8.c


ASRCS =  lowlevel-$(CPU).s usermem_std-$(CPU).s


CDOBJS = $(CDSRCS:.c=$(BINEXT))
C1OBJS = $(C1SRCS:.c=$(BINEXT))
C2OBJS = $(C2SRCS:.c=$(BINEXT))
CFONTOBJS = $(CFONTSRCS:.c=$(BINEXT))
CVIDEOOBJS = $(CVIDEOSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))

CSRCS = $(CDSRCS) $(C1SRCS) $(C2SRCS) $(CFONTSRCS) $(CVIDEOSRCS)
COBJS = $(CDOBJS) $(C1OBJS) $(C2OBJS) $(CFONTOBJS) $(CVIDEOOBJS)

OBJS  = $(COBJS) $(AOBJS)


JUNK = $(CSRCS:.c=.lst) $(CSRCS:.c=.s) $(CSRCS:.c=.asm) $(CSRCS:.c=.sym) $(ASRCS:.s=.lst) $(ASRCS:.s=.sym) $(CSRCS:.c=.rst) $(CSRCS:.c=.o)  $(ASRCS:.s=.rst)

all:	fuzix.bin

.SUFFIXES:		# delete the default suffixes
.SUFFIXES: .c .s .rel

target:
	-rm platform
	ln -sf platform-$(TARGET) platform
	$(MAKE) -C platform-$(TARGET)

$(C1OBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $<

$(CDOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) $<

$(C2OBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) $<

$(CFONTOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_FONT) $<

$(CVIDEOOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_VIDEO) $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) -o $*$(BINEXT) $<

version.c: makeversion
	./makeversion $(VERSION) $(SUBVERSION) $(TARGET)

toosl/analysemap: tools/analysemap.c

tools/memhogs: tools/analysemap
	cp tools/analysemap tools/memhogs

tools/binman: tools/binman.c

tools/filesizes: tools/filesizes.c

tools/make4x6: tools/make4x6.c

font4x6.c: tools/make4x6
	tools/make4x6 >font4x6.c

ifeq ($(CPU), z80)
uzi.ihx: target $(OBJS) platform-$(TARGET)/uzi.lnk
	$(CROSS_LD) -n -f platform-$(TARGET)/uzi.lnk
	
fuzix.bin: uzi.ihx tools/analysemap tools/memhogs tools/binman
	tools/analysemap <uzi.map
	-cp hogs.txt hogs.txt.old
	tools/memhogs <uzi.map |sort -nr >hogs.txt
	head -5 hogs.txt
	makebin -s 65536 -p uzi.ihx >uzi.tmp
	tools/binman uzi.tmp uzi.map fuzix.bin
	$(MAKE) -C platform-$(TARGET) image
else
uzi.bin: target $(OBJS)
	$(MAKE) -C platform-$(TARGET) image
endif

clean:
	rm -f $(OBJS) $(JUNK) uzi.cdb uzi.com fuzix.bin uzi.map uzi.noi uzi.ihx core *~ include/*~ version.c tools/analysemap tools/memhogs tools/binman hogs.txt hogs.txt.old tools/*~
	+make -C platform-$(TARGET) clean

clean-all: clean
	$(foreach target,$(TARGET_LIST), make -C $(target) clean;)
