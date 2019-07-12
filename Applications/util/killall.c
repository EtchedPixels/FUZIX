/*
 *	SYS5 style killall
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <proc.h>
#include <fcntl.h>

static pid_t pid, ppid;
static struct p_tab_buffer buf;

static void writes(int fd, const char *p)
{
    write(fd, p, strlen(p));
}

static int kill_pids(int sig)
{
    int fd = open("/dev/proc", O_RDONLY);
    int nodesize;
    int procs;
    int ct = 0;
    int i;

    if (fd == -1) {
        perror("/dev/proc");
        return 255;
    }
    if (ioctl(fd, 2, (char *)&nodesize) != 0) {
        perror("ioctl");
        return 255;
    }
    if (nodesize > sizeof(buf)) {
        writes(2, "killall: mismatch with kernel.\n");
        exit(1);
    }
    if (ioctl(fd, 1, (char *)&procs) != 0) {
        perror("ioctl");
        return 255;
    }
    for (i = 0; i < procs; i++) {
        if (read(fd, (char *)&buf, nodesize) != nodesize) {
            perror("read");
            return 255;
        }
        if (buf.p_tab.p_status != P_EMPTY && buf.p_tab.p_pid != ppid && buf.p_tab.p_pid != pid && buf.p_tab.p_pid != 1) {
            kill(buf.p_tab.p_pid, sig);
            ct++;
        }
    }
    close(fd);
    return ct;
}

int main(int argc, char *argv[])
{
    int sig = SIGTERM;

    if (argc == 2)
        sig = atoi(argv[1]);
    else if (argc > 2) {
        writes(1, "killall [signal]\n");
        exit(1);
    }
    pid = getpid();
    ppid = getppid();
    return kill_pids(sig);
}
