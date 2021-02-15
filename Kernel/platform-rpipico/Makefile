export PICO_SDK_PATH = /home/dg/src/pico/pico-sdk

build/fuzix.elf: build/Makefile
	$(MAKE) -C build

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

world: build/fuzix.elf
	$(MAKE) -C ../../Library/libs -f Makefile.armm0
	$(MAKE) -C ../../Applications/util -f Makefile.armm0
	$(MAKE) -C ../../Applications/V7/cmd -f Makefile.armm0
	$(MAKE) -C ../../Applications/V7/cmd/sh -f Makefile.armm0
	$(MAKE) -C ../../Applications/games -f Makefile.armm0
	$(MAKE) -C ../../Applications/levee -f Makefile.armm0
