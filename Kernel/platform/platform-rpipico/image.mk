fuzix.bin::
	+make -C platform/platform-rpipico image
	cp platform/platform-rpipico/build/fuzix.uf2 $@

