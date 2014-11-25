CC = sdcc
ASM = sdasz80
AR = sdar
LINKER = sdcc

<<<<<<< HEAD
CODELOC= 0xC000
=======
PROGLOAD=`(cat ../../Kernel/platform/config.h; echo PROGLOAD) | cpp -E | tail -n1`
>>>>>>> e059da05c2a2aa98ddc3528dd44c116c02b12a91

#CC_OPT = -mz80 -c --opt-code-size --std-c89 --max-allocs-per-node 2000000 -I../../Library/include
CC_OPT = -mz80 --std-c99 -c --opt-code-size --max-allocs-per-node 20000 -I../../Library/include 
# --oldralloc
ASM_OPT = -l -o -s
<<<<<<< HEAD
LINKER_OPT = -mz80 --nostdlib --no-std-crt0 --code-loc $(CODELOC) --data-loc  0
=======
LINKER_OPT = -mz80 --nostdlib --no-std-crt0 --code-loc $(PROGLOAD) --data-loc  0
BINMAN = ../../Library/tools/binman
>>>>>>> e059da05c2a2aa98ddc3528dd44c116c02b12a91

.SUFFIXES: .c .rel


SRCS  = basename.c \
	init.c \
	ssh.c
SRCS_NO  = \
	ssh.c \
	bd.c \
	cal.c \
	cat.c \
	chgrp.c \
	chmod.c \
	chown.c \
	cp.c \
	cut.c \
	date.c \
	dd.c \
	df.c \
	dirname.c \
	du.c \
	echo.c \
	ed.c \
	false.c \
	fsck.c \
	grep.c \
	id.c \
	init.c \
	kill.c \
	ll.c \
	ln.c \
	ls.c \
	mkdir.c \
	mkfs.c \
	mknod.c \
	more.c \
	mount.c \
	mv.c \
	od.c \
	passwd.c \
	patchcpm.c \
	printenv.c \
	prtroot.c \
	ps.c \
	pwd.c \
	rm.c \
	rmdir.c \
	sleep.c \
	su.c \
	sync.c \
	termcap.c \
	tget.c \
	touch.c \
	tr.c \
	true.c \
	umount.c \
	uniq.c \
	uud.c \
	uue.c \
	wc.c \
	which.c \
	whoami.c

OBJS = $(SRCS:.c=.rel)

LIBS = ../../Library/libs/syslib.lib

APPS = $(OBJS:.rel=)

all: $(APPS)

$(OBJS): $(SRCS)

.c.rel:
	$(CC) $(CC_OPT) $(@:.rel=.c)

%: %.rel
<<<<<<< HEAD
	$(LINKER) $(LINKER_OPT) ../../Library/libs/crt0.rel $<  $(LIBS) -o $@.ihex
	makebin -s 65536 $@.ihex $@.tmp
	../../Library/tools/binman $@.tmp $@.map $@
	#dd if=$@.bin of=$@ skip=49152 bs=1
	hex2bin  $@.ihex

clean:
	rm -f $(OBJS) $(APPS) core *~ *.asm *.lst *.sym *.map *.noi *.lk *.ihex *.bin
=======
	$(LINKER) $(LINKER_OPT) ../../Library/libs/crt0.rel $< $(LIBS) -o $@.ihx
	makebin -s 65536 $@.ihx $@.tmp
	$(BINMAN) $(PROGLOAD) $@.tmp $@.map $@
	chmod +x $@

clean:
	rm -f $(OBJS) $(APPS) $(SRCS:.c=) core *~ *.asm *.lst *.sym *.map *.noi *.lk *.ihx *.tmp
>>>>>>> e059da05c2a2aa98ddc3528dd44c116c02b12a91

rmbak:
	rm -f *~ core

