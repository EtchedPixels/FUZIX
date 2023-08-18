#
# Set this to the desired platform to build
#
# Useful values for general work
#
# 2063:		John Winans Z80 Retro system
# 68Knano:	A small retrobrew 68000 platform with IDE disk
# amprolb:	The legendary Ampro Littleboard
# amstradnc/nc100:	Amstrad NC100 (or emulator)
# amstradnc/nc200:	Amstrad NC200 (or emulator)
# coco2cart:	Tandy COCO2 or Dragon with 64K and IDE or SDC + cartridge flash
#		(or xroar emulator )
# coco3:	Tandy COCO3 512K (or MAME)
# cpm22:	Designed for S.100 and similar setups. Uses BIOS plus Z80
#		customisations
# cromemco:	Cromemco with banked memory
# dragon-mooh:	Dragon 32/64 with Mooh 512K card (or xroar emulator)
# dragon-nx32:	Dragon 32/64 with Spinx 512K card (or xroar emulator)
# dyno:		Z180 based platform using RomWBW
# easy-z80:	Easy-Z80 RC2014 compatible system
# esp8266:	ESP8266 module with added SD card
# genie-eg64:	Video genie with a banked memory expander
# kc87:		East German system
# linc80:	Z80 retrobrew system with custom RAM card or mods
# mb020:	68020 single board
# micro80:	Bill Shen's micro80 design
# mini11:	Mini11 68HC11A SBC
# msx1:		MSX1 as a cartridge
# msx2:		MSX2 with 128K or more and MegaFlashROM+SD interface
#		(or OpenMSX suitably configured)
# mtx:		Memotech MTX512 with SDX or SD (or MEMU emulator)
# multicomp09:	Extended multicomp 6809
# n8:		Retrobrew N8 home computer
# nascom:	Nascom 2 or 3 with page mode RAM and CF on PIO
# p112:		DX Designs P112
# pcw8256:	Amstrad PCW series
# pentagon:	Pentagon (spectrum not quite clone)
# pentagon1024: Pentagon 1MB
# pico68k:	Tiny 68K system using 6522/6850 and 128K RAM
# rbc-mark4:	Retrobrew Mark 4 Z180 system
# rbc-minim68k:	Retrobrew Mini 68K system
# rc2014:	RC2014 with 512K RAM/ROM and RTC
# rc2014-tiny:	RC2014 paged ROM system with Fuzix in ROM
# rcbus-6303:	RCBUS with 6303 or 6803 processor card
# rcbus-6502:	RCBUS with 65C02 or 65C816, VIA and 512K RAM/ROM
# rcbus-68008:  RCBUS with 68008 CPU, PPIDE and flat 512/512K memory card
# rcbus-6809:	RCBUS with 6809 CPU card
# rcbus-68hc11:	RCBUS wiht 68HC11 CPU
# rcbus-8085:	RCBUs with 80C85 CPU and 8/56K memory banking
# rcbus-ns32k:   RCBus with NS32K CPU
# rcbus-sbc64:  RCBUS Z80SBC64 128K system and RTC
# rcbus-z180:	RCBUS Z180 systems running in Z180 mode (includes SC126 etc)
# rhyophyre:	Andrew Lynch's rhyohphre Z180/NEC7220 graphics
# riz180:	Bill Shen's RIZ180 128K RAM Z180 platform (Fuzix in ROM)
# rpipico:	Raspberry Pi Pico
# sam:		Sam Coupe
# sbc2g:	Another banked Z80 system
# sbcv2:	RBC/N8VEM SBC v2
# sc108:	Small Computer Central SC108 and SC114 systems
# sc111:	Small Computer Central SC111 system
# sc720:	Small Computer Central SC720 system
# scorpion:	Scorpion 256K (and some relatives) with NemoIDE
# scrumpel:	Scrumpel Z180 system
# searle:	Searle Z80 system with modified ROM and a timer added
# simple80:	Bill Shen's Simple80 with board bugfix and a timer
# smallz80:	Stack180 SmallZ80 system
# socz80:	Will Sowerbutt's FPGA SocZ80 or extended version
# tc2068:	Timex TC2068/TS2068 with DivIDE/DivMMC disk interface
# tiny68k:	Bill Shen's Tiny68K or T68KRC
# tm4c129x:	Texas Instruments Tiva C Series Boards
# to8:		Thomson TO8/TO9+
# tomssbc:	Tom's SBC running in RAM
# tomssbc-rom:	Tom's SBC using the 4x16K banked ROM for the kernel
# trs80:	TRS80 Model 4/4D/4P with 128K RAM (or some other mappers)
# trs80m1:	TRS80 Model I/III with suitable banker (also clones)
# ubee:		Microbee
# v8080:	8080 development using Z80Pack
# vz200:	VZ200 with SDcard/memory
# yaz180:	Yet another Z180 system
# z1013:	East German system
# z180itx:	Z180ITX prototype
# z80-mbc2:	MBC2 Z80 with modern microcontroller as I/O
# z80membership: Z80 membership card
# z80retro:	Peter Wilson's Z80Retro
# z80pack:	Z80Pack virtual Z80 platform
# zeta-v2:	Zeta v2 retrobrew SBC (for Zeta V1 see sbcv2)
# zrc:		Bill Shen's Zrc platform
# zx+3:		ZX Spectrum +3
# zxdiv:	ZX Spectrum 128K with DivIDE/DivMMC interface

TARGET=rc2014

include version.mk

# Get the CPU type
include Kernel/platform-$(TARGET)/target.mk

ifeq ($(USERCPU),)
	USERCPU = $(CPU)
endif

# Base of the build directory
FUZIX_ROOT = $(shell pwd)

# FIXME: we should make it possible to do things entirely without /opt/fcc
PATH := /opt/fcc/bin:$(PATH)
# Add the tools directory
PATH := $(FUZIX_ROOT)/Build/tools/:$(PATH)

# Use Berkeley yacc always (Bison output is too large)
YACC = byacc

# TARGET is what we are building
# CPU is the CPU type for the kernel
# USERCPU is the CPU type for userspace and may be different
export TARGET CPU USERCPU PATH FUZIX_ROOT YACC

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

.PHONY: gtags
gtags:
	gtags

kernel: ltools
	mkdir -p Images/$(TARGET)
	+(cd Kernel; $(MAKE))

diskimage: stand ltools libs apps kernel
	mkdir -p Images/$(TARGET)
	+(cd Standalone/filesystem-src; ./build-filesystem $(ENDIANFLAG) $(FUZIX_ROOT)/Images/$(TARGET)/filesys.img 256 65535)
	+(cd Standalone/filesystem-src; ./build-filesystem $(ENDIANFLAG) $(FUZIX_ROOT)/Images/$(TARGET)/filesys8.img 256 16384)
	+(cd Kernel; $(MAKE) diskimage)

kclean:
	+(cd Kernel; $(MAKE) clean)

clean:
	rm -f GPATH GRTAGS GTAGS
	rm -f Images/$(TARGET)/*.img
	rm -f Images/$(TARGET)/*.DSK
	+(cd Standalone; $(MAKE) clean)
	+(cd Library/libs; $(MAKE) -f Makefile.$(USERCPU) clean)
	+(cd Library; $(MAKE) clean)
	+(cd Kernel; $(MAKE) clean)
	+(cd Applications; $(MAKE) clean)
