CC = sdcc
ASM = sdasz80
AR = sdar
LINKER = sdcc

PROGLOAD=`(cat ../../Kernel/platform/config.h; echo PROGLOAD) | cpp -E | tail -n1`

#CC_OPT = -mz80 -c --opt-code-size --std-c89 --max-allocs-per-node 2000000 -I../../Library/include
CC_OPT = -mz80 --std-c99 -c --opt-code-size --max-allocs-per-node 20000 -I../../Library/include
#--oldralloc
ASM_OPT = -l -o -s
LINKER_OPT = -mz80 --nostdlib --no-std-crt0 --code-loc $(PROGLOAD) --data-loc  0
BINMAN = ../../Library/tools/binman

.SUFFIXES: .c .rel


SRCS  = banner.c \
	basename.c \
	bd.c \
	cal.c \
	cat.c \
	chgrp.c \
	chmod.c \
	chown.c \
	cksum.c \
	cmp.c \
	cp.c \
	cut.c \
	date.c \
	dd.c \
	decomp16.c \
	df.c \
	dirname.c \
	du.c \
	echo.c \
	ed.c \
	env.c \
	false.c \
	fdisk.c \
	fgrep.c \
	fsck.c \
	grep.c \
	head.c \
	id.c \
	init.c \
	kill.c \
	ll.c \
	ln.c \
	logname.c \
	ls.c \
	man.c \
	mkdir.c \
	mkfs.c \
	mkfifo.c \
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
	sed.c \
	sleep.c \
	ssh.c \
	sort.c \
	stty.c \
	sum.c \
	su.c \
	sync.c \
	tee.c \
	termcap.c \
	tget.c \
	tail.c \
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
	$(LINKER) $(LINKER_OPT) ../../Library/libs/crt0.rel $< $(LIBS) -o $@.ihx
	makebin -s 65536 $@.ihx $@.tmp
	$(BINMAN) $(PROGLOAD) $@.tmp $@.map $@
	chmod +x $@

clean:
	rm -f $(OBJS) $(APPS) $(SRCS:.c=) core *~ *.asm *.lst *.sym *.map *.noi *.lk *.ihx *.tmp

rmbak:
	rm -f *~ core

