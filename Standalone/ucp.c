/**************************************************
  UZI (Unix Z80 Implementation) Utilities:  ucp.c
Modifications:
14 June 1998 - Reformatted, restructured command
switch, sense Ctrl-Z in type.   HFB
21 Sept 1999 - Corrected the 'constant expression'
problem, added some missing breaks.
HP
 ***************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <libgen.h>
#include "fuzix_fs.h"

#define UCP_VERSION  "1.1ac"

int16_t *syserror = (int16_t*)&udata.u_error;
static char cwd[100];
static char line[128];
char *nextline = NULL;
char *month[] =
{ "Jan", "Feb", "Mar", "Apr",
    "May", "Jun", "Jul", "Aug",
    "Sep", "Oct", "Nov", "Dec" };

int swizzling = 0;

int match(char *cmd);
void usage(void);
void prmode(int mode);
int ls(char *path);
int chmod(char *modes, char *path);
int mknod( char *path, char *modes, char *devs);
int mkdir(char *path);
int get( char *arg, int binflag);
int put( char *arg, int binflag);
int type( char *arg);
int fdump(char *arg);
int unlink( char *path);
int rmdir(char *path);

int main(argc, argval)
    int argc;
    char *argval[];
{
    int  rdev;
    char cmd[30], arg1[30], arg2[30], arg3[30];
    int  count;
    int  interactive;
    int  pending_line = 0;
    struct filesys fsys;
    int  j, retc;
    /*--    char *argv[5];--*/

    /*
       if (argc < 2)
       rdev = 0;
       else
       rdev = atoi(argval[1]);
       */
    if (argc == 2) {
        fd_open(argval[1]);
        interactive = 1;
    } else if (argc == 3) {
        fd_open(argval[1]);
        strncpy(&line[0], argval[2], 127);
        line[127] = '\0';
        interactive = 0;
    } else {
        printf("Usage: ucp FILE [COMMAND]\n");
        exit(1);
    }

    rdev = 0;

    xfs_init(rdev);
    strcpy(cwd, "/");

    printf("Fuzix UCP version " UCP_VERSION ". Type ? for help.\n");

    do {
        if (interactive && !pending_line) {
            printf("unix: ");
            if (fgets(line, 128, stdin) == NULL) {
                xfs_end();
                exit(1);
            }
        }

        if (!pending_line) {
            nextline = strchr(&line[0], ';');
	    if (nextline != NULL) {
                nextline[0] = '\0';
                nextline++;
            }
        }

        cmd[0] = '\0';
        *arg1 = '\0';
        arg2[0] = '\0';
        arg3[0] = '\0';

        if (pending_line) {
            count = sscanf(nextline, "%s %s %s %s", cmd, arg1, arg2, arg3);
            nextline = NULL;
            pending_line = 0;
            if (count == 0 || cmd[0] == '\0')
                continue;
        } else {
            count = sscanf(line, "%s %s %s %s", cmd, arg1, arg2, arg3);
            if (nextline != NULL) {
                pending_line = 1;
            }
            if (count == 0 || cmd[0] == '\0')
                continue;
        }

        _sync();

        if (strcmp(cmd, "\n") == 0)
            continue;
        switch (match(cmd)) {
            case 0:         /* exit */
                xfs_end();
                exit(1);

            case 1:         /* ls */
                if (*arg1)
                    retc = ls(arg1);
                else
                    retc = ls(".");
                break;

            case 2:         /* cd */
                if (*arg1) {
                    strcpy(cwd, arg1);
                    if ((retc = _chdir(arg1)) != 0) {
                        printf("cd: error number %d\n", *syserror);
                    }
                }
                break;

            case 3:         /* mkdir */
                if (*arg1)
                    retc = mkdir(arg1);
                break;

            case 4:         /* mknod */
                if (*arg1 && *arg2 && *arg3)
                    retc = mknod(arg1, arg2, arg3);
                break;

            case 5:         /* chmod */
                if (*arg1 && *arg2)
                    retc = chmod(arg1, arg2);
                break;

            case 6:         /* get */
                if (*arg1)
                    retc = get(arg1, 0);
                break;

            case 7:         /* bget */
                if (*arg1)
                    retc = get(arg1, 1);
                break;

            case 8:         /* put */
                if (*arg1)
                    retc = put(arg1, 0);
                break;

            case 9:         /* bput */
                if (*arg1)
                    retc = put(arg1, 1);
                break;

            case 10:        /* type */
                if (*arg1)
                    retc = type(arg1);
                break;

            case 11:        /* dump */
                if (*arg1)
                    retc = fdump(arg1);
                break;

            case 12:        /* rm */
                if (*arg1)
                    retc = unlink(arg1);
                break;

            case 13:        /* df */
                for (j = 0; j < 4; ++j) {
                    retc = _getfsys(j, (char*)&fsys);
                    if (retc == 0 && fsys.s_mounted) {
                        printf("%d:  %u blks used, %u free;  ", j,
                                (fsys.s_fsize - fsys.s_isize) - fsys.s_tfree,
                                fsys.s_tfree);
                        printf("%u inodes used, %u free\n",
                                (8 * (fsys.s_isize - 2) - fsys.s_tinode),
                                fsys.s_tinode);
                    }
                }
                break;

            case 14:        /* rmdir */
                if (*arg1)
                    retc = rmdir(arg1);
                break;

            case 15:        /* mount */
                if (*arg1 && *arg2)
                    if ((retc = _mount(arg1, arg2, 0)) != 0) {
                        printf("Mount error: %d\n", *syserror);
                    }
                break;

            case 16:        /* umount */
                if (*arg1)
                    if ((retc = _umount(arg1)) != 0) {
                        printf("Umount error: %d\n", *syserror);
                    }
                break;

            case 50:        /* help */
                usage();
                retc = 0;
                break;

            default:        /* ..else.. */
                printf("Unknown command, type ? for help.\n");
                retc = -1;
                break;
        }           /* End Switch */
    } while (interactive || pending_line);

    _sync();

    return retc;
}


int match(char *cmd)
{
    if (strcmp(cmd, "exit") == 0)
        return (0);
    else if (strcmp(cmd, "quit") == 0)
        return (0);
    else if (strcmp(cmd, "ls") == 0)
        return (1);
    else if (strcmp(cmd, "dir") == 0)
        return (1);
    else if (strcmp(cmd, "cd") == 0)
        return (2);
    else if (strcmp(cmd, "mkdir") == 0)
        return (3);
    else if (strcmp(cmd, "mknod") == 0)
        return (4);
    else if (strcmp(cmd, "chmod") == 0)
        return (5);
    else if (strcmp(cmd, "get") == 0)
        return (6);
    else if (strcmp(cmd, "bget") == 0)
        return (7);
    else if (strcmp(cmd, "put") == 0)
        return (8);
    else if (strcmp(cmd, "bput") == 0)
        return (9);
    else if (strcmp(cmd, "type") == 0)
        return (10);
    else if (strcmp(cmd, "cat") == 0)
        return (10);
    else if (strcmp(cmd, "dump") == 0)
        return (11);
    else if (strcmp(cmd, "rm") == 0)
        return (12);
    else if (strcmp(cmd, "df") == 0)
        return (13);
    else if (strcmp(cmd, "rmdir") == 0)
        return (14);
    else if (strcmp(cmd, "mount") == 0)
        return (15);
    else if (strcmp(cmd, "umount") == 0)
        return (16);
    else if (strcmp(cmd, "help") == 0)
        return (50);
    else if (strcmp(cmd, "?") == 0)
        return (50);
    else
        return (-1);
}


void usage(void)
{
    printf("UCP commands:\n");
    printf("?|help\n");
    printf("exit|quit\n");
    printf("dir|ls [path]\n");
    printf("cd path\n");
    printf("mkdir dirname\n");
    printf("mknod name mode dev#\n");
    printf("chmod mode path\n");
    printf("[b]get cpmfile\n");
    printf("[b]put uzifile\n");
    printf("type|cat filename\n");
    printf("dump filename\n");
    printf("rm path\n");
    printf("rmdir dirname\n");
    printf("df\n");
    printf("mount dev# path\n");
    printf("umount path\n");
}

void prmode(int mode)
{
    if (mode & 4)
        printf("r");
    else
        printf("-");

    if (mode & 2)
        printf("w");
    else
        printf("-");

    if (mode & 1)
        printf("x");
    else
        printf("-");
}

int ls(char *path)
{
    struct direct buf;
    struct uzi_stat statbuf;
    char dname[128];
    int d, st;

    /*
       if (_stat(path, &statbuf) != 0 || (statbuf.st_mode & F_MASK) != F_DIR) {
       printf("ls: can't stat %s\n", path);
       return -1;
       }
       */

    d = _open(path, 0);
    if (d < 0) {
        printf("ls: can't open %s\n", path);
        return -1;
    }
    while (_read(d, (char *) &buf, 16) == 16) {
        if (buf.d_name[0] == '\0')
            continue;

        if (path[0] != '.' || path[1]) {
            strcpy(dname, path);
            strcat(dname, "/");
        } else {
            dname[0] = '\0';
        }

        strcat(dname, buf.d_name);

        if (_stat(dname, &statbuf) != 0) {
            printf("ls: can't stat %s\n", dname);
            break;
        }
        st = (statbuf.st_mode & F_MASK);

        if ((st & F_MASK) == F_DIR) /* & F_MASK is redundant */
            printf("d");
        else if ((st & F_MASK) == F_CDEV)
            printf("c");
        else if ((st & F_MASK) == F_BDEV)
            printf("b");
        else if ((st & F_MASK) == F_PIPE)
            printf("p");
        else if ((st & F_REG) == 0)
            printf("l");
        else
            printf("-");

        prmode(statbuf.st_mode >> 6);
        prmode(statbuf.st_mode >> 3);
        prmode(statbuf.st_mode);

        printf("%4d %5d", statbuf.st_nlink, statbuf.st_ino);

        if ((statbuf.st_mode & F_MASK) == F_DIR)
            strcat(dname, "/");

        printf("%12u ",
                (statbuf.st_mode & F_CDEV) ?
                statbuf.st_rdev :
                statbuf.st_size);

        if (statbuf.fst_mtime == 0) { /* st_mtime? */
            /*printf("--- -- ----   --:--");*/
            printf("                   ");
        } else {
            time_t t = statbuf.fst_mtime;
            struct tm *tm = gmtime(&t);
            printf("%s %02d %4d   ",
                    month[tm->tm_mon],
                    tm->tm_mday,
                    tm->tm_year);

            printf("%2d:%02d",
                    tm->tm_hour,
                    tm->tm_min);
        }

        printf("  %-15s\n", dname);
    }
    _close(d);
    return 0;
}

int chmod(char *modes, char *path)
{
    int mode;

    printf("chmod %s to %s\n", path, modes);
    mode = -1;
    sscanf(modes, "%o", &mode);
    if (mode == -1) {
        printf("chmod: bad mode\n");
        return (-1);
    }
    /* Preserve the type if not specified */
    if (mode < 10000) {
        struct uzi_stat st;
        if (_stat(path, &st) != 0) {
            printf("chmod: can't stat file %d\n", *syserror);
            return -1;
        }
        mode = (st.st_mode & ~0x7777) | mode;
    }
    if (_chmod(path, mode)) {
        printf("chmod: error %d\n", *syserror);
        return (-1);
    }
    return 0;
}


int mknod( char *path, char *modes, char *devs)
{
    int mode;
    int dev;

    mode = -1;
    sscanf(modes, "%o", &mode);
    if (mode == -1) {
        printf("mknod: bad mode\n");
        return (-1);
    }
    if ((mode & F_MASK) != F_BDEV && (mode & F_MASK) != F_CDEV) {
        printf("mknod: mode is not device\n");
        return (-1);
    }
    dev = -1;
    sscanf(devs, "%d", &dev);
    if (dev == -1) {
        printf("mknod: bad device\n");
        return (-1);
    }
    if (_mknod(path, mode, dev) != 0) {
        printf("_mknod: error %d\n", *syserror);
        return (-1);
    }
    return (0);
}



int mkdir(char *path)
{
    char dot[100];

    if (_mknod(path, 040000 | 0777, 0) != 0) {
        printf("mkdir: mknod error %d\n", *syserror);
        return (-1);
    }
    strcpy(dot, path);
    strcat(dot, "/.");
    if (_link(path, dot) != 0) {
        printf("mkdir: link \".\" error %d\n", *syserror);
        return (-1);
    }
    strcpy(dot, path);
    strcat(dot, "/..");
    if (_link(".", dot) != 0) {
        printf("mkdir: link \"..\" error %d\n", *syserror);
        return (-1);
    }
    return (0);
}



int get( char *arg, int binflag)
{
    FILE *fp;
    int d;
    char cbuf[512];
    int nread;

    fp = fopen(arg, binflag ? "rb" : "r");
    if (fp == NULL) {
        printf("Source file not found\n");
        return (-1);
    }
    d = _creat(basename(arg), 0666);
    if (d < 0) {
        printf("Cant open unix file error %d\n", *syserror);
        return (-1);
    }
    for (;;) {
        nread = fread(cbuf, 1, 512, fp);
        if (nread == 0)
            break;
        if (_write(d, cbuf, nread) != nread) {
            printf("_write error %d\n", *syserror);
            fclose(fp);
            _close(d);
            return (-1);
        }
    }
    fclose(fp);
    _close(d);
    _sync();
    return (0);
}


int put( char *arg, int binflag)
{
    FILE *fp;
    int d;
    char cbuf[512];
    int nread;

    fp = fopen(arg, binflag ? "wb" : "w");
    if (fp == NULL) {
        printf("Cant open destination file.\n");
        return (-1);
    }
    d = _open(arg, 0);
    if (d < 0) {
        printf("Cant open unix file error %d\n", *syserror);
        return (-1);
    }
    for (;;) {
        if ((nread = _read(d, cbuf, 512)) == 0)
            break;
        if (fwrite(cbuf, 1, nread, fp) != nread) {
            printf("fwrite error");
            fclose(fp);
            _close(d);
            return (-1);
        }
    }
    fclose(fp);
    _close(d);
    return (0);
}


int type( char *arg)
{
    int d, i;
    char cbuf[512];
    int nread;

    d = _open(arg, 0);
    if (d < 0) {
        printf("Cant open unix file error %d\n", *syserror);
        return (-1);
    }
    for (;;) {
        if ((nread = _read(d, cbuf, 512)) == 0)
            break;

        for (i = 0; i < nread; i++) {
            if (cbuf[i] == 0x1a)
                break;
            fputc(cbuf[i], stdout);
        }
    }
    fputc('\n', stdout);
    _close(d);
    return (0);
}


int fdump(char *arg)
{
    int d;
    char cbuf[512];
    int nread;

    printf("Dump starting.\n");
    d = _open(arg, 0);
    if (d < 0) {
        printf("Cant open unix file error %d\n", *syserror);
        return (-1);
    }
    for (;;) {
        if ((nread = _read(d, cbuf, 512)) == 0)
            break;
    }
    _close(d);
    printf("Dump done.\n");
    return (0);
}


int unlink( char *path)
{
    struct uzi_stat statbuf;

    if (_stat(path, &statbuf) != 0) {
        printf("unlink: can't stat %s\n", path);
        return (-1);
    }
    if ((statbuf.st_mode & F_MASK) == F_DIR) {
        printf("unlink: %s is a directory\n", path);
        return (-1);
    }
    if (_unlink(path) != 0) {
        printf("unlink: _unlink errn=or %d\n", *syserror);
        return (-1);
    }
    return (0);
}


int rmdir(char *path)
{
    struct uzi_stat statbuf;
    char newpath[100];
    struct direct dir;
    int fd;

    if (_stat(path, &statbuf) != 0) {
        printf("rmdir: can't stat %s\n", path);
        return (-1);
    }
    if ((statbuf.st_mode & F_DIR) == 0) {
        /*-- Constant expression !!!  HFB --*/
        printf("rmdir: %s is not a directory\n", path);
        return (-1);
    }
    if ((fd = _open(path, 0)) < 0) {
        printf("rmdir: %s is unreadable\n", path);
        return (-1);
    }
    while (_read(fd, (char *) &dir, sizeof(dir)) == sizeof(dir)) {
        if (dir.d_ino == 0)
            continue;
        if (!strcmp(dir.d_name, ".") || !strcmp(dir.d_name, ".."))
            continue;
        printf("rmdir: %s is not empty\n", path);
        _close(fd);
        return (-1);
    }
    _close(fd);

    strcpy(newpath, path);
    strcat(newpath, "/.");
    if (_unlink(newpath) != 0) {
        printf("rmdir: can't unlink \".\"  error %d\n", *syserror);
        /*return (-1); */
    }
    strcat(newpath, ".");
    if (_unlink(newpath) != 0) {
        printf("rmdir: can't unlink \"..\"  error %d\n", *syserror);
        /*return (-1); */
    }
    if (_unlink(path) != 0) {
        printf("rmdir: _unlink error %d\n", *syserror);
        return (-1);
    }
    return (0);
}
