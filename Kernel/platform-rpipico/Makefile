export PICO_SDK_PATH = /home/dg/src/pico/pico-sdk

build/fuzix.elf: ../version.c build/Makefile
	$(MAKE) -C build

../version.c:
	$(MAKE) -C .. TARGET=rpipico version.c

build/Makefile: CMakeLists.txt $(wildcard ../*.[chS]) $(wildcard ../*/*.[chS])
	mkdir -p build
	(cd build && cmake ..)

clean:
	rm -rf build
	$(MAKE) -C ../../Library/libs -f Makefile.armm0 clean
	$(MAKE) -C ../../Applications/util -f Makefile.armm0 clean
	$(MAKE) -C ../../Applications/V7/cmd -f Makefile.armm0 clean
	$(MAKE) -C ../../Applications/V7/cmd/sh -f Makefile.armm0 clean
	$(MAKE) -C ../../Applications/games -f Makefile.armm0 clean
	$(MAKE) -C ../../Applications/levee -f Makefile.armm0 clean
	$(MAKE) -C ../../Applications/cave -f Makefile.armm0 clean
	$(MAKE) -C ../../Applications/cursesgames -f Makefile.armm0 clean
	$(MAKE) -C ../../Standalone clean

world: build/fuzix.elf
	$(MAKE) -C ../../Library/libs -f Makefile.armm0
	$(MAKE) -C ../../Applications/util -f Makefile.armm0
	$(MAKE) -C ../../Applications/V7/cmd -f Makefile.armm0
	$(MAKE) -C ../../Applications/V7/cmd/sh -f Makefile.armm0
	$(MAKE) -C ../../Applications/games -f Makefile.armm0
	$(MAKE) -C ../../Applications/levee -f Makefile.armm0
	$(MAKE) -C ../../Applications/cave -f Makefile.armm0
	$(MAKE) -C ../../Applications/cursesgames -f Makefile.armm0
	$(MAKE) -C ../../Standalone

