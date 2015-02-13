TARGET_LIST = platform-nc100 platform-micropack platform-pcw8256 platform-socz80 platform-zx128 platform-trs80 platform-z80pack platform-z80pack-lite platform-z80pack32 platform-dragon platform-tgl6502

#export TARGET = 8086test
#export TARGET = atarist
#export TARGET = dragon
#export TARGET = micropack
#export TARGET = msx1
#export TARGET = msx2
#export TARGET = mtx
#export TARGET = n8vem-mark4
#export TARGET = nc100
#export TARGET = p112
#export TARGET = pcw8256
#export TARGET = px4plus
#export TARGET = tgl6502
#export TARGET = trs80
#export TARGET = ubee
#export TARGET = z80pack
#export TARGET = z80pack-lite
#export TARGET= zx128

export VERSION = "0.1"
export SUBVERSION = "ac1"

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
#
#	Keep all of the syscalls in C2SRCS
#
C2SRCS =  syscall_proc.c syscall_fs.c
C2SRCS += syscall_fs2.c syscall_other.c syscall_exec$(BITS).c process.c malloc.c
C2SRCS += simple.c single.c bank16k.c bank16k_low.c bank32k.c bankfixed.c flat.c
#
#	Drop some bits into CODE3 so the 6502 banks fit nicely. May well
#	need to do this on Z80 as well
#
C3SRCS = devsys.c mm.c swap.c usermem.c timer.c
CVIDEOSRCS = vt.c
CFONTSRCS = font4x6.c font6x8.c font8x8.c


ASRCS =  lowlevel-$(CPU)$(BANKED)$(ASMEXT) usermem_std-$(CPU)$(BANKED)$(ASMEXT)


CDOBJS = $(CDSRCS:.c=$(BINEXT))
C1OBJS = $(C1SRCS:.c=$(BINEXT))
C2OBJS = $(C2SRCS:.c=$(BINEXT))
C3OBJS = $(C3SRCS:.c=$(BINEXT))
CFONTOBJS = $(CFONTSRCS:.c=$(BINEXT))
CVIDEOOBJS = $(CVIDEOSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:$(ASMEXT)=$(BINEXT))

CSRCS = $(CDSRCS) $(C1SRCS) $(C2SRCS) $(C3SRCS) $(CFONTSRCS) $(CVIDEOSRCS)
COBJS = $(CDOBJS) $(C1OBJS) $(C2OBJS) $(C3OBJS) $(CFONTOBJS) $(CVIDEOOBJS)

OBJS  = $(COBJS) $(AOBJS)

JUNK = $(CSRCS:.c=.lst) $(CSRCS:.c=.s) $(CSRCS:.c=.asm) $(CSRCS:.c=.sym) $(ASRCS:$(ASMEXT)=.lst) $(ASRCS:$(ASMEXT)=.sym) $(CSRCS:.c=.rst) $(CSRCS:.c=.o)  $(ASRCS:$(ASMEXT)=.rst)

.SUFFIXES:		# delete the default suffixes
.SUFFIXES: .c .s .rel .S

usermem_std-z180.rel:	usermem_std-z180.s usermem_std-z80.s

lowlevel-z180.rel:	lowlevel-z180.s lowlevel-z80.s

target:
	-rm -f platform
	ln -sf platform-$(TARGET) platform
	+make -C platform-$(TARGET)

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

$(AOBJS): %$(BINEXT): %$(ASMEXT)
	$(CROSS_AS) $(ASOPTS) -o $*$(BINEXT) $<

version.c: makeversion
	./makeversion $(VERSION) $(SUBVERSION) $(TARGET)

tools/filesizes: tools/filesizes.c

tools/make4x6: tools/make4x6.c

font4x6.c: tools/make4x6
	tools/make4x6 >font4x6.c

clean:
	rm -f $(OBJS) $(JUNK) fuzix.cdb fuzix.com fuzix.tmp platform fuzix.bin fuzix.map fuzix.noi fuzix.ihx common.ihx common.bin relocs.dat core *~ include/*~ version.c tools/make4x6 tools/analysemap tools/memhogs tools/binman tools/bihx tools/bintomdv tools/decbdragon hogs.txt hogs.txt.old tools/*~
	+make -C platform-$(TARGET) clean
	+make -C cpm-loader clean
	+make -C tools/bankld clean

clean-all: clean
	(cd tools/bankld; make clean)
	$(foreach target,$(TARGET_LIST), make -C $(target) clean;)

include cpu-$(CPU)/image.mk
-include platform-$(TARGET)/image.mk
