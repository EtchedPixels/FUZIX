CC = sdcc
ASM = sdasz80
AR = sdar
LINKER = sdldz80
#CC_OPT = -mz80 -c --opt-code-size --std-c99 --max-allocs-per-node 2000000 -I../include
CC_OPT = -mz80 --std-c99 -c --opt-code-size --max-allocs-per-node 20000 -I../include
# for stuff that gives sdcc nightmares
CC_NOOPT = -mz80 --std-c99 -c --opt-code-size --max-allocs-per-node 1000 -I../include
ASM_OPT = -l -o -s
LINKER_OPT = -m -i -o
SRC_CRT0 = crt0.s
OBJ_CRT0 = $(SRC_CRT0:.s=.rel)
SRC_ASM =
OBJ_ASM = $(SRC_ASM:.s=.rel)
SRC_C =  __argv.c abort.c asctime.c assert.c atexit.c
SRC_C += bcmp.c bcopy.c bsearch.c bzero.c calloc.c cfree.c clock.c closedir.c
SRC_C += clock_gettime.c clock_getres.c clock_settime.c
SRC_C += creat.c crypt.c ctime.c ctype.c difftime.c err.c errno.c error.c
SRC_C += execl.c execv.c execvp.c exit.c
SRC_C += fclose.c fflush.c fgetc.c fgetgrent.c fgetpwent.c
SRC_C += fgetpos.c fgets.c fopen.c fprintf.c fputc.c fputs.c fread.c free.c
SRC_C += fsetpos.c ftell.c fwrite.c getcwd.c
SRC_C += getenv.c __getgrent.c getgrgid.c getgrnam.c getopt.c
SRC_C += getpw.c __getpwent.c getpwnam.c getpwuid.c gets.c gettimeofday.c
SRC_C += gmtime.c gmtime_r.c grent.c index.c isatty.c itoa.c killpg.c
SRC_C += localtim.c localtim_r.c lseek.c lsearch.c lstat.c ltoa.c ltostr.c
SRC_C += malloc.c mkfifo.c nanosleep.c opendir.c pause.c perror.c
SRC_C += popen.c printf.c putenv.c putgetch.c putpwent.c pwent.c qsort.c
SRC_C += raise.c rand.c readdir.c readlink.c realloc.c regerror.c
SRC_C += regsub.c remove.c rewind.c rindex.c setbuffer.c setenv.c setjmp.c
SRC_C += setvbuf.c settimeofday.c sleep.c sprintf.c stat.c stdio0.c
SRC_C += strcasecmp.c strcasestr.c strdup.c stricmp.c strlcpy.c strncasecmp.c
SRC_C += strnicmp.c strsep.c
SRC_C += strtod.c strtol.c system.c time.c tmpnam.c ttyname.c
SRC_C += tzset.c ungetc.c utent.c utimes.c utsname.c
SRC_C += vfprintf.c vprintf.c wait.c xitoa.c pathconf.c
SRC_C += gethostname.c sysconf.c confstr.c memccpy.c getpass.c
# tty layer
SRC_C += tcgetattr.c tcsetattr.c tcdrain.c tcflow.c tcflush.c
SRC_C += cfmakeraw.c cfspeed.c revoke.c
# scanf
SRC_C += fscanf.c scanf.c sscanf.c vfscanf.c vscanf.c vsscanf.c
# Seems to give the compiler a hard time
SRC_HARD += regexp.c
# Not supported yet
#SRC_C += initgroups.c
# Pieces we inherit in this case from the compiler library instead
#SRC_C += strcpy.c strlen.c abs.c atof.c atoi.c atol.c labs.c
#SRC_C += strcat.c strchr.c strcmp.c strcspn.c strncat.c strncmp.c
#SRC_C += strncpy.c strpbrk.c strrchr.c strspn.c strstr.c strtok.c
#SRC_C += memchr.c memcmp.c memcpy.c memset.c

OBJ_C = $(SRC_C:.c=.rel)
OBJ_HARD = $(SRC_HARD:.c=.rel)
OBJ_ALL = $(OBJ_ASM) $(OBJ_C) $(OBJ_HARD)

all: syslib.lib crt0.rel

libc.l:%.l:$(OBJ_ALL)
	ls $(OBJ_ALL) > libc.l

syscall.l: fuzix/syslib.l
	../tools/syscall
	(cd fuzix; make)
	cat fuzix/syslib.l | tr " " "\\n" | sed -e "s/^/fuzix\//" >syscall.l

sdccz80.lib:
	../tools/libclean

syslib.lib: syscall.l libc.l sdccz80.lib
	cat libc.l syscall.l >syslib.l
	cp sdccz80.lib syslib.lib
	$(AR) rc syslib.lib @syslib.l
	$(AR) s syslib.lib
	ln -sf syslib.lib c.lib

fuzix/syslib.l:
	../tools/syscall
	make -C fuzix

$(OBJ_ASM):%.rel: %.s
	$(ASM) $(ASM_OPT) $@ $(@:.rel=.s)

$(OBJ_CRT0):%.rel: %.s
	$(ASM) $(ASM_OPT) $@ $(@:.rel=.s)

$(OBJ_C):%.rel: %.c
	$(CC) $(CC_OPT) $(@:.rel=.c)

$(OBJ_HARD):%.rel: %.c
	$(CC) $(CC_NOOPT) $(@:.rel=.c)

clean:
	rm -rf *.rel *.asm *.sym *.lst *.lib *~ syscall.l  libc.l syslib.l
	(cd fuzix; make clean)
