$(call find-makefile)

# Pull the list of syscalls out of the kernel header.
syscalls = $(shell awk -F '"' '/".*"/ { print $$2 }' < $(TOP)/Kernel/include/syscall_name.h)

libsrcnames = \
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
	killpg.c \
	libintl.c \
	localtim.c \
	localtim_r.c \
	lsearch.c \
	lseek.c \
	lstat.c \
	ltostr.c \
	malloc.c \
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
	isprint.c \
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

libsrcnames += $(SYSCALL_STUB)

libc.srcs = $(addprefix libs/, $(libsrcnames))
$(call build, libc, target-lib)

$(SYSCALL_GENERATOR).srcs = tools/$(SYSCALL_GENERATOR).c
$(SYSCALL_GENERATOR).includes = -I$(TOP)/Kernel/include
$(call build, $(SYSCALL_GENERATOR), host-exe)

libsyscalls.objdir = $(OBJ)/$(PLATFORM)/syscalls
libsyscalls.srcs = $(patsubst %, $(libsyscalls.objdir)/fuzix/syscall_%.s, $(syscalls))
libsyscalls.objs = $(patsubst %, $(libsyscalls.objdir)/fuzix/syscall_%.$O, $(syscalls))
$(call build, libsyscalls, target-lib)

# We have to use a pattern in this rule, because accurséd make only supports
# rules with multiple outputs in pattern rules. If we just used
# $(libsyscalls.srcs) as the source it would run the rule once for every
# syscall.
$(libsyscalls.objdir)/fuzix/%.s: $($(SYSCALL_GENERATOR).exe)
	@echo SYSCALLS $@
	@mkdir -p $(libsyscalls.objdir)/fuzix
	$(hide) (cd $(libsyscalls.objdir) && $(abspath $^))

