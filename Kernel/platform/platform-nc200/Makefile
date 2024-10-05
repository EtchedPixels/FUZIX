CSRCS = \
	devfd.c \
	devlpr.c \
	devtty.c \
	devrd.c \
	devaudio.c \
	devgfx.c \
	devices.c \
	main.c \
	m146818.c \
	bank.c

ASRCS = \
	nc200.s \
	crt0.s \
	tricks.s \
	commonmem.s \
	fdc765.s

COBJS = $(CSRCS:.c=.rel)
AOBJS = $(ASRCS:.s=.rel)
OBJS  = $(COBJS) $(AOBJS)

all: $(OBJS)

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -o $@ -c $<

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) -o $@ $<

clean:
	rm -f $(OBJS) *.lst *.asm *.sym *.rst *.lk  core *~
	rm -f *.rel *.ihx *.map
	rm -f floppyskeleton.img autoprg.bin bootfloppy.img

bootblock.img: bootblock.s
	sdasz80 -fflopzws bootblock.rel bootblock.s
	sdldz80 -nwmx -b BOOTBLOCK=0 -i bootblock.ihx bootblock.rel
	srec_cat -disable-sequence-warning \
                bootblock.ihx -intel \
                -output bootblock.img -binary

.PHONY: image floppyimage cardimage
image: floppyimage cardimage

cardimage: $(ROOT_DIR)/fuzix.bin bootblock.img
	dd if=bootblock.img of=$(ROOT_DIR)/fuzix.bin bs=1 conv=notrunc

floppyimage: cardimage floppyskeleton.img autoprg.bin $(ROOT_DIR)/fuzix.bin
	cp floppyskeleton.img $(FUZIX_ROOT)/Images/$(TARGET)/fuzixboot.img
	truncate $(FUZIX_ROOT)/Images/$(TARGET)/fuzixboot.img --size 7680
	mcopy -i $(FUZIX_ROOT)/Images/$(TARGET)/fuzixboot.img autoprg.bin ::auto.prg
	dd if=$(ROOT_DIR)/fuzix.bin bs=16k skip=0 count=1 | mcopy -i $(FUZIX_ROOT)/Images/$(TARGET)/fuzixboot.img - ::load4000.80
	dd if=$(ROOT_DIR)/fuzix.bin bs=16k skip=1 count=1 | mcopy -i $(FUZIX_ROOT)/Images/$(TARGET)/fuzixboot.img - ::load4000.81
	dd if=$(ROOT_DIR)/fuzix.bin bs=16k skip=2 count=1 | mcopy -i $(FUZIX_ROOT)/Images/$(TARGET)/fuzixboot.img - ::load4000.82
	echo -n | mcopy -i $(FUZIX_ROOT)/Images/$(TARGET)/fuzixboot.img - ::call4100.80

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

diskimage: cardimage floppyimage
	(cd $(FUZIX_ROOT)/Standalone/filesystem-src; ./build-mini-filesystem $(ENDIANFLAGH) $(FUZIX_ROOT)/Images/$(TARGET)/filesys.img 64 1440)
	(cd $(FUZIX_ROOT)/Standalone/filesystem-src; ./build-mini-filesystem $(ENDIANFLAGH) $(FUZIX_ROOT)/Images/$(TARGET)/pcmcia.tmp 64 1408)
	dd if=$(ROOT_DIR)/fuzix.bin bs=327680 conv=sync >pcmcia.tmp2
	cat pcmcia.tmp2 $(FUZIX_ROOT)/Images/$(TARGET)/pcmcia.tmp >$(FUZIX_ROOT)/Images/$(TARGET)/pcmcia.img
	rm -f $(FUZIX_ROOT)/Images/$(TARGET)/pcmcia.tmp pcmcia.tmp2
	dd if=$(FUZIX_ROOT)/Images/$(TARGET)/fuzixboot.img of=$(FUZIX_ROOT)/Images/$(TARGET)/fuzixboot.full bs=737280 conv=sync
	dsktrans -itype raw -format ibm720 $(FUZIX_ROOT)/Images/$(TARGET)/fuzixboot.full $(FUZIX_ROOT)/Images/$(TARGET)/fuzixboot.DSK
	rm -f $(FUZIX_ROOT)/Images/$(TARGET)/fuzixboot.full
	dsktrans -itype raw -format ibm720 $(FUZIX_ROOT)/Images/$(TARGET)/filesys.img $(FUZIX_ROOT)/Images/$(TARGET)/fuzixroot.DSK

