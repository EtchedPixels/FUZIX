# Pull the list of syscalls out of the kernel header.
syscalls = $(shell awk -F '"' '/".*"/ { print $$2 }' < $(TOP)/Kernel/include/syscall_name.h)

syscallsrcs = $(patsubst %,$(OBJ)/Library/libs/fuzix/%.S,$(syscalls))
.SECONDARY: $(syscallsrcs)

libsrcnames = \
	fuzix/syscall.$(ARCH).s \
	__argv.c \
	__getgrent.c \
	__getpwent.c \
	abort.c \
	asctime.c \
	assert.c \
	atexit.c \
	bcmp.c \
	bcopy.c \
	bsearch.c \
	bzero.c \
	calloc.c \
	cfmakeraw.c \
	cfree.c \
	cfspeed.c \
	clock.c \
	clock_getres.c \
	clock_gettime.c \
	clock_settime.c \
	closedir.c \
	confstr.c \
	creat.c \
	crypt.c \
	ctime.c \
	difftime.c \
	err.c \
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
	getpass.c \
	getpw.c \
	getpwnam.c \
	getpwuid.c \
	gets.c \
	gettimeofday.c \
	gmtime.c \
	gmtime_r.c \
	grent.c \
	index.c \
	isatty.c \
	itoa.c \
	killpg.c \
	libintl.c \
	localtim.c \
	localtim_r.c \
	lsearch.c \
	lseek.c \
	lstat.c \
	ltostr.c \
	malloc.c \
	memccpy.c \
	mkfifo.c \
	nanosleep.c \
	opendir.c \
	pathconf.c \
	pause.c \
	perror.c \
	popen.c \
	printf.c \
	putchar.c \
	putenv.c \
	putgetch.c \
	putpwent.c \
	pwent.c \
	qsort.c \
	raise.c \
	rand.c \
	readdir.c \
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
	setjmp.c \
	setlocale.c \
	settimeofday.c \
	setvbuf.c \
	sleep.c \
	sprintf.c \
	sscanf.c \
	stat.c \
	stdio0.c \
	stime.c \
	strcasecmp.c \
	strcasestr.c \
	strcoll.c \
	strdup.c \
	stricmp.c \
	strlcpy.c \
	strncasecmp.c \
	strnicmp.c \
	strnlen.c \
	strsep.c \
	strxfrm.c \
	sysconf.c \
	system.c \
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
	utent.c \
	utimes.c \
	utsname.c \
	vfprintf.c \
	vfscanf.c \
	vprintf.c \
	vscanf.c \
	vsscanf.c \
	wait.c \
	xitoa.c \
	toupper.c \
	tolower.c \
	toascii.c \
	isascii.c \
	isalnum.c \
	isalpha.c \
	iscntrl.c \
	isdigit.c \
	isgraph.c \
	islower.c \
	ispunct.c \
	isspace.c \
	isupper.c \
	isxdigit.c \
	isblank.c \
	
ifneq ($(WANT_FUZIX_FLOAT),n)

libsrcnames += \
	strtod.c \

endif

ifneq ($(WANT_FUZIX_NUMBERSLIB),n)

libsrcnames += \
	atoi.c \
	ltoa.c \
	strtol.c \

endif

ifneq ($(SDCC_LIBS),)

# Nasty hack --- SDCC's runtime library has optimised versions of a lot of the
# number stuff, *except* strtol, so we need to include it here.

libsrcnames += \
	strtol.c

endif

ifneq ($(WANT_FUZIX_STRINGLIB),n)

libsrcnames += \
	  memmove.c \
      memccpy.c \
      memchr.c \
      memcmp.c \
      memcpy.c \
      memset.c \
      strcasecmp.c \
      strcasestr.c \
      strcat.c \
      strchr.c \
      strcmp.c \
      strcoll.c \
      strcpy.c \
      strcspn.c \
      strdup.c \
      stricmp.c \
      strlcpy.c \
      strlen.c \
      strncasecmp.c \
      strncat.c \
	  strncmp.c \
      strncpy.c \
      strnicmp.c \
      strnlen.c \
      strpbrk.c \
      strrchr.c \
      strsep.c \
      strspn.c \
      strstr.c \
      strtok.c \
      strxfrm.c \

endif

libsrcs = $(patsubst %,$(TOP)/Library/libs/%,$(libsrcnames))
libobjs = $(patsubst $(TOP)/%,$(OBJ)/%.$O,$(basename $(libsrcs)))
libsysobjs = $(patsubst %.S,%.$O,$(syscallsrcs))

$(LIBC): $(libobjs) $(libsysobjs)

syscallfragment = $(TOP)/Library/libs/fuzix/syscall.$(ARCH).t

$(syscallsrcs): $(syscallfragment) $(OBJ)/Library/libs/fuzix/numbers.h
	@echo SYSCALL $@
	@mkdir -p $(dir $@)
	$(hide) sed -e 's/@NAME@/$(basename $(notdir $@))/g' > $@ < $<

# Emit the header file which maps syscall names to numbers.
$(OBJ)/Library/libs/fuzix/numbers.h: $(HOSTOBJ)/Library/tools/mknumbers
	@echo NUMBERS $@
	@mkdir -p $(dir $@)
	$(hide) $< > $@

$(HOSTOBJ)/Library/tools/mknumbers: $(HOSTOBJ)/Library/tools/mknumbers.o
$(HOSTOBJ)/Library/tools/mknumbers.o: private INCLUDES += -I$(TOP)/Kernel/include

