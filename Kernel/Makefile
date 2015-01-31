TARGET_LIST = platform-nc100 platform-micropack platform-pcw8256 platform-socz80 platform-zx128 platform-trs80 platform-z80pack platform-z80pack-lite platform-z80pack32 platform-dragon platform-tgl6502

export TARGET= trs80
export CPU = z80
#export TARGET = dragon
#export CPU = 6809
#export TARGET = tgl6502
#export CPU = 6502
#export TARGET=atarist
#export CPU = 68000
#export TARGET = 8086test
#export CPU = 8086

export VERSION = "0.1"
export SUBVERSION = "ac1"

UNAME_S := $(shell uname -s)

ifneq (,$(filter $(CPU),z80 z180)) # matches CPU = z80 or z180
export CROSS_AS=sdasz80
export CROSS_LD=tools/bankld/sdldz80
export CROSS_CC=sdcc
#export CROSS_CCOPTS=-c --std-sdcc99 --no-std-crt0 -mz80 -I$(ROOT_DIR)/cpu-z80 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include --max-allocs-per-node 1000000 --opt-code-size --Werror --stack-auto --constseg CONST
#export CROSS_CCOPTS=-c --std-sdcc99 --no-std-crt0 -mz80 -I$(ROOT_DIR)/cpu-z80 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include --max-allocs-per-node 200000 --opt-code-size --Werror --stack-auto --constseg CONST
export CROSS_CCOPTS=-c --std-sdcc99 --no-std-crt0 -m$(CPU) -I$(ROOT_DIR)/cpu-$(CPU) -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include --max-allocs-per-node 30000 --opt-code-size --Werror --stack-auto --constseg CONST
#export CROSS_CCOPTS+=--nostdlib --nostdinc -Isdcclib/include 
export CROSS_CC_SEG2=--codeseg CODE2
# For now but we are overspilling in a lot of configs so will need a real CODE3
export CROSS_CC_SEG3=--codeseg CODE2
export CROSS_CC_SEGDISC=--codeseg DISCARD --constseg DISCARD
export CROSS_CC_FONT=--constseg FONT
export CROSS_CC_VIDEO=--codeseg VIDEO
export ASOPTS=-plosff
export ASMEXT = .s
export BINEXT = .rel
export BITS=16
#
#	Adjust this as needed for your platform (or contribute a script
#	to look in the usual places !)
#
ifeq ($(SDCC_LIB),)
  ifeq ($(UNAME_S),Darwin)
    export LIBZ80=/usr/local/share/sdcc/lib/$(CPU)
  else
    export LIBZ80=/usr/share/sdcc/lib/$(CPU)
  endif
else
  export LIBZ80=$(SDCC_LIB)/$(CPU)
endif
else ifeq ($(CPU),6502)
export CROSS_AS=ca65
export CROSS_LD=cl65
export CROSS_CC=cl65
export CROSS_CCOPTS=--all-fastcall -c -O -t none -I$(ROOT_DIR)/cpu-6502 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
#
# The 6502 compiler produces what is mostly threadcode and is quite determined
# that the runtime lives in the code segment. As we want the runtime in common
# memory we use SEG1/SEG2 names for all the kernel code.
#
export CROSS_CC_SEG1=--code-name SEG1
export CROSS_CC_SEG2=--code-name SEG2
# 6502 we need a real SEG3 to make it fit
export CROSS_CC_SEG3=--code-name SEG3
export CROSS_CC_SEGDISC=--code-name DISCARD --rodata-name DISCARDDATA
export ASMEXT = .s
export BINEXT = .o
export BITS=16
else ifeq ($(CPU),6809)
export CROSS_AS=m6809-unknown-as
export CROSS_LD=lwlink
export CROSS_CC = m6809-unknown-gcc
#export CROSS_CCOPTS=-Wall -O2 -I$(ROOT_DIR)/cpu-6809 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
export CROSS_CCOPTS=-c -Wall -Os -msoft-reg-count=0 -mfar-stack-param -I$(ROOT_DIR)/cpu-6809 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
export CROSS_CC_SEG1=-mcode-section=.text -mfar-code-page=1
export CROSS_CC_SEG2=-mcode-section=.text2 -mfar-code-page=2
#Given the compactness we don't need a CODE3 segment really
export CROSS_CC_SEG3=-mcode-section=.text2 -mfar-code-page=2
export CROSS_CC_SEGDISC=-mcode-section=.discard -mfar-code-page=3
export CROSS_CC_VIDEO=-mcode-section=.video -mdata-section=.videodata -mfar-code-page=4
export ASOPTS=
export ASMEXT = .s
export BINEXT = .o
export BITS=16
else ifeq ($(CPU),68000)
export CROSS_LD=m68k-linux-gnu-ld
export CROSS_CC = m68k-linux-gnu-gcc
export CROSS_CCOPTS=-c -fno-builtin -Wall -Os -m68000 -mshort -I$(ROOT_DIR)/cpu-68000 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
export CROSS_AS=$(CROSS_CC) $(CROSS_CCOPTS) #-Wa,-M
export CROSS_CC_SEG1=
export CROSS_CC_SEG2=
export CROSS_CC_SEG3=
# Fixme: we should split discard off
export CROSS_CC_SEGDISC=
export CROSS_CC_VIDEO=
export ASOPTS=
export ASMEXT = .S
export BINEXT = .o
export BITS=32
else
export CROSS_LD=ld86
export CROSS_CC = bcc
export CROSS_CCOPTS=-c -ansi -0 -O -I$(ROOT_DIR)/cpu-8086 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include
export CROSS_AS=$(CROSS_CC) $(CROSS_CCOPTS)
export CROSS_CC_SEG1=
export CROSS_CC_SEG2=
export CROSS_CC_SEG3=
# Fixme: we should split discard off
export CROSS_CC_SEGDISC=
export CROSS_CC_VIDEO=
export ASOPTS=
export ASMEXT = .S
export BINEXT = .o
export BITS=16
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


ASRCS =  lowlevel-$(CPU)$(ASMEXT) usermem_std-$(CPU)$(ASMEXT)


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

all:	fuzix.bin

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

tools/analysemap: tools/analysemap.c

tools/bihx: tools/bihx.c

tools/binmunge: tools/binmunge.c

tools/memhogs: tools/analysemap
	cp tools/analysemap tools/memhogs

tools/binman: tools/binman.c

tools/filesizes: tools/filesizes.c

tools/make4x6: tools/make4x6.c

font4x6.c: tools/make4x6
	tools/make4x6 >font4x6.c

tools/decbdragon: tools/decbdragon.c

tools/bintomdv: tools/bintomdv.c

tools/bankld/sdldz80:
	+(cd tools/bankld; make)

cpm-loader/cpmload.bin:	cpm-loader/cpmload.s cpm-loader/makecpmloader.c
	+make -C cpm-loader

tools/makejv3: tools/makejv3.c

ifneq (,$(filter $(CPU),z80 z180)) # matches CPU = z80 or z180
fuzix.ihx: target $(OBJS) platform-$(TARGET)/fuzix.lnk tools/bankld/sdldz80
	$(CROSS_LD) -n -k $(LIBZ80) -f platform-$(TARGET)/fuzix.lnk

fuzix.bin: fuzix.ihx tools/bihx tools/analysemap tools/memhogs tools/binman tools/bintomdv cpm-loader/cpmload.bin
	-cp hogs.txt hogs.txt.old
	tools/memhogs <fuzix.map |sort -nr >hogs.txt
	head -5 hogs.txt
	tools/bihx fuzix.ihx
	tools/binprep
	+make -C platform-$(TARGET) image
else
fuzix.bin: target $(OBJS) tools/decbdragon
	+make -C platform-$(TARGET) image
endif

clean:
	rm -f $(OBJS) $(JUNK) fuzix.cdb fuzix.com fuzix.tmp platform fuzix.bin fuzix.map fuzix.noi fuzix.ihx common.ihx common.bin relocs.dat core *~ include/*~ version.c tools/make4x6 tools/analysemap tools/memhogs tools/binman tools/bihx tools/bintomdv hogs.txt hogs.txt.old tools/*~
	+make -C platform-$(TARGET) clean
	+make -C cpm-loader clean
	+make -C tools/bankld clean

clean-all: clean
	(cd tools/bankld; make clean)
	$(foreach target,$(TARGET_LIST), make -C $(target) clean;)
