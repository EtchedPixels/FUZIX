bootfloppy.img: floppyskeleton.img autoprg.bin ../fuzix.bin
	rm -f bootfloppy.img
	cp floppyskeleton.img bootfloppy.img
	truncate bootfloppy.img --size 7680
	mcopy -i bootfloppy.img autoprg.bin ::auto.prg
	dd if=../fuzix.bin bs=16k skip=0 count=1 | mcopy -i bootfloppy.img - ::load4000.80
	dd if=../fuzix.bin bs=16k skip=1 count=1 | mcopy -i bootfloppy.img - ::load4000.81
	dd if=../fuzix.bin bs=16k skip=2 count=1 | mcopy -i bootfloppy.img - ::load4000.82
	echo -n | mcopy -i bootfloppy.img - ::call4000.80

floppyskeleton.img: floppyskeleton.s
	sdasz80 -fflopzws floppyskeleton.rel floppyskeleton.s
	sdldz80 -nwmx -i floppyskeleton.ihx floppyskeleton.rel
	srec_cat -disable-sequence-warning \
 		floppyskeleton.ihx -intel \
 		-output floppyskeleton.img -binary

autoprg.bin: autoprg.s
	sdasz80 -fflopzws autoprg.rel autoprg.s
	sdldz80 -nwmx -b _CODE=0xa000 -i autoprg.ihx autoprg.rel
	srec_cat -disable-sequence-warning \
 		autoprg.ihx -intel -offset -0xa000 \
 		-output autoprg.bin -binary

clean:
	rm -f floppyskeleton.img autoprg.bin bootfloppy.img
