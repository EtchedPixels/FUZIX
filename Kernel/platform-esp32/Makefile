export SHELL = bash
export IDF_PATH = $(HOME)/src/esp-idf
SETUP = . $(IDF_PATH)/export.sh

build/fuzix.bin::
	($(SETUP) && idf.py build)

menuconfig::
	($(SETUP) && idf.py menuconfig)

size::
	($(SETUP) && idf.py size)

clean: idf/Makefile
	($(SETUP) && idf.py clean)

