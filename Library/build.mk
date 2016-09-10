$(call find-makefile)

libc-functions.localsrcs += \
	__argv.c \
	__getgrent.c \
	__getpwent.c \
	abort.c \
	abs.c \
	asctime.c \
	assert.c \
	atexit.c \
	atoi.c \
	atof.c \
	atol.c \
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
	closedir_r.c \
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
	fork.c \
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
	getloadavg.c \
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
	initgroups.c \
	isalnum.c \
	isalpha.c \
	isascii.c \
	isatty.c \
	isblank.c \
	iscntrl.c \
	isdigit.c \
	isgraph.c \
	islower.c \
	isprint.c \
	ispunct.c \
	isspace.c \
	isupper.c \
	isxdigit.c \
	killpg.c \
	labs.c \
	libintl.c \
	localtim.c \
	localtim_r.c \
	lsearch.c \
	lseek.c \
	lstat.c \
	ltoa.c \
	ltostr.c \
	malloc.c \
	memmove.c \
	mkfifo.c \
	mkstemps.c \
	nanosleep.c \
	opendir.c \
	opendir_r.c \
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
	setlocale.c \
	settimeofday.c \
	setvbuf.c \
	sleep.c \
	sprintf.c \
	sscanf.c \
	stat.c \
	stdio0.c \
	stime.c \
	strncmp.c \
	strsignal.c \
	strtod.c \
	strtol.c \
	strxfrm.c \
	sysconf.c \
	system.c \
	tcdrain.c \
	tcflow.c \
	tcflush.c \
	tcgetattr.c \
	tcsetattr.c \
	telldir.c \
	time.c \
	tmpfile.c \
	tmpnam.c \
	tolower.c \
	toupper.c \
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
    memccpy.c \
    memchr.c \
    memcmp.c \
    memcpy.c \
    memset.c \
    seekdir.c \
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
    strncpy.c \
    strnicmp.c \
    strnlen.c \
    strpbrk.c \
    strrchr.c \
    strsep.c \
    strspn.c \
    strstr.c \
    strtok.c \
	$(SYSCALL_STUB)

libc-functions.srcs = $(addprefix libs/, \
	$(filter-out $(libc-functions.omit), $(libc-functions.localsrcs)))
libc-functions.extradeps = $(MAKEFILE)
$(call build, libc-functions, target-lib)


$(SYSCALL_GENERATOR).srcs = tools/$(SYSCALL_GENERATOR).c
$(SYSCALL_GENERATOR).includes = -I$(TOP)/Kernel/include
$(call build, $(SYSCALL_GENERATOR), host-exe)


# Pull the list of syscalls out of the kernel header.
syscalls = $(shell awk -F '"' '/".*"/ { print $$2 }' < $(TOP)/Kernel/include/syscall_name.h)

syscalldir = $(dir $(SYSCALL_STUB))
libc-syscalls.objdir = $(OBJ)/$(PLATFORM)/libc-syscalls
libc-syscalls.srcs = $(patsubst %, $(libc-syscalls.objdir)/$(syscalldir)/syscall_%.s, $(syscalls))
libc-syscalls.objs = $(patsubst %, $(libc-syscalls.objdir)/$(syscalldir)/syscall_%.$O, $(syscalls))
$(call build, libc-syscalls, target-lib)

$(libc-syscalls.srcs): $(libc-syscalls.objdir)/syscalls-made
$(libc-syscalls.objdir)/syscalls-made: $($(SYSCALL_GENERATOR).result)
	@echo SYSCALLS $@
	@mkdir -p $(libc-syscalls.objdir)/$(syscalldir)
	$(hide) (cd $(libc-syscalls.objdir) && $(abspath $^))
	$(hide) touch $@


# The actual libc is the functions library with the syscalls library merged in
# on top.

libc.srcs = $(libc-functions.result) $(libc-syscalls.result) $(PLATFORM_EXTRA_LIBC)
$(call build, libc, target-lib)


# The crt object file (built and linked in seperately).

crt0.srcs = libs/$(CRT)
crt0.result = $(crt0.objdir)/Library/libs/$(CRT:.s=.$O)
$(call build, crt0, target-lib)


# Helper tool used to construct Fuzix executables.

binman.srcs = tools/binman.c
$(call build, binman, host-exe)


# Helper tool to generate the platform liberror.

liberror-gen.srcs = tools/liberror.c
$(call build, liberror-gen, host-exe)

liberror.ext = txt
liberror.srcs = $(liberror-gen.result)
$(call build, liberror, nop)
$(liberror.result): $(liberror-gen.result)
	@echo LIBERROR $@
	@mkdir -p $(dir $@)
	$(hide) $(liberror-gen.result) $(liberror.flags) > $(liberror.result)

