export PICO_SDK_PATH = /home/dg/src/pico/pico-sdk

build/fuzix.elf: build/Makefile
	$(MAKE) -C build

build/Makefile: CMakeLists.txt $(wildcard ../*.[chS]) $(wildcard ../*/*.[chS])
	mkdir -p build
	(cd build && cmake ..)

clean:
	rm -rf build
	$(MAKE) -C ../../Library/libs -f Makefile.rpipico clean
	$(MAKE) -C ../../Applications/util -f Makefile.rpipico clean
	$(MAKE) -C ../../Applications/V7/cmd -f Makefile.rpipico clean
	$(MAKE) -C ../../Applications/V7/cmd/sh -f Makefile.rpipico clean
	$(MAKE) -C ../../Applications/games -f Makefile.rpipico clean
	$(MAKE) -C ../../Applications/levee -f Makefile.rpipico clean

world: build/fuzix.elf
	$(MAKE) -C ../../Library/libs -f Makefile.rpipico
	$(MAKE) -C ../../Applications/util -f Makefile.rpipico
	$(MAKE) -C ../../Applications/V7/cmd -f Makefile.rpipico
	$(MAKE) -C ../../Applications/V7/cmd/sh -f Makefile.rpipico
	$(MAKE) -C ../../Applications/games -f Makefile.rpipico
	$(MAKE) -C ../../Applications/levee -f Makefile.rpipico
