/* printroot - print root device on stdout	Author: Bruce Evans */

/* This program figures out what the root device is by doing a stat on it, and
 * then searching /dev until it finds an entry with the same device number.
 * A typical use (probably the only use) is in /etc/rc for initializing
 * /etc/mtab, as follows:
 *
 *	/usr/bin/printroot >/etc/mtab
 *
 *  9 Dec 1989	- clean up for 1.5 - full prototypes (BDE)
 * 15 Oct 1989	- avoid ACK cc bugs (BDE):
 *		- sizeof "foo" is 2 (from wrong type char *) instead of 4
 *		- char foo[10] = "bar"; allocates 4 bytes instead of 10
 *  1 Oct 1989	- Minor changes by Andy Tanenbaum
 *  5 Oct 1992	- Use readdir (kjb)
 * 26 Nov 1994	- Flag -r: print just the root device (kjb)
 * 19 May 2001  - Ported to UZI180
 *  3 Feb 2015  - Flag -i: initialise the /etc/mtab file (WRS)
 * 21 May 2015  - Report the rootfs as fuzix not uzi (AC)
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#define DEV_PATH	"/dev/"
#define MESSAGE		" / fuzix rw 0 0\n"
#define UNKNOWN_DEV	"/dev/unknown"
#define ROOT		"/"

int rflag = 0;
int iflag = 0;

void done(const char *name, int status)
{
    int fd;

    if(iflag) {
        fd = open("/etc/mtab", O_CREAT | O_TRUNC | O_WRONLY, 0644);
        if(fd < 0){
            perror("prtroot: cannot open /etc/mtab: ");
            exit(1);
        }
    } else { /* !iflag */
        fd = 1;
    }

    write(fd, name, strlen(name));
    if (rflag)
        write(fd, "\n", 1);
    else
        write(fd, MESSAGE, sizeof(MESSAGE)-1); /* do not write the terminating NUL */

    exit(status);
}

int main(int argc, const char *argv[])
{
    static DIR dp;
    int i;
    struct dirent *entry;
    struct stat filestat, rootstat;
    static char namebuf[sizeof(DEV_PATH) + MAXNAMLEN + 1];

    for(i=1; i<argc; i++){
        if(strcmp(argv[i], "-r") == 0)
            rflag = 1;
        else if(strcmp(argv[i], "-i") == 0)
            iflag = 1;
    }

    if (stat(ROOT, &rootstat) == 0
	&& opendir_r(&dp,DEV_PATH) != (DIR *) NULL) {
	while ((entry = readdir(&dp)) != (struct dirent *) NULL) {
	    strcpy(namebuf, DEV_PATH);
	    strlcat(namebuf, entry->d_name, sizeof(namebuf));
	    if (stat(namebuf, &filestat) != 0)
		continue;
	    if (!S_ISBLK(filestat.st_mode))
		continue;
	    if (filestat.st_rdev != rootstat.st_dev)
		continue;
	    done(namebuf, 0);
	}
    }
    done(UNKNOWN_DEV, 1);
    return 0;			/* not reached */
}

