# Pull the list of syscalls out of the kernel header.
syscalls = $(shell awk -F '"' '/".*"/ { print $$2 }' < $(TOP)/Kernel/include/syscall_name.h)

syscallsrcs = $(patsubst %,$(OBJ)/Library/libs/fuzix/%.S,$(syscalls))
.SECONDARY: $(syscallsrcs)

libsrcnames = \
	fuzix/syscall.$(ARCH).s \
	__getgrent.c \
	__getpwent.c \
	abort.c \
	asctime.c \
	assert.c \
	atexit.c \
	atoi.c \
	bcmp.c \
	bcopy.c \
	bsearch.c \
	bzero.c \
	calloc.c \
	cfree.c \
	cfspeed.c \
	clock.c \
	closedir.c \
	confstr.c \
	creat.c \
	crypt.c \
	ctime.c \
	ctype.c \
	difftime.c \
	errno.c \
	error.c \
	execl.c \
	execv.c \
	execvp.c \
	exit.c \
	fclose.c \
	fflush.c \
	fgetc.c \
	fgetgrent.c \
	fgetpos.c \
	fgetpwent.c \
	fgets.c \
	fopen.c \
	fprintf.c \
	fputc.c \
	fputs.c \
	fread.c \
	free.c \
	fscanf.c \
	fsetpos.c \
	ftell.c \
	fwrite.c \
	getcwd.c \
	getenv.c \
	getgrgid.c \
	getgrnam.c \
	gethostname.c \
	getopt.c \
	getpw.c \
	getpwnam.c \
	getpwuid.c \
	gets.c \
	gmtime.c \
	grent.c \
	index.c \
	isatty.c \
	itoa.c \
	localtim.c \
	lsearch.c \
	lseek.c \
	lstat.c \
	ltoa.c \
	ltostr.c \
	malloc.c \
	memcpy.c \
	memccpy.c \
	memcmp.c \
	memset.c \
	opendir.c \
	pause.c \
	perror.c \
	popen.c \
	printf.c \
	putenv.c \
	putgetch.c \
	putpwent.c \
	pwent.c \
	qsort.c \
	raise.c \
	rand.c \
	readlink.c \
	realloc.c \
	regerror.c \
	regexp.c \
	regsub.c \
	remove.c \
	revoke.c \
	rewind.c \
	rindex.c \
	scanf.c \
	setbuffer.c \
	setenv.c \
	setvbuf.c \
	sprintf.c \
	sscanf.c \
	stdio0.c \
	strcasecmp.c \
	strcat.c \
	strcmp.c \
	strdup.c \
	stricmp.c \
	strncasecmp.c \
	strnicmp.c \
	strsep.c \
	strtod.c \
	strtol.c \
	tcdrain.c \
	tcflow.c \
	tcflush.c \
	tcgetattr.c \
	tcsetattr.c \
	time.c \
	tmpnam.c \
	ttyname.c \
	tzset.c \
	ungetc.c \
	utsname.c \
	vfprintf.c \
	vfscanf.c \
	vprintf.c \
	vscanf.c \
	vsscanf.c \
	wait.c \
	xitoa.c \
	strchr.c \
	strlen.c \
	strrchr.c \
	strcpy.c \
	
libsrcs = $(patsubst %,$(TOP)/Library/libs/%,$(libsrcnames))
libobjs = $(patsubst $(TOP)/%,$(OBJ)/%.o,$(basename $(libsrcs)))
libsysobjs = $(patsubst %.S,%.o,$(syscallsrcs))

$(LIBC): $(libobjs) $(libsysobjs)

syscallfragment = $(TOP)/Library/libs/fuzix/syscall.$(ARCH).t

$(syscallsrcs): $(syscallfragment) $(OBJ)/Library/libs/fuzix/numbers.h
	@echo SYSCALL $@
	@mkdir -p $(dir $@)
	$(hide) sed -e 's/@NAME@/$(basename $(notdir $@))/g' > $@ < $<

# Emit the header file which maps syscall names to numbers.
$(OBJ)/Library/libs/fuzix/numbers.h: $(TOP)/Kernel/include/syscall_name.h
	@echo NUMBERS $@
	@mkdir -p $(dir $@)
	$(hide) awk -F '"' 'BEGIN { i=0 } /".*"/ { print "__syscall_" $$2 " = " i; i++ }' \
		< $< > $@

