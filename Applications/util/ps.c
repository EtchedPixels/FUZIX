#include <proc.h>
#include <pwd.h>
#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

#define	F_a	0x01		/* all users flag */
#define	F_h	0x02		/* no header flag */
#define	F_r	0x04		/* running only flag */
#define	F_n	0x08		/* numeric output flag */

#define PTABSIZE	128

int flags;

char mapstat(char s)
{
	switch (s) {
	case P_ZOMBIE:
		return 'Z';
	case P_FORKING:
		return 'F';
	case P_RUNNING:
		return 'R';
	case P_READY:
		return 'R';
	case P_SLEEP:
		return 'S';
	case P_IOWAIT:
		return 'D';
	case P_STOPPED:
		return 'T';
	}
	return '?';
}

static struct p_tab_buffer ptab[PTABSIZE];
static int ppid_slot[PTABSIZE];
static int uid;

void print_header(void)
{
	if (!(flags & F_h)) {
		if (flags & F_n)
			printf("  PID\t PPID\t  UID\tSTAT\tWCHAN\tALARM\tCOMMAND\n");
		else
			printf("USER\t  PID\t PPID\tSTAT\tWCHAN\tALARM\tCOMMAND\n");
	}
}

int show_process(struct p_tab *pp)
{
	if (pp->p_status == 0)
		return 0;

	if ((flags & F_r) && (pp->p_status != P_RUNNING && pp->p_status != P_READY))
		return 0;

	if (!(flags & F_a) && (pp->p_uid != uid))
		return 0;

	return 1;
}

void display_process(struct p_tab *pp, int i)
{
	struct passwd *pwd;
	static char name[10], uname[20];
	int j;

	strncpy(name, pp->p_name, 8);
	name[8] = '\0';

	for (j = 0; j < 8; ++j) {
		if (name[j] != 0)
			if (name[j] < ' ')
				name[j] = '?';
	}

	if (flags & F_n) {
		printf("%5d\t%5d\t%5d\t%c\t%04x\t%-5d\t%s\n", pp->p_pid, ptab[ppid_slot[i]].p_tab.p_pid, pp->p_uid, mapstat(pp->p_status), pp->p_wait, pp->p_alarm, name);
	} else {
		pwd = getpwuid(pp->p_uid);
		if (pwd)
			strcpy(uname, pwd->pw_name);
		else
			sprintf(uname, "%d", pp->p_uid);
		printf("%s\t%5d\t%5d\t%c\t%04x\t%-5d\t%s\n", uname, pp->p_pid, ptab[ppid_slot[i]].p_tab.p_pid, mapstat(pp->p_status), pp->p_wait, pp->p_alarm, name);
	}
}

int do_ps(void)
{
	int i, pfd, ptsize, nodesize;
	struct p_tab_buffer *ppbuf;
	struct p_tab *pp;

	uid = getuid();

	if ((pfd = open("/dev/proc", O_RDONLY)) < 0) {
		perror("ps");
		return 1;
	}

	if (ioctl(pfd, 2, (char *) &nodesize) != 0) {
		perror("ioctl");
		close(pfd);
		return 1;
	}

	if (nodesize > sizeof(struct p_tab_buffer)) {
		fprintf(stderr, "kernel/user include mismatch.\n");
		exit(1);
	}

	if (ioctl(pfd, 1, (char *) &ptsize) != 0) {
		perror("ioctl");
		close(pfd);
		return 1;
	}

	if (ptsize > PTABSIZE)
		ptsize = PTABSIZE;

	for (i = 0; i < ptsize; ++i) {
		if (read(pfd, (char *) &ptab[i], nodesize) != nodesize) {
			fprintf(stderr, "ps: error reading from /dev/proc\n");
			close(pfd);
			return 1;
		}
		ppid_slot[i] = ptab[i].p_tab.p_pptr - ptab[0].p_tab.p_pptr;
	}
	close(pfd);

	print_header();

	for (ppbuf = ptab, i = 0; i < ptsize; ++i, ++ppbuf) {
		pp = &ppbuf->p_tab;

		if (!show_process(pp))
			continue;

		display_process(pp, i);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int i;

	flags = 0;
	for (i = 1; i < argc; ++i) {
		char *p = argv[i];

		if (*p == '-')
			++p;
		while (*p)
			switch (*p++) {
			case 'a':
				flags |= F_a;
				break;
			case 'h':
				flags |= F_h;
				break;
			case 'r':
				flags |= F_r;
				break;
			case 'n':
				flags |= F_n;
				break;
			default:
				fprintf(stderr, "usage: ps [-][ahrn]\n");
				return 1;
			}
	}

	return do_ps();
}
