export SHELL = bash
export IDF_PATH = $(HOME)/src/esp-idf
SETUP = cd idf && . $(IDF_PATH)/export.sh

idf/build/fuzix.bin: idf/Makefile
	($(SETUP) && idf.py build)

idf/Makefile: idf/CMakeLists.txt
	($(SETUP) && cmake .)

clean: idf/Makefile
	($(SETUP) && idf.py clean)
	rm idf/CMakeCache.txt

