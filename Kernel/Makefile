TARGET_LIST = platform-nc100 platform-micropack platform-pcw8256 platform-socz80 platform-zx128 platform-trs80 platform-z80pack platform-z80pack-lite platform-z80pack32 platform-dragon platform-tgl6502 platform-plus3 platform-zeta-v2 platform-coco3 platform-multicomp09

#export TARGET = 6809simple
#export TARGET = 68hc11test
#export TARGET = 8086test
#export TARGET = atarist
#export TARGET = bbcmicro
#export TARGET = dragon
#export TARGET = coco2
#export TARGET = coco3
#export TARGET = dragon-nx32
#export TARGET = multicomp09
#export TARGET = micropack
#export TARGET = msx1
#export TARGET = msx2
#export TARGET = mtx
#export TARGET = n8vem-mark4
#export TARGET = nc100
#export TARGET = p112
#export TARGET = pcw8256
#export TARGET = plus3
#export TARGET = px4plus
#export TARGET = socz80
#export TARGET = tgl6502
#export TARGET = trs80
#export TARGET = ubee
export TARGET = z80pack
#export TARGET = z80pack-lite
#export TARGET = zeta-v2
#export TARGET = zx128

export VERSION = "0.1"
export SUBVERSION = "ac1"
export ROOT_DIR

ifeq ($(MAKE),)
  export MAKE = "make"
endif

UNAME_S := $(shell uname -s)

ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

all:	fuzix.bin

# Platform properties
include platform-$(TARGET)/target.mk
# CPU properties
include cpu-$(CPU)/rules.mk
# Overrides (optional)
-include platform-$(TARGET)/rules.mk



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
C1SRCS =  version.c filesys.c devio.c kdata.c
C1SRCS += inode.c tty.c
C2SRCS =  syscall_proc.c process.c malloc.c
C2SRCS += simple.c single.c bank16k.c bank16k_low.c bank32k.c bankfixed.c flat.c
#
#	Usually we map sysalls into CODE2 but they can be overlaid
#	Assume we'll never be doing overlays with level2 or networking
#
CS1SRCS = syscall_fs.c syscall_level2.c select.c
CS2SRCS = syscall_fs2.c
CS3SRCS = syscall_fs3.c	syscall_net.c
CS4SRCS = syscall_other.c
CS5SRCS = syscall_exec$(BITS).c
#
#
#	Drop some bits into CODE3 so the 6502 banks fit nicely. May well
#	need to do this on Z80 as well
#
C3SRCS = devsys.c mm.c swap.c usermem.c timer.c audio.c level2.c
CVIDEOSRCS = vt.c
CFONTSRCS = font4x6.c font6x8.c font8x8.c


ASRCS =  lowlevel-$(CPU)$(BANKED)$(ASMEXT) usermem_std-$(CPU)$(BANKED)$(ASMEXT)


CDOBJS = $(CDSRCS:.c=$(BINEXT))
C1OBJS = $(C1SRCS:.c=$(BINEXT))
C2OBJS = $(C2SRCS:.c=$(BINEXT))
C3OBJS = $(C3SRCS:.c=$(BINEXT))

CS1OBJS = $(CS1SRCS:.c=$(BINEXT))
CS2OBJS = $(CS2SRCS:.c=$(BINEXT))
CS3OBJS = $(CS3SRCS:.c=$(BINEXT))
CS4OBJS = $(CS4SRCS:.c=$(BINEXT))
CS5OBJS = $(CS5SRCS:.c=$(BINEXT))

CFONTOBJS = $(CFONTSRCS:.c=$(BINEXT))
CVIDEOOBJS = $(CVIDEOSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:$(ASMEXT)=$(BINEXT))

CSSRCS = $(CS1SRCS) $(CS2SRCS) $(CS3SRCS) $(CS4SRCS) $(CS5SRCS)
CSOBJS = $(CS1OBJS) $(CS2OBJS) $(CS3OBJS) $(CS4OBJS) $(CS5OBJS)

CSRCS = $(CDSRCS) $(C1SRCS) $(C2SRCS) $(C3SRCS) $(CFONTSRCS) $(CVIDEOSRCS) $(CSSRCS)
COBJS = $(CDOBJS) $(C1OBJS) $(C2OBJS) $(C3OBJS) $(CFONTOBJS) $(CVIDEOOBJS) $(CSOBJS)

OBJS  = $(COBJS) $(AOBJS)

JUNK = $(CSRCS:.c=.lst) $(CSRCS:.c=.s) $(CSRCS:.c=.asm) $(CSRCS:.c=.sym) $(ASRCS:$(ASMEXT)=.lst) $(ASRCS:$(ASMEXT)=.sym) $(CSRCS:.c=.rst) $(CSRCS:.c=.o)  $(ASRCS:$(ASMEXT)=.rst)

.SUFFIXES:		# delete the default suffixes
.SUFFIXES: .c .s .rel .S

usermem_std-z180.rel:	usermem_std-z180.s usermem_std-z80.s

lowlevel-z180.rel:	lowlevel-z180.s lowlevel-z80.s

target:
	-rm -f platform
	ln -sf platform-$(TARGET) platform
	+$(MAKE) -C platform-$(TARGET)

$(C1OBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) $<

$(CDOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) $<

$(C2OBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) $<

$(C3OBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG3) $<

$(CFONTOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_FONT) $<

$(CVIDEOOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_VIDEO) $<

$(CS1OBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SYS1) $<

$(CS2OBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SYS2) $<

$(CS3OBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SYS3) $<

$(CS4OBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SYS4) $<

$(CS5OBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SYS5) $<

$(AOBJS): %$(BINEXT): %$(ASMEXT)
	$(CROSS_AS) $(ASOPTS) -o $*$(BINEXT) $<

version.c: makeversion
	./makeversion $(VERSION) $(SUBVERSION) $(TARGET)

tools/filesizes: tools/filesizes.c

tools/map_syscall: tools/map_syscall.c

tools/make4x6: tools/make4x6.c

tools/bbc: tools/bbc.c

font4x6.c: tools/make4x6
	tools/make4x6 >font4x6.c

clean:
	rm -f $(OBJS) $(JUNK) fuzix.cdb fuzix.com fuzix.tmp platform fuzix.bin fuzix.map fuzix.noi fuzix.ihx common.ihx common.bin relocs.dat core *~ include/*~ version.c tools/make4x6 tools/analysemap tools/memhogs tools/binman tools/bihx tools/bintomdv tools/chkmdv tools/decbdragon tools/decb-image hogs.txt hogs.txt.old tools/*~
	+$(MAKE) -C platform-$(TARGET) clean
	+$(MAKE) -C cpm-loader clean
	+$(MAKE) -C tools/bankld clean

clean-all: clean
	(cd tools/bankld; $(MAKE) clean)
	$(foreach target,$(TARGET_LIST), $(MAKE) -C $(target) clean;)

include cpu-$(CPU)/image.mk
-include platform-$(TARGET)/image.mk
