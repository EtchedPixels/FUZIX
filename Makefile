# Set this to the desired platform to build
#
# Useful values for general work
#
# amstradnc/nc100:	Amstrad NC100 (or emulator)
# amstradnc/nc200:	Amstrad NC100 (or emulator)
# coco2cart:	Tandy COCO2 or Dragon with 64K and IDE or SDC + cartridge flash
#		(or modified xroar)
# coco3:	Tandy COCO3 512K (or MAME)
# dragon-nx32:	Dragon 32/64 with Spinx 512K card (or modified xroar)
# msx2:		MSX2 with 128K or more and MegaFlashROM+SD interface
#		(or OpenMSX suitably configured)
# mtx:		Memotech MTX512 with SDX (or MEMU emulator)
# multicomp09:	Extended multicomp 6809
# n8vem-mark4:	RBC/N8VEM Retrobrew Z180 board
# p112:		DX Designs P112
# rc2014:	RC2014 with 512K RAM/ROM and RTC
# rc2014-sbc64: RC2014 Z80SBC64 128K system and RTC
# rc2014-tiny:	RC2014 with 64KK RAM, banked ROM and RTC
# sam:		Sam Coupe
# sbcv2:	RBC/N8VEM SBC v2
# socz80:	Will Sowerbutt's FPGA SocZ80 or extended version
# tc2068:	Timex TC2068/TS2068 with DivIDE/DivMMC disk interface
# tiny68k:	Bill Shen's Tiny68K (development only)
# trs80:	TRS80 Model 4/4D/4P with 128K RAM (or some other mappers)
# trs80m1:	TRS80 Model I/III with suitable banker (also clones)
# ubee:		Microbee
# z80pack:	Z80Pack virtual Z80 platform
# zeta-v2:	Zeta retrobrew SBC
# zx+3:		ZX Spectrum +3
# zxdiv:	ZX Spectrum 128K with DivIDE/DivMMC interface
#
# Virtual platforms for in progress development work
#
# v65:		Virtual platform for 6502 development (dead)
# v65c816:	Virtual platform for 65c816 development
# v68:		Virtual platform for 68000 development

TARGET=z80pack

# Get the CPU type
include Kernel/platform-$(TARGET)/target.mk

ifeq ($(USERCPU),)
	USERCPU = $(CPU)
endif

# FIXME: we should make it possible to do things entirely without /opt/fcc
PATH := /opt/fcc/bin:$(PATH)

# TARGET is what we are building
# CPU is the CPU type for the kernel
# USERCPU is the CPU type for userspace and eventually may be different
# (eg for 65c816 with 6502 user)
export TARGET CPU USERCPU PATH

# FUZIX_CCOPTS is the global CC optimization level
ifeq ($(FUZIX_CCOPTS),)
	FUZIX_CCOPTS = -O2
endif
export FUZIX_CCOPTS

all: stand ltools libs apps kernel

stand:
	+(cd Standalone; $(MAKE))

ltools:
	+(cd Library; $(MAKE); $(MAKE) install)

libs: ltools
	+(cd Library/libs; $(MAKE) -f Makefile.$(USERCPU); \
		$(MAKE) -f Makefile.$(USERCPU) install)

apps: libs
	+(cd Applications; $(MAKE))

kernel: ltools
	+(cd Kernel; $(MAKE))

kclean:
	+(cd Kernel; $(MAKE) clean)

clean:
	+(cd Standalone; $(MAKE) clean)
	+(cd Library/libs; $(MAKE) -f Makefile.$(USERCPU) clean)
	+(cd Library; $(MAKE) clean)
	+(cd Kernel; $(MAKE) clean)
	+(cd Applications; $(MAKE) clean)
