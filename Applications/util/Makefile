CC = sdcc
ASM = sdasz80
AR = sdar
LINKER = sdcc
FCC = ../../Library/tools/fcc

PROGLOAD=`(cat ../../Kernel/platform/config.h; echo PROGLOAD) | cpp -E | tail -n1`

#CC_OPT = -mz80 -c --opt-code-size --std-c89 --max-allocs-per-node 2000000 -I../../Library/include
CC_OPT = -mz80 --std-c99 -c --opt-code-size --max-allocs-per-node 20000 -I../../Library/include

# Used for programs that make sdcc go boom or take hours
CC_CRAP = -mz80 --std-c99 -c --opt-code-size --max-allocs-per-node 1000 -I../../Library/include
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
	dosread.c \
	du.c \
	echo.c \
	ed.c \
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
	pagesize.c \
	passwd.c \
	patchcpm.c \
	printenv.c \
	prtroot.c \
	ps.c \
	pwd.c \
	rm.c \
	rmdir.c \
	sleep.c \
	ssh.c \
	sort.c \
	stty.c \
	sum.c \
	su.c \
	sync.c \
	tee.c \
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
	who.c \
	whoami.c \
	write.c \
	xargs.c \
	yes.c

SRCSBAD = sed.c

OBJS = $(SRCS:.c=.rel)
OBJSBAD = $(SRCSBAD:.c=.rel)

LIBS = ../../Library/libs/syslib.lib

APPS = $(OBJS:.rel=) $(OBJSBAD:.rel=)

all: $(APPS)

$(OBJS): $(SRCS)

$(OBJSBAD): $(SRCSBAD)
	$(CC) $(CC_CRAP) $(@:.rel=.c)

.c.rel:
	$(FCC) -c $<

%: %.rel
	$(FCC) $< -o $@

clean:
	rm -f $(OBJS) $(APPS) $(SRCS:.c=) core *~ *.asm *.lst *.sym *.map *.noi *.lk *.ihx *.tmp

rmbak:
	rm -f *~ core

