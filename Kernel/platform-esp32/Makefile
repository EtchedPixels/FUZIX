export SHELL = bash
export IDF_PATH = $(HOME)/src/esp-idf
SETUP = cd idf && . $(IDF_PATH)/export.sh

idf/build/fuzix.bin::
	($(SETUP) && idf.py build)

clean: idf/Makefile
	($(SETUP) && idf.py clean)
	rm idf/CMakeCache.txt

