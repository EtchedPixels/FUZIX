/*
 *	Min System 5 style init daemon. Based upon the simple init/login from
 *	UZI180. It's much like a normal sysvinit except the code is a lot
 *	tighter and we integrate the getty/login sequence to reduce size
 *	and disk activity.
 *
 *	Note: once we have select() for tty devices we could conceivably
 *	even avoid forking on a level 2 system until the tty is used (maybe
 *	even to the point of login completion ?)
 *
 *	TODO: (once we have SIGCLD)
 *	- Give serious consideration to hiding cron/at in the daemon
 *	  to keep our background daemon count as low as we can
 *	- Ditto for syslogd
 */

#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <utmp.h>
#include <errno.h>
#include <paths.h>
#include <sys/wait.h>

#define	INIT_OFF	0
#define INIT_SYS	1
#define INIT_BOOT	2
#define INIT_ONCE	3
#define INIT_RESPAWN	4
#define INIT_DEFAULT	5
#define INIT_OPMASK	0x0F
#define INIT_WAIT	0x80
#define MASK_BOOT	0x80

#define MAX_ARGS	16


#define crlf()   write(1, "\n", 1)

static void spawn_login(struct passwd *, const char *, const char *);
static pid_t getty(const char *, const char *);

static struct utmp ut;

/* State that has to persist across an inittab reload */
struct initpid {
	pid_t pid;
	uint8_t id[2];
};

/* Base of our working space */
static uint8_t *membase;
/* Next line to parse */
static uint8_t *snext;
/* Current parse position */
static uint8_t *sdata;
/* End of input */
static uint8_t *sdata_end;
/* Current output ptr */
static uint8_t *idata;
/* Pointer to record start */
static uint8_t *ibackup;
/* PID table */
static struct initpid *initpid;
/* Previous PID table (if reloaded inittab) */
static struct initpid *oldpid;
/* Base of processed data */
static uint8_t *inittab;
/* Number of entries */
static unsigned int initcount;
static unsigned int oldcount;
/* Default runlevel to move to */
static int default_rl;
/* Current run level */
static int runlevel;

volatile static int dingdong;

void sigalarm(int sig)
{
	return;
}

void sigusr1(int sig)
{
	signal(SIGUSR1, sigusr1);
	dingdong = 1;
}

int showfile(char *fname)
{
	int fd, len;
	char buf[80];

	fd = open(fname, O_RDONLY);
	if (fd >= 0) {
		do {
			len = read(fd, buf, 80);
			write(1, buf, len);
		} while (len > 0);
		close(fd);
		return 1;
	}
	return 0;
}

void putstr(char *str)
{
	write(1, str, strlen(str));
}

static pid_t spawn_process(uint8_t * p, uint8_t wait)
{
	static const char *args[MAX_ARGS + 3];
	uint8_t *dp = p + 5;
	uint8_t *ep = p + *p - 1; /* -1 as there is a final \0 */
	pid_t pid;
	int an = 3;

	args[0] = "/bin/sh";
	args[1] = "-c";
	args[2] = dp;
	/* Set pointers to each string */
	while (dp < ep) {
		if (*dp++ == 0 && an < MAX_ARGS)
			args[an++] = dp;
	}
	args[an] = NULL;

	/* Check for internal processes */
	if (strcmp(args[2], "getty") == 0)
		pid = getty(args[3], p + 1);
	else {
		/* External */
		pid = fork();
		if (pid == -1) {
			perror("fork");
			return 0;
		}
		if (pid == 0) {
			/* Child */
			ut.ut_type = INIT_PROCESS;
			ut.ut_pid = getpid();
			ut.ut_id[0] = p[1];
			ut.ut_id[1] = p[2];
			pututline(&ut);
			/* Don't leak utmp into the child */
			endutent();
			/* Run the child */
			execv(args[2], (char**) (args + 2));
			/* If it didn't look binary run it via the shell */
			if (errno == ENOEXEC)
				execv("/bin/sh", (char**) args);
			/* Oh bugger */
			perror(args[2]);
			exit(1);
		}
	}
	/* We need to force utmp closed otherwise we may end up fd sharing
	   with our child and having our lseek() calls messed up. Or maybe
	   it's time to support pread/pwrite ? */
	endutent();
	/* Let it complete if that is the instruction */
	if (wait) {
		while (waitpid(pid, NULL, 0) != pid);
		return 0;
	}
	else
		return pid;
}

/* Get condensed inittab entry pointer from index */

static uint8_t *get_inittab( uint8_t index)
{
	uint8_t *p = inittab;
	uint8_t i;
	for( i = 0; i < index; i++ )
		p += *p; /* add length to p for next entry pointer */
	return p;
}



/*
 *	Clear any dead processes
 */

static int clear_utmp(struct initpid *ip, uint16_t count, pid_t pid)
{
	uint16_t i;
	uint8_t *p = inittab;
	for (i = 0; i < count; i++) {
		if (ip->pid == pid) {
			/* Clear the utmp entry */
			ut.ut_pid = 1;
			ut.ut_type = INIT_PROCESS;
			*ut.ut_line = 0;
			ut.ut_id[0] = p[1];
			ut.ut_id[1] = p[2];
			*ut.ut_user = 0;
			pututline(&ut);
			/* Mark as done */
			initpid[i].pid = 0;
			/* Respawn the task if appropriate */
			p = get_inittab(i);
			if ((p[3] & (1 << runlevel)) && p[4] == INIT_RESPAWN)
				initpid[i].pid = spawn_process(p, 0);
			return 0;
		}
		ip++;
		p += *p;
	}
	return -1;
}

static void clear_zombies(int flags)
{
	pid_t pid = waitpid(-1, NULL, flags);
	/* Interrupted ? */
	if (pid < 0)
		return;
	/* See if we care what died. If we do then also check that
	 * do not need to respawn it
	 */
	 if (clear_utmp(initpid, initcount, pid) == 0)
	 	return;
	if (oldpid)
		clear_utmp(oldpid, oldcount, pid);
}

/*
 *	The init line we are processing turns out to be broken. Move on a line
 *	and rewind the idata pointer to undo the parse of the record
 */
static void bad_line(void)
{
	putstr("inittab error: ");
	snext = strchr(sdata, '\n');
	if (snext)
		*snext++ = 0;
	putstr(sdata);
	sdata = snext;
	idata = ibackup;
}

/*
 *	Turn the character codes 0123456sS into run level numbers
 */
static uint8_t to_runlevel(uint8_t c)
{
	if (c == 's' || c == 'S')
		return 7;		/* 1 << 7 is used for boot/single */
	if (c >=  '0' && c <= '6')
		return c - '0';
	return 0xFF;
}

/*
 *	Squash an init line in situ from
 *
 *	id:runlevels:type:args
 *	into
 *
 *	uint8 length
 *	char id[2]
 *	uint8 runlevel bit mask
 *	uint8 optype
 *	arguments with \0 separation
 */
static void parse_initline(void)
{
	uint8_t bit = 0;
	uint8_t *linelen;

	if (*sdata == '#') {
		sdata = strchr(sdata, '\n');
		if (sdata)
			sdata++;
		return;
	}
	/* We start with a line length then the id: bits. Don't write
	 * the length yet - we may still be using that byte for input */
	linelen = idata++;
	*idata++ = *sdata++;
	*idata++ = *sdata++;	/* Copy the init string */
	if (*sdata++ != ':') {
		bad_line();
		return;
	}
	while (*sdata != ':') {
		if (*sdata == '\n' || sdata > sdata_end) {
			bad_line();
			return;
		}
		bit = to_runlevel(*sdata++);
		if (bit == 0xFF) {
			bad_line();
			return;
		}
		/* Add the run level to the bitmask */
		*idata |= 1 << bit;
	}

	idata++;
	sdata++;
	if (memcmp(sdata, "respawn:", 8) == 0) {
		*idata++ = INIT_RESPAWN;
		sdata += 8;
	} else if (memcmp(sdata, "wait:", 5) == 0) {
		*idata++ = INIT_ONCE | INIT_WAIT;
		sdata += 5;
	} else if (memcmp(sdata, "once:", 5) == 0) {
		*idata++ = INIT_ONCE;
		sdata += 5;
	} else if (memcmp(sdata, "boot:", 5) == 0) {
		idata[-1] = MASK_BOOT;
		*idata++ = INIT_BOOT;
		sdata += 5;
	} else if (memcmp(sdata, "bootwait:", 9) == 0) {
		idata[-1] = MASK_BOOT;
		*idata++ = INIT_BOOT | INIT_WAIT;
		sdata += 9;
	} else if (memcmp(sdata, "off:", 4) == 0) {
		*idata++ = INIT_OFF;
		sdata += 4;
	} else if (memcmp(sdata, "initdefault:", 12) == 0) {
		*idata++ = INIT_DEFAULT;
		default_rl = bit;
		sdata += 12;
	} else if (memcmp(sdata, "sysinit:", 8) == 0) {
		idata[-1] = MASK_BOOT;
		*idata++ = INIT_SYS | INIT_WAIT;
		sdata += 8;
	} else {
		/* We don't yet spport power* methods */
		bad_line();
		return;
	}
	while (*sdata && *sdata != '\n' && sdata < sdata_end) {
		if (*sdata != ' ')
			*idata++ = *sdata;
		else
			*idata++ = 0;
		sdata++;
	}
	/* Terminate the final argument */
	*idata++ = 0;
	*linelen = idata - linelen;
	sdata++;
	initcount++;
}

static void brk_warn(void *p)
{
	if (brk(p) == -1)
		putstr("unable to return space\n");
}

/*
 *	Parse the init table, then set up the pointers after the processed
 *	data, and adjust the brk() value to allow for the tables
 */
static void parse_inittab(void)
{
	idata = inittab = sdata;
	while (sdata < sdata_end)
		parse_initline();
	/* Allocate space for the control arrays */
	initpid = (struct initpid *) idata;
	idata += sizeof(struct initpid) * initcount;
	brk_warn(idata);
	memset(initpid, 0, sizeof(struct initpid) * initcount);
}

/*
 *	Load the inittab into brk space. If it doesn't work then throw a
 *	wobbly and run /bin/sh
 */

static void load_inittab(void)
{
	int fd = open("/etc/inittab", O_RDONLY);
	static struct stat st;
	if (fd == -1 || fstat(fd, &st) == -1 || !S_ISREG(st.st_mode) || !st.st_size) {
		write(2, "init: no inittab\n", 17);
		goto fail;
	}
	/* FIXME: this may unalign our memory pool */
	sdata = sbrk(st.st_size + 1);
	if (sdata == (uint8_t *) - 1) {
		write(2, "init: out of memory\n", 20);
		goto fail;
	}
	if (read(fd, sdata, st.st_size) != st.st_size) {
		write(2, "init: read error\n", 17);
		goto fail;
	}
	close(fd);
	sdata_end = sdata + st.st_size;
	return;
      fail:
	close(fd);
	execl("/bin/sh", "-sh", NULL);
	execl("/bin/ssh", "-ssh", NULL);
	perror("sh");
	_exit(1);
}

static struct initpid *find_id(uint8_t *id)
{
	struct initpid *e = initpid + initcount;
	struct initpid *t = initpid;
	while (t < e) {
		if (id[0] == t->id[0] && id[1] == t->id[1])
			return t;
		t++;
	}
	return NULL;
}

static void reload_inittab(void)
{
	/* Save the old pid table */
	unsigned int size = sizeof(struct initpid) * initcount;
	struct initpid *p = (struct initpid *)membase;
	uint16_t i;

	memmove(membase, initpid, size);
	/* Throw everything else out */
	brk_warn(membase + size);
	/* Save the old pointer and size */
	oldpid = p;
	oldcount = initcount;
	/* Load the new table */
	load_inittab();
	parse_inittab();
	/* We now have the new initspaces all set up and the pointers
	   are valid. We have the old initpid table at membase and we
	   saved the old length in count. We can now transcribe the
	   pid entries and kill off anyone who doesn't belong */
	/* FIXME: we don't kill and restart an entry that has changed but
	   will respawn the new form. There isn't an easy way to fix that
	   without further major reworking */
	for (i = 0; i < oldcount; i++) {
		struct initpid *n = find_id(p->id);
		if (n == NULL && p->pid) {
			/* Deleted line - kill the process group */
			kill(-p->pid, SIGHUP);
			sleep(5);
			kill(-p->pid, SIGKILL);
		} else {
			n->pid = p->pid;
		}
	}
	/* We could shuffle all the pointers back down and free the old
	   pid table, but there is no point. If we do a further reload
	   we will move the current pid table right down and all will
	   be just as good */
}

/*
 *	We are exiting a runlevel. Prune anybody who is running and should
 *	no longer be present. We don't do the cleanup here. We do that
 *	in clear_zombies() or after we return to the main loop.
 */
static int cleanup_runlevel(uint8_t oldmask, uint8_t newmask, int sig)
{
	uint8_t *p = inittab;
	int n = 0;
	int nrun = 0;

	while (n < initcount) {
		/* Dying ? */
		if ((p[3] & oldmask) && !(p[3] && newmask)) {
			/* Count number still to die */
			if (p[4] == INIT_RESPAWN && initpid[n].pid) {
				/* Group kill */
				if (kill(-initpid[n].pid, sig) == 0)
					nrun++;
			}
		}
		/* Next entry */
		p += *p;
		n++;
	}

	return nrun;
}

/*
 *	Clear up the tasks that should not be running. Start with a HUP then
 *	probe them allowing up to 45 seconds, after which we send SIGKILL
 */
static void exit_runlevel(uint8_t oldmask, uint8_t newmask)
{
	uint8_t n = 0;
	if (cleanup_runlevel(oldmask, newmask, SIGHUP)) {
		while (n++ < 9) {
			sleep(5);
			clear_zombies(WNOHANG);
			if (cleanup_runlevel(oldmask, newmask, 0) == 0)
				return;
		}
		cleanup_runlevel(oldmask, newmask, SIGKILL);
	}
}

/*
 *	Start everything that should be runnign at this run level. Take
 *	care not to re-start stuff that survives the transition
 */
static void do_for_runlevel(uint8_t newmask, int op)
{
	uint8_t *p = inittab;
	struct initpid *t = initpid;
	int n = 0;
	while (n < initcount) {
		t->id[0] = p[1];
		t->id[1] = p[2];
		if (!(p[3] & newmask))
			goto next;
		if ((p[4] & INIT_OPMASK) == op) {
			/* Already running ? */
			if (op == INIT_RESPAWN && t->pid)
				goto next;
			/* Spawn and maybe wait for a process */
			t->pid = spawn_process(p, (p[3] & INIT_WAIT));
		}
next:		p += *p;
		t++;
		n++;
	}
}

/*
 *	Launch the one off processes for this run level, then begin
 *	the normal respawn processing.
 */
static void enter_runlevel(uint8_t newmask)
{
	do_for_runlevel(newmask, INIT_ONCE);
	do_for_runlevel(newmask, INIT_RESPAWN);
}

/*
 *	Run through the boot processing from inittab
 */
static void boot_runlevel(void)
{
	do_for_runlevel(MASK_BOOT, INIT_SYS);
	do_for_runlevel(MASK_BOOT, INIT_BOOT);
	runlevel = default_rl;
	enter_runlevel(1 << default_rl);
}


int main(int argc, char *argv[])
{
	int fdtty1;

	signal(SIGINT, SIG_IGN);
	signal(SIGUSR1, sigusr1);

	/* remove any stale /etc/mtab file */

	unlink("/etc/mtab");

	/* clean up anything handed to us by the kernel */
	close(0);
	close(1);
	close(2);

	/* loop until we can open the first terminal */

	do {
		fdtty1 = open("/dev/tty1", O_RDWR|O_NOCTTY);
	} while (fdtty1 < 0);

	/* make stdin, stdout and stderr point to /dev/tty1 */

	if (fdtty1 != 0)
		close(0);
	dup(fdtty1);
	close(1);
	dup(fdtty1);
	close(2);
	dup(fdtty1);

	putstr("init version 0.9.0ac#1\n");

	close(open("/var/run/utmp", O_WRONLY | O_CREAT | O_TRUNC));

	membase = sbrk(0);

	load_inittab();
	parse_inittab();

	boot_runlevel();

	for (;;) {
		clear_zombies(0);
		if (dingdong) {
			uint8_t newrl;
			int fd = open("/var/run/intctl", O_RDONLY);
			if (fd != -1 && read(fd, &newrl, 1) == 1) {
				if (newrl != 'q') {
					exit_runlevel(1 << runlevel, 1 << newrl);
					runlevel = newrl;
					enter_runlevel(1 << runlevel);
				} else {
					/* Reload */
					reload_inittab();
					/* Prune anything running that should
					   not in fact be there */
					exit_runlevel(255, 1 << runlevel);
					/* Start anything added to the current
					   run level */
					enter_runlevel(1 << runlevel);
				}
			}
			close(fd);
			dingdong = 0;
		}
	}
}

/*
 *	Child process helper logic. Use sbrk space to build the environment
 */
static char *env[10];
static int envn;

static void envset(const char *a, const char *b)
{
	int al = strlen(a);
	static char hptr[5];
	/* May unalign the memory pool but we don't care by this point */
	char *tp = sbrk(al + strlen(b) + 2);
	if (tp == (char *) -1) {
		putstr("out of memory.\n");
		return;
	}
	strcpy(tp, a);
	tp[al] = '=';
	strcpy(tp + al + 1, b);
	env[envn++] = tp;
}

/*
 *	Internal implementation of "getty" and "login"
 */
static pid_t getty(const char *ttyname, const char *id)
{
	int fdtty, pid;
	struct passwd *pwd;
	const char *pr;
	char *p, buf[50], salt[3];
	char hn[64];
	gethostname(hn, sizeof(hn));

	for (;;) {
		pid = fork();
		if (pid == -1) {
			putstr("init: can't fork\n");
		} else {
			if (pid != 0)
				/* parent's context: return pid of the child process */
				return pid;

			close(0);
			close(1);
			close(2);
			setpgrp();
			setpgid(0,0);

			/* Throw all the init working spacd we inherited */
			brk_warn(membase);

			fdtty = open(ttyname, O_RDWR);
			if (fdtty < 0)
				return -1;

			/* here we are inside child's context of execution */
			envset("PATH", "/bin:/usr/bin");
			envset("CTTY", ttyname);

			/* make stdin, stdout and stderr point to fdtty */

			dup(fdtty);
			dup(fdtty);

			ut.ut_type = INIT_PROCESS;
			ut.ut_pid = getpid();
			ut.ut_id[0] = id[0];
			ut.ut_id[1] = id[1];
			pututline(&ut);

			/* display the /etc/issue file, if exists */
			showfile("/etc/issue");
			if (*hn) {
				putstr(hn);
				putstr(" ");
			}
			/* loop until a valid user name is entered
			 * and a shell is spawned */

			for (;;) {
				putstr("login: ");
				while (read(0, buf, 20) < 0);	/* EINTR might happens because of the alarm() call below */

				if ((p = strchr(buf, '\n')) != NULL)
					*p = '\0';	/* strip newline */

				pwd = getpwnam(buf);

				if (pwd) {
					if (pwd->pw_passwd[0] != '\0') {
						p = getpass("Password: ");
						salt[0] = pwd->pw_passwd[0];
						salt[1] = pwd->pw_passwd[1];
						salt[2] = '\0';
						pr = crypt(p, salt);
					} else {
						pr = "";
					}
					if (strcmp(pr, pwd->pw_passwd) == 0)
						spawn_login(pwd, ttyname, id);
				}

				putstr("\nLogin incorrect\n\n");
				signal(SIGALRM, sigalarm);
				alarm(2);
				pause();
			}
		}
	}
}


static char *argp[] = { "sh", NULL };

static void spawn_login(struct passwd *pwd, const char *tty, const char *id)
{
	char *p, buf[50];

	/* utmp */
	ut.ut_type = USER_PROCESS;
	ut.ut_pid = getpid();
	strncpy(ut.ut_line, tty, UT_LINESIZE);
	strncpy(ut.ut_id, id, 2);
	time(&ut.ut_time);
	strncpy(ut.ut_user, pwd->pw_name, UT_NAMESIZE);
	pututline(&ut);
	/* Don't leak utmp into the child */
	endutent();

	if (setgid(pwd->pw_gid) == -1 ||
		setuid(pwd->pw_uid) == -1)
			_exit(255);
	signal(SIGINT, SIG_DFL);

	/* setup user environment variables */

	envset("LOGNAME", pwd->pw_name);
	envset("HOME", pwd->pw_dir);
	envset("SHELL", pwd->pw_shell);

	/* home directory */

	if (chdir(pwd->pw_dir))
		putstr("login: unable to change to home directory, using /\n");

	/* show the motd file */

	if (!showfile("/etc/motd"))
		crlf();

	/* and spawn the shell */

	strcpy(buf, "-");
	if ((p = strrchr(pwd->pw_shell, '/')) != NULL)
		strcat(buf, ++p);
	else
		strcat(buf, pwd->pw_shell);

	argp[0] = buf;
	argp[1] = NULL;

	execve(pwd->pw_shell, (void *) argp, (void *) env);
	putstr("login: can't execute shell\n");
	exit(1);
}
