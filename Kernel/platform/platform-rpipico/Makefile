# Set one of these depending.
export PICO_SDK_FETCH_FROM_GIT = yes
#export PICO_SDK_PATH = ${HOME}/.pico-sdk/sdk/1.5.1

# Set 'SUBTARGET' to one of pico-sdk compatible boards
# valid values: pico or pico2
ifndef SUBTARGET
SUBTARGET=pico # set default subtarget
endif

ifeq ($(SUBTARGET),pico)
TOTALMEM=160
endif
ifeq ($(SUBTARGET),pico_w)
TOTALMEM=160
endif
ifeq ($(SUBTARGET),pico2)
TOTALMEM=320
endif
ifeq ($(SUBTARGET),pico2_w)
TOTALMEM=320
endif

PICOTOOLFLAGS= --family rp2040
ifeq (${SUBTARGET}, pico2)
PICOTOOLFLAGS= --family data
endif

PICOTOOL=picotool

ifeq (, $(shell which picotool))
PICOTOOL=build/_deps/picotool/picotool
endif

include ../../../version.mk

build/fuzix.elf: ../../version.c build/Makefile
	$(MAKE) -C build

../../version.c:
	$(MAKE) -C ../.. TARGET=rpipico version.c

build/Makefile: CMakeLists.txt $(wildcard ../../*.[chS]) $(wildcard ../../*/*.[chS])
	mkdir -p build
	(cd build && cmake -DPICO_BOARD=${SUBTARGET} -DTOTALMEM=${TOTALMEM} ..)

image:: world
	./update-flash.sh
	../../../Standalone/mkftl -s 1952 -e 0x1000 -g 10 filesystem.img -o filesystem.ftl
	${PICOTOOL} uf2 convert filesystem.ftl -t bin filesystem.uf2 -o 0x10018000 ${PICOTOOLFLAGS}

diskimage:

clean:
	rm -rf build
	rm -f uf2conv
	$(MAKE) -C ../../../Library/libs -f Makefile.armm0 clean
	$(MAKE) -C ../../../Applications/util -f Makefile.armm0 clean
	$(MAKE) -C ../../../Applications/V7/cmd -f Makefile.armm0 clean
	$(MAKE) -C ../../../Applications/V7/cmd/sh -f Makefile.armm0 clean
	$(MAKE) -C ../../../Applications/games -f Makefile.armm0 clean
	$(MAKE) -C ../../../Applications/levee -f Makefile.armm0 clean
	$(MAKE) -C ../../../Applications/cave -f Makefile.armm0 clean
	$(MAKE) -C ../../../Applications/cursesgames -f Makefile.armm0 clean
	$(MAKE) -C ../../../Standalone clean
	$(MAKE) -C utils clean

world: build/fuzix.elf
	$(MAKE) -C ../../../Library/libs -f Makefile.armm0
	$(MAKE) -C ../../../Applications/util -f Makefile.armm0
	$(MAKE) -C ../../../Applications/V7/cmd -f Makefile.armm0
	$(MAKE) -C ../../../Applications/V7/cmd/sh -f Makefile.armm0
	$(MAKE) -C ../../../Applications/games -f Makefile.armm0
	$(MAKE) -C ../../../Applications/levee -f Makefile.armm0
	$(MAKE) -C ../../../Applications/cave -f Makefile.armm0
	$(MAKE) -C ../../../Applications/cursesgames -f Makefile.armm0
	$(MAKE) -C ../../../Standalone
	$(MAKE) -C utils
