export PICO_SDK_PATH = /home/dg/src/pico/pico-sdk

build/fuzix.elf: build/Makefile
	$(MAKE) -C build

build/Makefile: CMakeLists.txt $(wildcard ../*.[chS]) $(wildcard ../*/*.[chS])
	mkdir -p build
	(cd build && cmake ..)

clean:
	rm -rf build

