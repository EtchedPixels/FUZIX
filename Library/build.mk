# Pull the list of syscalls out of the kernel header.
syscalls = $(shell awk -F '"' '/".*"/ { print $$2 }' < $(TOP)/Kernel/include/syscall_name.h)

syscallsrcs = $(patsubst %,$(OBJ)/Library/libs/fuzix/%.S,$(syscalls))
.SECONDARY: $(syscallsrcs)

libsrcnames = \
	fuzix/syscall.$(ARCH).s \
	__argv.c abort.c asctime.c assert.c atexit.c \
	bcmp.c bcopy.c bsearch.c bzero.c calloc.c cfree.c clock.c closedir.c \
	clock_gettime.c clock_getres.c clock_settime.c \
	creat.c crypt.c ctime.c ctype.c difftime.c err.c errno.c error.c \
	execl.c execv.c execvp.c exit.c \
	fclose.c fflush.c fgetc.c fgetgrent.c fgetpwent.c \
	fgetpos.c fgets.c fopen.c fprintf.c fputc.c fputs.c fread.c free.c \
	fsetpos.c ftell.c fwrite.c getcwd.c \
	getenv.c __getgrent.c getgrgid.c getgrnam.c getopt.c \
	getpw.c __getpwent.c getpwnam.c getpwuid.c gets.c gettimeofday.c \
	gmtime.c gmtime_r.c grent.c index.c isatty.c itoa.c killpg.c \
	libintl.c \
	localtim.c localtim_r.c lseek.c lsearch.c lstat.c ltoa.c ltostr.c \
	malloc.c mkfifo.c nanosleep.c opendir.c pause.c perror.c \
	popen.c printf.c putenv.c putgetch.c putpwent.c pwent.c qsort.c \
	raise.c rand.c readdir.c readlink.c realloc.c regerror.c \
	regsub.c remove.c rewind.c rindex.c setbuffer.c setenv.c setjmp.c \
	setlocale.c setvbuf.c settimeofday.c sleep.c sprintf.c  \
	stat.c stdio0.c stime.c \
	strcasecmp.c strcasestr.c strdup.c stricmp.c strlcpy.c strncasecmp.c \
	strnlen.c strnicmp.c strsep.c strxfrm.c strcoll.c \
	strtod.c strtol.c system.c time.c tmpnam.c ttyname.c \
	tzset.c ungetc.c utent.c utimes.c utsname.c \
	vfprintf.c vprintf.c wait.c xitoa.c pathconf.c \
	gethostname.c sysconf.c confstr.c memccpy.c getpass.c \
	tcgetattr.c tcsetattr.c tcdrain.c tcflow.c tcflush.c \
	cfmakeraw.c cfspeed.c revoke.c \
	fscanf.c scanf.c sscanf.c vfscanf.c vscanf.c vsscanf.c \
	regexp.c
	
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
$(OBJ)/Library/libs/fuzix/numbers.h: $(TOP)/Kernel/include/syscall_name.h
	@echo NUMBERS $@
	@mkdir -p $(dir $@)
	$(hide) awk -F '"' 'BEGIN { i=0 } /".*"/ { print "__syscall_" $$2 " = " i; i++ }' \
		< $< > $@

