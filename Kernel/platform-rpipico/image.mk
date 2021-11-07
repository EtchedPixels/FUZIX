fuzix.bin::
	+make -C platform-rpipico image
	cp platform-rpipico/build/fuzix.uf2 $@

