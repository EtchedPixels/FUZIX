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

int flags;

char *mapstat(char s)
{
    switch (s) {
	case P_ZOMBIE:
	case P_ZOMBIE2: return "Defunct";
	case P_FORKING: return "Forking";
	case P_RUNNING: return "Running";
	case P_READY:   return "Ready";
	case P_SLEEP:   
	case P_XSLEEP:  return "Sleeping";
	case P_PAUSE:   return "Paused";
	case P_WAIT:    return "Waiting";
    }
    return "?";
}

int do_ps(void)
{
    int i, j, uid, pfd, ptsize;
    struct passwd *pwd;
    struct p_tab *pp;
    struct p_tab ptab[PTABSIZE];
    char name[10], uname[20];

    uid = getuid();

    if ((pfd = open("/dev/proc", O_RDONLY)) < 0) {
        perror("ps");
        return 1;
    }
        
    if (ioctl(pfd, 1, (char *) &ptsize) != 0) {
        perror("ioctl");
        close(pfd);
        return 1;
    }
    
    if (ptsize > PTABSIZE) ptsize = PTABSIZE;
    
    for (i = 0; i < ptsize; ++i) {
        if (read(pfd, (char * ) &ptab[i], sizeof(struct p_tab)) !=
                                          sizeof(struct p_tab)) {
            fprintf(stderr, "ps: error reading from /dev/proc\n");
            close(pfd);
            return 1;
        }
    }
    close(pfd);

    if (!(flags & F_h)) {
        if (flags & F_n)
	    printf("  PID\tUID\tSTAT\tWCHAN\tALARM\tCOMMAND\n");
	else
	    printf("USER\t  PID\tSTAT\tWCHAN\tALARM\tCOMMAND\n");
    }

    for (pp = ptab, i = 0; i < ptsize; ++i, ++pp) {
	if (pp->p_status == 0)
	    continue;

	if ((flags & F_r) &&
	    (pp->p_status != P_RUNNING && pp->p_status != P_READY))
	    continue;
	    
	if (!(flags & F_a) && (pp->p_uid != uid))
	    continue;

        strncpy(name, pp->p_name, 8);
        name[8] = '\0';

        for (j = 0; j < 8; ++j) {
            if (name[j] != 0)
                if (name[j] < ' ') name[j] = '?';
        }

	if (flags & F_n) {
	    printf("%5d\t%-3d\t%s\t%04x\t%-5d\t%s\n",
	           pp->p_pid, pp->p_uid,
	           mapstat(pp->p_status), pp->p_wait, pp->p_alarm,
	           name);
	} else {
	    pwd = getpwuid(pp->p_uid);
	    if (pwd)
	        strcpy(uname, pwd->pw_name);
	    else
	        sprintf(uname, "%d", pp->p_uid);
	    printf("%s\t%5d\t%s\t%04x\t%-5d\t%s\n",
	           uname, pp->p_pid,
	           mapstat(pp->p_status), pp->p_wait, pp->p_alarm,
	           name);
	}
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int i;

    flags = 0;
    for (i = 1; i < argc; ++i) {
	char *p = argv[i];

	if (*p == '-') ++p;
	while (*p)
	    switch (*p++) {
		case 'a': flags |= F_a; break;
		case 'h': flags |= F_h; break;
		case 'r': flags |= F_r; break;
		case 'n': flags |= F_n; break;
		default:
		    fprintf(stderr, "usage: ps [-][ahrn]\n");
		    return 1;
	    }
    }

    return do_ps();
}
