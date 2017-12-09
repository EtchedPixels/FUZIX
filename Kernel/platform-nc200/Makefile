bootfloppy.img: bootblock.img autoprg.bin
	rm -f bootfloppy.img
	cp bootblock.img bootfloppy.img
	truncate bootfloppy.img --size 7680
	mcopy -i bootfloppy.img autoprg.bin ::auto.prg
	mcopy -i bootfloppy.img big ::bank80.img
	mcopy -i bootfloppy.img big ::bank81.img

bootblock.img: bootblock.s
	sdasz80 -fflopzws bootblock.rel bootblock.s
	sdldz80 -nwmx -i bootblock.ihx bootblock.rel
	srec_cat -disable-sequence-warning \
 		bootblock.ihx -intel \
 		-output bootblock.img -binary

autoprg.bin: autoprg.s
	sdasz80 -fflopzws autoprg.rel autoprg.s
	sdldz80 -nwmx -b _CODE=0xa000 -i autoprg.ihx autoprg.rel
	srec_cat -disable-sequence-warning \
 		autoprg.ihx -intel -offset -0xa000 \
 		-output autoprg.bin -binary

clean:
	rm -f bootblock.img autoprg.bin bootfloppy.img
