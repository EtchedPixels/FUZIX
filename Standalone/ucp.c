/**************************************************
  UZI (Unix Z80 Implementation) Utilities:  ucp.c
Modifications:
14 June 1998 - Reformatted, restructured command
switch, sense Ctrl-Z in type.   HFB
21 Sept 1999 - Corrected the 'constant expression'
problem, added some missing breaks.
HP
 ***************************************************/

#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h>
#define UCP
#include "fuzix_fs.h"
#include "util.h"

#define UCP_VERSION  "1.4ac"

static int16_t *syserror = (int16_t *) & udata.u_error;
static char cwd[100];
static char line[128];
static char *nextline = NULL;
static char *month[] = { "Jan", "Feb", "Mar", "Apr",
	"May", "Jun", "Jul", "Aug",
	"Sep", "Oct", "Nov", "Dec"
};

static uint16_t bufclock = 0;	/* Time-stamp counter for LRU */
static struct blkbuf bufpool[NBUFS];

static struct cinode i_tab[ITABSIZE];
static struct filesys fs_tab[1];

static inoptr root;
static struct oft of_tab[OFTSIZE];


static int match(char *cmd);
static void usage(void);
static void prmode(int mode);
static int cmd_ls(char *path);
static int cmd_chmod(char *modes, char *path);
static int cmd_mknod(char *path, char *modes, char *devs);
static int cmd_mkdir(char *path);
static int cmd_link(char *src, char *dst);
static int cmd_get(char *src, char *dest, int binflag);
static int cmd_put(char *arg, int binflag);
static int cmd_type(char *arg);
static int cmd_fdump(char *arg);
static int cmd_rm(char *path);
static int cmd_rmdir(char *path);

static int interactive = 0;

static void cmdusage(void)
{
	fprintf(stderr, "Usage: ucp [-b] FILE [COMMAND]\n");
	exit(1);
}

int main(int argc, char *argval[])
{
	int rdev;
	char cmd[30], arg1[1024], arg2[30], arg3[30];
	int count;
	int multiline;
	int pending_line = 0;
	struct filesys fsys;
	int j, retc = 0;
	int opt;

	while ((opt = getopt(argc, argval, "b")) != -1) {
		switch (opt) {
		case 'b':
			swapped = 1;
			break;
		default:
			cmdusage();
		}
	}
	if (optind >= argc)
		cmdusage();
	if (argc - optind == 1) {
		fd_open(argval[optind], 0);
		multiline = 1;
	} else if (argc - optind == 2) {
		fd_open(argval[optind], 0);
		strncpy(&line[0], argval[optind + 1], 127);
		line[127] = '\0';
		multiline = 0;
	} else {
		cmdusage();
	}

	if (isatty(0))
		interactive = 1;

	rdev = 0;

	xfs_init(rdev);
	strcpy(cwd, "/");

	printf("Fuzix UCP version " UCP_VERSION ".%s\n",
	       interactive ? " Type ? for help." : "");

	do {
		if (multiline && !pending_line) {
			if (interactive)
				printf("ucp: ");
			if (fgets(line, 128, stdin) == NULL) {
				xfs_end();
				exit(retc);
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
			count =
			    sscanf(nextline, "%s %s %s %s", cmd, arg1,
				   arg2, arg3);
			nextline = NULL;
			pending_line = 0;
			if (count == 0 || cmd[0] == '\0')
				continue;
		} else {
			count =
			    sscanf(line, "%s %s %s %s", cmd, arg1, arg2,
				   arg3);
			if (nextline != NULL) {
				pending_line = 1;
			}
			if (count == 0 || cmd[0] == '\0')
				continue;
		}

		fuzix_sync();

		if (strcmp(cmd, "\n") == 0)
			continue;
		if (cmd[0] == '#')
			continue;
		switch (match(cmd)) {
		case 0:	/* exit */
			xfs_end();
			exit(retc);

		case 1:	/* ls */
			if (*arg1)
				retc = cmd_ls(arg1);
			else
				retc = cmd_ls(".");
			break;

		case 2:	/* cd */
			if (*arg1) {
				strcpy(cwd, arg1);
				if ((retc = fuzix_chdir(arg1)) != 0) {
					printf("cd: error number %d\n",
					       *syserror);
				}
			}
			break;

		case 3:	/* mkdir */
			if (*arg1)
				retc = cmd_mkdir(arg1);
			break;

		case 4:	/* mknod */
			if (*arg1 && *arg2 && *arg3)
				retc = cmd_mknod(arg1, arg2, arg3);
			break;

		case 5:	/* chmod */
			if (*arg1 && *arg2)
				retc = cmd_chmod(arg1, arg2);
			break;

		case 6:	/* get */
			if (*arg1)
				retc =
				    cmd_get(arg1, *arg2 ? arg2 : arg1, 0);
			break;

		case 7:	/* bget */
			if (*arg1)
				retc =
				    cmd_get(arg1, *arg2 ? arg2 : arg1, 1);
			break;

		case 8:	/* put */
			if (*arg1)
				retc = cmd_put(arg1, 0);
			break;

		case 9:	/* bput */
			if (*arg1)
				retc = cmd_put(arg1, 1);
			break;

		case 10:	/* type */
			if (*arg1)
				retc = cmd_type(arg1);
			break;

		case 11:	/* dump */
			if (*arg1)
				retc = cmd_fdump(arg1);
			break;

		case 12:	/* rm */
			if (*arg1)
				retc = cmd_rm(arg1);
			break;

		case 13:	/* df */
			for (j = 0; j < 4; ++j) {
				retc = fuzix_getfsys(j, (char *) &fsys);
				if (retc == 0 && fsys.s_mounted) {
					printf
					    ("%d:  %u blks used, %u free;  ",
					     j,
					     (fsys.s_fsize -
					      fsys.s_isize) - fsys.s_tfree,
					     fsys.s_tfree);
					printf("%u inodes used, %u free\n",
					       (8 * (fsys.s_isize - 2) -
						fsys.s_tinode),
					       fsys.s_tinode);
				}
			}
			break;

		case 14:	/* rmdir */
			if (*arg1)
				retc = cmd_rmdir(arg1);
			break;

		case 17:	/* ln */
			if (*arg1 && *arg2)
				retc = cmd_link(arg1, arg2);
			break;

		case 50:	/* help */
			usage();
			retc = 0;
			break;

		default:	/* ..else.. */
			printf("Unknown command, type ? for help.\n");
			retc = -1;
			break;
		}		/* End Switch */
	} while (multiline || pending_line);

	fuzix_sync();

	return retc;
}


static int match(char *cmd)
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
	else if (strcmp(cmd, "ln") == 0)
		return (17);
	else if (strcmp(cmd, "help") == 0)
		return (50);
	else if (strcmp(cmd, "?") == 0)
		return (50);
	else
		return (-1);
}


static void usage(void)
{
	printf("UCP commands:\n");
	printf("?|help\n");
	printf("exit|quit\n");
	printf("dir|ls [path]\n");
	printf("cd path\n");
	printf("mkdir dirname\n");
	printf("mknod name mode dev#\n");
	printf("chmod mode path\n");
	printf("[b]get sourcefile [destfile]\n");
	printf("[b]put uzifile\n");
	printf("type|cat filename\n");
	printf("dump filename\n");
	printf("rm path\n");
	printf("rmdir dirname\n");
	printf("df\n");
	printf("ln sourcefile destfile\n");
}

static void prmode(int mode)
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

static void fuzix_ls_out(char *dname)
{
	char *p;
	int st;

	struct uzi_stat statbuf;
	if (fuzix_stat(dname, &statbuf) != 0) {
		fprintf(stderr, "ls: can't stat %s\n", dname);
		return;
	}

	st = (statbuf.st_mode & F_MASK);
	if ((st & F_MASK) == F_DIR)	/* & F_MASK is redundant */
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
	printf("%12u ", (statbuf.st_mode & F_CDEV) ?
	       statbuf.st_rdev : statbuf.st_size);

	if (statbuf.fst_mtime == 0) {	/* st_mtime? */
		printf("                   ");
	} else {
		time_t t = statbuf.fst_mtime;
		struct tm *tm = gmtime(&t);
		printf("%s %02d %4d   ",
		       month[tm->tm_mon], tm->tm_mday, tm->tm_year);
		printf("%2d:%02d", tm->tm_hour, tm->tm_min);
	}
	p = strrchr(dname, '/');
	if (p)
		dname = p + 1;
	if ((statbuf.st_mode & F_MASK) == F_DIR)
		strcat(dname, "/");
	printf("  %-15s\n", dname);
}

static int cmd_ls(char *path)
{
	struct direct buf;
	struct uzi_stat statbuf;
	char dname[512];
	int d, st;

	d = fuzix_open(path, 0);
	if (d < 0) {
		fprintf(stderr, "ls: can't open %s\n", path);
		return -1;
	}
	if (fuzix_stat(path, &statbuf) != 0) {
		fprintf(stderr, "ls: can't stat %s\n", path);
		return -1;
	}
	st = (statbuf.st_mode & F_MASK);
	if ((st & F_MASK) == F_DIR) {
		while (fuzix_read(d, (char *) &buf, 16) == 16) {
			if (buf.d_name[0] == '\0')
				continue;

			if (path[0] != '.' || path[1]) {
				strcpy(dname, path);
				strcat(dname, "/");
			} else {
				dname[0] = '\0';
			}
			strcat(dname, buf.d_name);
			fuzix_ls_out(dname);
		}
	} else
		fuzix_ls_out(path);
	fuzix_close(d);
	return 0;
}

static int cmd_chmod(char *modes, char *path)
{
	unsigned int mode;

	if (interactive)
		printf("chmod %s to %s\n", path, modes);
	if (sscanf(modes, "%o", &mode) != 1) {
		fprintf(stderr, "chmod: bad mode\n");
		return (-1);
	}
	/* Preserve the type if not specified */
	if (mode < 010000) {
		struct uzi_stat st;
		if (fuzix_stat(path, &st) != 0) {
			fprintf(stderr,
				"chmod: can't stat file '%s': %d\n", path,
				*syserror);
			return -1;
		}
		mode = (st.st_mode & ~0x7777) | mode;
	}
	if (fuzix_chmod(path, mode)) {
		fprintf(stderr, "chmod: error %d\n", *syserror);
		return (-1);
	}
	return 0;
}


static int cmd_mknod(char *path, char *modes, char *devs)
{
	unsigned int mode;
	int dev;

	if (sscanf(modes, "%o", &mode) != 1) {
		fprintf(stderr, "mknod: bad mode\n");
		return (-1);
	}
	if ((mode & F_MASK) != F_BDEV && (mode & F_MASK) != F_CDEV) {
		fprintf(stderr, "mknod: mode is not device\n");
		return (-1);
	}
	dev = -1;
	sscanf(devs, "%d", &dev);
	if (dev == -1) {
		fprintf(stderr, "mknod: bad device\n");
		return (-1);
	}
	if (fuzix_mknod(path, mode, dev) != 0) {
		fprintf(stderr, "fuzix_mknod: error %d\n", *syserror);
		return (-1);
	}
	return (0);
}



static int cmd_mkdir(char *path)
{
	if (fuzix_mkdir(path, 0777) != 0) {
		fprintf(stderr, "mkdir: mknod error %d\n", *syserror);
		return (-1);
	}
	return (0);
}

static int cmd_link(char *src, char *dst)
{
	if (fuzix_link(src, dst) != 0) {
		fprintf(stderr, "link: error %d\n", *syserror);
		return -1;
	}
	return 0;
}

static int cmd_get(char *src, char *dest, int binflag)
{
	FILE *fp;
	int d;
	char cbuf[512];
	int nread;

	fp = fopen(src, binflag ? "rb" : "r");
	if (fp == NULL) {
		fprintf(stderr, "Source file '%s' not found\n", src);
		return (-1);
	}
	d = fuzix_creat(basename(dest), 0666);
	if (d < 0) {
		fprintf(stderr,
			"Can't open destination file '%s'; error %d\n",
			dest, *syserror);
		fclose(fp);
		return (-1);
	}
	for (;;) {
		nread = fread(cbuf, 1, 512, fp);
		if (nread == 0)
			break;
		if (fuzix_write(d, cbuf, nread)) {
			fprintf(stderr, "fuzix_write error %d\n",
				*syserror);
			fclose(fp);
			fuzix_close(d);
			return (-1);
		}
	}
	fclose(fp);
	fuzix_close(d);
	fuzix_sync();
	return (0);
}


static int cmd_put(char *arg, int binflag)
{
	FILE *fp;
	int d;
	char cbuf[512];
	int nread;

	fp = fopen(arg, binflag ? "wb" : "w");
	if (fp == NULL) {
		fprintf(stderr, "Can't open destination file.\n");
		return (-1);
	}
	d = fuzix_open(arg, 0);
	if (d < 0) {
		fprintf(stderr, "Can't open unix file error %d\n",
			*syserror);
		return (-1);
	}
	for (;;) {
		if ((nread = fuzix_read(d, cbuf, 512)) == 0)
			break;
		if (fwrite(cbuf, 1, nread, fp) != nread) {
			fprintf(stderr, "fwrite error");
			fclose(fp);
			fuzix_close(d);
			return (-1);
		}
	}
	fclose(fp);
	fuzix_close(d);
	return (0);
}


static int cmd_type(char *arg)
{
	int d, i;
	char cbuf[512];
	int nread;

	d = fuzix_open(arg, 0);
	if (d < 0) {
		fprintf(stderr, "Can't open unix file error %d\n",
			*syserror);
		return (-1);
	}
	for (;;) {
		if ((nread = fuzix_read(d, cbuf, 512)) == 0)
			break;

		for (i = 0; i < nread; i++) {
			if (cbuf[i] == 0x1a)
				break;
			fputc(cbuf[i], stdout);
		}
	}
	fputc('\n', stdout);
	fuzix_close(d);
	return (0);
}


static int cmd_fdump(char *arg)
{
	int d;
	char cbuf[512];
	int nread;

	printf("Dump starting.\n");
	d = fuzix_open(arg, 0);
	if (d < 0) {
		fprintf(stderr, "Can't open unix file error %d\n",
			*syserror);
		return (-1);
	}
	for (;;) {
		if ((nread = fuzix_read(d, cbuf, 512)) == 0)
			break;
	}
	fuzix_close(d);
	printf("Dump done.\n");
	return (0);
}


static int cmd_rm(char *path)
{
	struct uzi_stat statbuf;

	if (fuzix_stat(path, &statbuf) != 0) {
		fprintf(stderr, "unlink: can't stat %s\n", path);
		return (-1);
	}
	if ((statbuf.st_mode & F_MASK) == F_DIR) {
		fprintf(stderr, "unlink: %s is a directory\n", path);
		return (-1);
	}
	if (fuzix_unlink(path) != 0) {
		fprintf(stderr, "unlink: fuzix_unlink errn=or %d\n",
			*syserror);
		return (-1);
	}
	return (0);
}


static int cmd_rmdir(char *path)
{
	struct uzi_stat statbuf;
	char newpath[100];
	struct direct dir;
	int fd;

	if (fuzix_stat(path, &statbuf) != 0) {
		fprintf(stderr, "rmdir: can't stat %s\n", path);
		return (-1);
	}
	if ((statbuf.st_mode & F_DIR) == 0) {
	/*-- Constant expression !!!  HFB --*/
		fprintf(stderr, "rmdir: %s is not a directory\n", path);
		return (-1);
	}
	if ((fd = fuzix_open(path, 0)) < 0) {
		fprintf(stderr, "rmdir: %s is unreadable\n", path);
		return (-1);
	}
	while (fuzix_read(fd, (char *) &dir, sizeof(dir)) == sizeof(dir)) {
		if (dir.d_ino == 0)
			continue;
		if (!strcmp(dir.d_name, ".") || !strcmp(dir.d_name, ".."))
			continue;
		fprintf(stderr, "rmdir: %s is not empty\n", path);
		fuzix_close(fd);
		return (-1);
	}
	fuzix_close(fd);

	strcpy(newpath, path);
	strcat(newpath, "/.");
	if (fuzix_unlink(newpath) != 0) {
		fprintf(stderr, "rmdir: can't unlink \".\"  error %d\n",
			*syserror);
		/*return (-1); */
	}
	strcat(newpath, ".");
	if (fuzix_unlink(newpath) != 0) {
		fprintf(stderr, "rmdir: can't unlink \"..\"  error %d\n",
			*syserror);
		/*return (-1); */
	}
	if (fuzix_unlink(path) != 0) {
		fprintf(stderr, "rmdir: fuzix_unlink error %d\n",
			*syserror);
		return (-1);
	}
	return (0);
}

static void fs_init(void)
{
	udata.u_euid = 0;
	udata.u_insys = 1;
}

static void xfs_init(int bootdev)
{
	register char *j;

	fs_init();
	bufinit();

	/* User's file table */
	for (j = udata.u_files; j < (udata.u_files + UFTSIZE); ++j)
		*j = -1;

	/* Mount the root device */
	if (fmount(ROOTDEV, NULLINODE))
		panic("no filesys");

	ifnot(root = i_open(ROOTDEV, ROOTINODE))
	    panic("no root");

	i_ref(udata.u_cwd = root);
}


static void xfs_end(void)
{
	register int16_t j;

	for (j = 0; j < UFTSIZE; ++j) {
		ifnot(udata.u_files[j] & 0x80)	/* Portable equivalent of == -1 */
		    doclose(j);
	}
}


static int fuzix_open(char *name, int16_t flag)
{
	int16_t uindex;
	register int16_t oftindex;
	register inoptr ino;

	udata.u_error = 0;

	if (flag < 0 || flag > 2) {
		udata.u_error = EINVAL;
		return (-1);
	}
	if ((uindex = uf_alloc()) == -1)
		return (-1);

	if ((oftindex = oft_alloc()) == -1)
		goto nooft;

	ifnot(ino = n_open(name, NULLINOPTR))
	    goto cantopen;

	of_tab[oftindex].o_inode = ino;

	if (fuzix_getmode(ino) == F_DIR &&
	    (flag == FO_WRONLY || flag == FO_RDWR)) {
		udata.u_error = EISDIR;
		goto cantopen;
	}

	if (isdevice(ino)) {	/* && d_open((int)ino->c_node.i_addr[0]) != 0) */
		udata.u_error = ENXIO;
		goto cantopen;
	}

	udata.u_files[uindex] = oftindex;

	of_tab[oftindex].o_ptr = 0;
	of_tab[oftindex].o_access = flag;

	return (uindex);

      cantopen:
	oft_deref(oftindex);	/* This will call i_deref() */
      nooft:
	udata.u_files[uindex] = -1;
	return (-1);
}

static int doclose(int16_t uindex)
{
	register int16_t oftindex;
	inoptr ino;

	udata.u_error = 0;
	ifnot(ino = getinode(uindex))
	    return (-1);
	oftindex = udata.u_files[uindex];

#if 0
	if (isdevice(ino)
	    /* && ino->c_refs == 1 && of_tab[oftindex].o_refs == 1 */
	    )
		d_close((int) (ino->c_node.i_addr[0]));
#endif

	udata.u_files[uindex] = -1;
	oft_deref(oftindex);

	return (0);
}

static int fuzix_close(int16_t uindex)
{
	udata.u_error = 0;
	return (doclose(uindex));
}

static int fuzix_creat(char *name, int16_t mode)
{
	register inoptr ino;
	register int16_t uindex;
	register int16_t oftindex;
	inoptr parent;
	register int16_t j;

	udata.u_error = 0;
	parent = NULLINODE;

	if ((uindex = uf_alloc()) == -1)
		return (-1);
	if ((oftindex = oft_alloc()) == -1)
		return (-1);

	ino = n_open(name, &parent);
	if (ino) {
		i_deref(parent);
		if (fuzix_getmode(ino) == F_DIR) {
			i_deref(ino);
			udata.u_error = EISDIR;
			goto nogood;
		}
		if (fuzix_getmode(ino) == F_REG) {
			/* Truncate the file to zero length */
			f_trunc(ino);
			/* Reset any oft pointers */
			for (j = 0; j < OFTSIZE; ++j)
				if (of_tab[j].o_inode == ino)
					of_tab[j].o_ptr = 0;
		}
	} else {
		if (parent && (ino = newfile(parent, name)))
			/* Parent was derefed in newfile */
		{
			ino->c_node.i_mode =
			    swizzle16(F_REG |
				      (mode & MODE_MASK & ~udata.u_mask));
			setftime(ino, A_TIME | M_TIME | C_TIME);
			/* The rest of the inode is initialized in newfile() */
			wr_inode(ino);
		} else {
			/* Doesn't exist and can't make it */
			goto nogood;
		}
	}
	udata.u_files[uindex] = oftindex;

	of_tab[oftindex].o_ptr = 0;
	of_tab[oftindex].o_inode = ino;
	of_tab[oftindex].o_access = FO_WRONLY;

	return (uindex);

      nogood:
	oft_deref(oftindex);
	return (-1);
}

static int fuzix_link(char *name1, char *name2)
{
	register inoptr ino;
	register inoptr ino2;
	inoptr parent2;

	udata.u_error = 0;
	ifnot(ino = n_open(name1, NULLINOPTR))
	    return (-1);

	/* Make sure file2 doesn't exist, and get its parent */
	if ((ino2 = n_open(name2, &parent2))) {
		i_deref(ino2);
		i_deref(parent2);
		udata.u_error = EEXIST;
		goto nogood;
	}

	ifnot(parent2)
	    goto nogood;

	if (ino->c_dev != parent2->c_dev) {
		i_deref(parent2);
		udata.u_error = EXDEV;
		goto nogood;
	}

	if (ch_link(parent2, "", filename(name2), ino) == 0)
		goto nogood;

	/* Update the link count. */
	ino->c_node.i_nlink =
	    swizzle16(swizzle16(ino->c_node.i_nlink) + 1);
	wr_inode(ino);
	setftime(ino, C_TIME);

	i_deref(parent2);
	i_deref(ino);
	return (0);

      nogood:
	i_deref(ino);
	return (-1);
}

static int fuzix_unlink(char *path)
{
	register inoptr ino;
	inoptr pino;

	udata.u_error = 0;
	ino = n_open(path, &pino);

	ifnot(pino && ino) {
		udata.u_error = ENOENT;
		return (-1);
	}

	/* Remove the directory entry */

	if (ch_link(pino, filename(path), "", NULLINODE) == 0)
		goto nogood;

	/* Decrease the link count of the inode */

	if (ino->c_node.i_nlink == 0) {
		ino->c_node.i_nlink =
		    swizzle16(swizzle16(ino->c_node.i_nlink) + 2);
		printf("fuzix_unlink: bad nlink\n");
	} else
		ino->c_node.i_nlink =
		    swizzle16(swizzle16(ino->c_node.i_nlink) - 1);
	setftime(ino, C_TIME);
	i_deref(pino);
	i_deref(ino);
	return (0);
      nogood:
	i_deref(pino);
	i_deref(ino);
	return (-1);
}

static uint16_t fuzix_read(int16_t d, char *buf, uint16_t nbytes)
{
	register inoptr ino;
	uint16_t r;

	udata.u_error = 0;
	/* Set up u_base, u_offset, ino; check permissions, file num. */
	if ((ino = rwsetup(1, d, buf, nbytes)) == NULLINODE)
		return (-1);	/* bomb out if error */

	r = readi(ino);
	updoff(d);
	return r;
}

static uint16_t fuzix_write(int16_t d, char *buf, uint16_t nbytes)
{
	register inoptr ino;
	uint16_t r;

	udata.u_error = 0;
	/* Set up u_base, u_offset, ino; check permissions, file num. */
	if ((ino = rwsetup(0, d, buf, nbytes)) == NULLINODE)
		return (-1);	/* bomb out if error */

	r = writei(ino);
	updoff(d);

	return r;
}

static inoptr rwsetup(int rwflag, int d, char *buf, int nbytes)
{
	register inoptr ino;
	register struct oft *oftp;

	udata.u_base = buf;
	udata.u_count = nbytes;

	if ((ino = getinode(d)) == NULLINODE)
		return (NULLINODE);

	oftp = of_tab + udata.u_files[d];
	if (oftp->o_access == (rwflag ? FO_WRONLY : FO_RDONLY)) {
		udata.u_error = EBADF;
		return (NULLINODE);
	}

	setftime(ino, rwflag ? A_TIME : (A_TIME | M_TIME | C_TIME));

	/* Initialize u_offset from file pointer */
	udata.u_offset = oftp->o_ptr;

	return (ino);
}

static uint16_t readi(inoptr ino)
{
	register uint16_t amount;
	register uint16_t toread;
	register blkno_t pblk;
	register uint8_t *bp;

	switch (fuzix_getmode(ino)) {

	case F_DIR:
	case F_REG:

		/* See of end of file will limit read */
		toread = udata.u_count =
		    min(udata.u_count,
			swizzle32(ino->c_node.i_size) - udata.u_offset);
		while (toread) {
			if ((pblk =
			     bmap(ino, udata.u_offset >> 9, 1)) != NULLBLK)
				bp = bread(0, pblk, 0);
			else
				bp = zerobuf();

			memcpy(udata.u_base, bp + (udata.u_offset & 511),
			       (amount =
				min(toread,
				    512 - (udata.u_offset & 511))));
			brelse((bufptr) bp);

			udata.u_base += amount;
			udata.u_offset += amount;
			toread -= amount;
		}
		return udata.u_count - toread;

	default:
		udata.u_error = ENODEV;
	}
	return 0;
}



/* Writei (and readi) need more i/o error handling */

static uint16_t writei(inoptr ino)
{
	register uint16_t amount;
	register uint16_t towrite;
	register uint8_t *bp;
	blkno_t pblk;

	switch (fuzix_getmode(ino)) {

	case F_DIR:
	case F_REG:
		towrite = udata.u_count;
		while (towrite) {
			amount =
			    min(towrite, 512 - (udata.u_offset & 511));

			if ((pblk =
			     bmap(ino, udata.u_offset >> 9, 0)) == NULLBLK)
				break;	/* No space to make more blocks */

			/* If we are writing an entire block, we don't care
			   about its previous contents */
			bp = bread(0, pblk, (amount == 512));

			memcpy(bp + (udata.u_offset & 511), udata.u_base,
			       amount);
			bawrite((bufptr) bp);

			udata.u_base += amount;
			udata.u_offset += amount;
			towrite -= amount;
		}

		/* Update size if file grew */
		if (udata.u_offset > swizzle32(ino->c_node.i_size)) {
			ino->c_node.i_size = swizzle32(udata.u_offset);
			ino->c_dirty = 1;
		}
		return towrite;

	default:
		udata.u_error = ENODEV;
	}
	return udata.u_count;
}



static void updoff(int d)
{
	/* Update current file pointer */
	of_tab[udata.u_files[d]].o_ptr = udata.u_offset;
}

static int valadr(char *base, uint16_t size)
{
	return (1);
}


static int fuzix_mknod(char *name, int16_t mode, int16_t dev)
{
	register inoptr ino;
	inoptr parent;

	udata.u_error = 0;

	if ((ino = n_open(name, &parent))) {
		udata.u_error = EEXIST;
		goto nogood;
	}

	ifnot(parent) {
		udata.u_error = ENOENT;
		return (-1);
	}

	ifnot(ino = newfile(parent, name))
	    goto nogood2;

	/* Initialize mode and dev */
	ino->c_node.i_mode = swizzle16(mode & ~udata.u_mask);
	ino->c_node.i_addr[0] = swizzle16(isdevice(ino) ? dev : 0);
	setftime(ino, A_TIME | M_TIME | C_TIME);
	wr_inode(ino);

	i_deref(ino);
	return (0);

      nogood:
	i_deref(ino);
      nogood2:
	i_deref(parent);
	return (-1);
}

static void fuzix_sync(void)
{
	int j;
	inoptr ino;
	uint8_t *buf;

	/* Write out modified inodes */

	udata.u_error = 0;
	for (ino = i_tab; ino < i_tab + ITABSIZE; ++ino)
		if ((ino->c_refs) > 0 && ino->c_dirty != 0) {
			wr_inode(ino);
			ino->c_dirty = 0;
		}

	/* Write out modified super blocks */
	/* This fills the rest of the super block with garbage. */

	for (j = 0; j < NDEVS; ++j) {
		if (swizzle16(fs_tab[j].s_mounted) == SMOUNTED
		    && fs_tab[j].s_fmod) {
			fs_tab[j].s_fmod = 0;
			buf = bread(j, 1, 1);
			memcpy(buf, (char *) &fs_tab[j],
			       sizeof(struct filesys));
			bfree((bufptr) buf, 2);
		}
	}
	bufsync();		/* Clear buffer pool */
}

static int fuzix_chdir(char *dir)
{
	register inoptr newcwd;

	udata.u_error = 0;
	ifnot(newcwd = n_open(dir, NULLINOPTR))
	    return (-1);

	if (fuzix_getmode(newcwd) != F_DIR) {
		udata.u_error = ENOTDIR;
		i_deref(newcwd);
		return (-1);
	}
	i_deref(udata.u_cwd);
	udata.u_cwd = newcwd;
	return (0);
}

static int min(int a, int b)
{
	return (a < b ? a : b);
}

static int fuzix_chmod(char *path, int16_t mode)
{
	inoptr ino;

	udata.u_error = 0;
	ifnot(ino = n_open(path, NULLINOPTR))
	    return (-1);

	ino->c_node.i_mode =
	    swizzle16((mode & MODE_MASK) |
		      (swizzle16(ino->c_node.i_mode) & F_MASK));
	setftime(ino, C_TIME);
	i_deref(ino);
	return (0);
}

static int fuzix_stat(char *path, struct uzi_stat *buf)
{
	register inoptr ino;

	udata.u_error = 0;
	ifnot(valadr((char *) buf, sizeof(struct uzi_stat))
	      && (ino = n_open(path, NULLINOPTR))) {
		return (-1);
	}
	stcpy(ino, buf);
	i_deref(ino);
	return (0);
}

/* Utility for stat and fstat */
static void stcpy(inoptr ino, struct uzi_stat *buf)
{
	struct uzi_stat *b = (struct uzi_stat *) buf;

	b->st_dev = swizzle16(ino->c_dev);
	b->st_ino = swizzle16(ino->c_num);
	b->st_mode = swizzle16(ino->c_node.i_mode);
	b->st_nlink = swizzle16(ino->c_node.i_nlink);
	b->st_uid = swizzle16(ino->c_node.i_uid);
	b->st_gid = swizzle16(ino->c_node.i_gid);

	b->st_rdev = swizzle16(ino->c_node.i_addr[0]);

	b->st_size = swizzle32(ino->c_node.i_size);
	b->fst_atime = swizzle32(ino->c_node.i_atime);
	b->fst_mtime = swizzle32(ino->c_node.i_mtime);
	b->fst_ctime = swizzle32(ino->c_node.i_ctime);
}

/* Special system call returns super-block of given filesystem for
 * users to determine free space, etc.  Should be replaced with a
 * sync() followed by a read of block 1 of the device.
 */

static int fuzix_getfsys(int dev, char *buf)
{
	udata.u_error = 0;
	if (dev < 0 || dev >= NDEVS
	    || swizzle16(fs_tab[dev].s_mounted) != SMOUNTED) {
		udata.u_error = ENXIO;
		return (-1);
	}

	/* FIXME: endian swapping here */
	memcpy(buf, &fs_tab[dev], sizeof(struct filesys));
	return (0);
}

static int fuzix_mkdir(char *name, int mode)
{
	inoptr ino;
	inoptr parent;
	char fname[FILENAME_LEN + 1];

	if ((ino = n_open(name, &parent)) != NULL) {
		udata.u_error = EEXIST;
		goto nogood;
	}

	if (!parent) {
		udata.u_error = ENOENT;
		return (-1);
	}

	if (swizzle16(parent->c_node.i_nlink) == 0xFFFF) {
		udata.u_error = EMLINK;
		goto nogood2;
	}

	filename_2(name, fname);

	i_ref(parent);		/* We need it again in a minute */
	if (!(ino = newfile(parent, fname))) {
		i_deref(parent);
		goto nogood2;	/* parent inode is derefed in newfile. */
	}

	/* Initialize mode and dev */
	ino->c_node.i_mode = swizzle16(F_DIR | 0200);	/* so ch_link is allowed */
	setftime(ino, A_TIME | M_TIME | C_TIME);
	if (ch_link(ino, "", ".", ino) == 0 ||
	    ch_link(ino, "", "..", parent) == 0)
		goto cleanup;

	/* Link counts and permissions */
	ino->c_node.i_nlink = swizzle16(2);
	parent->c_node.i_nlink =
	    swizzle16(swizzle16(parent->c_node.i_nlink) + 1);
	ino->c_node.i_mode =
	    swizzle16(((mode & ~udata.u_mask) & MODE_MASK) | F_DIR);
	i_deref(parent);
	wr_inode(ino);
	i_deref(ino);
	return (0);

      cleanup:
	if (!ch_link(parent, fname, "", NULLINODE))
		fprintf(stderr, "mkdir: bad rec\n");
	/* i_deref will put the blocks */
	ino->c_node.i_nlink = 0;
	wr_inode(ino);
      nogood:
	i_deref(ino);
      nogood2:
	i_deref(parent);
	return (-1);

}

static inoptr n_open(register char *name, register inoptr * parent)
{
	register inoptr wd;	/* the directory we are currently searching. */
	register inoptr ninode;

	if (*name == '/')
		wd = root;
	else
		wd = udata.u_cwd;

	i_ref(ninode = wd);
	i_ref(ninode);

	for (;;) {
		if (ninode)
			magic(ninode);

		/* See if we are at a mount point */
		if (ninode)
			ninode = srch_mt(ninode);

		while (*name == '/')	/* Skip (possibly repeated) slashes */
			++name;
		ifnot(*name)	/* No more components of path? */
		    break;
		ifnot(ninode) {
			udata.u_error = ENOENT;
			goto nodir;
		}
		i_deref(wd);
		wd = ninode;
		if (fuzix_getmode(wd) != F_DIR) {
			udata.u_error = ENOTDIR;
			goto nodir;
		}

		ninode = srch_dir(wd, name);

		while (*name != '/' && *name)
			++name;
	}

	if (parent)
		*parent = wd;
	else
		i_deref(wd);
	ifnot(parent || ninode)
	    udata.u_error = ENOENT;
	return (ninode);

      nodir:
	if (parent)
		*parent = NULLINODE;
	i_deref(wd);
	return (NULLINODE);

}



/* Srch_dir is given a inode pointer of an open directory and a string
 * containing a filename, and searches the directory for the file.  If
 * it exists, it opens it and returns the inode pointer, otherwise NULL.
 * This depends on the fact that ba_read will return unallocated blocks
 * as zero-filled, and a partially allocated block will be padded with
 * zeroes.
 */

static inoptr srch_dir(inoptr wd, register char *compname)
{
	register int curentry;
	register blkno_t curblock;
	register struct direct *buf;
	register int nblocks;
	unsigned inum;

	nblocks = (swizzle32(wd->c_node.i_size) + 511) >> 9;

	for (curblock = 0; curblock < nblocks; ++curblock) {
		buf =
		    (struct direct *) bread(wd->c_dev,
					    bmap(wd, curblock, 1), 0);
		for (curentry = 0; curentry < 16; ++curentry) {
			if (namecomp(compname, buf[curentry].d_name)) {
				inum =
				    swizzle16(buf[curentry & 0x0f].d_ino);
				brelse((bufptr) buf);
				return (i_open(wd->c_dev, inum));
			}
		}
		brelse((bufptr) buf);
	}
	return (NULLINODE);
}


/* Srch_mt sees if the given inode is a mount point. If so it
 * dereferences it, and references and returns a pointer to the
 * root of the mounted filesystem.
 */

static inoptr srch_mt(inoptr ino)
{
	register int j;

	for (j = 0; j < NDEVS; ++j)
		if (swizzle16(fs_tab[j].s_mounted) == SMOUNTED
		    && fs_tab[j].s_mntpt == ino) {
			i_deref(ino);
			return (i_open(j, ROOTINODE));
		}

	return (ino);
}


/* I_open is given an inode number and a device number,
 * and makes an entry in the inode table for them, or
 * increases it reference count if it is already there.
 * An inode # of zero means a newly allocated inode.
 */

static inoptr i_open(register int dev, register unsigned ino)
{

	struct dinode *buf;
	register inoptr nindex;
	int i;
	register inoptr j;
	int new;
	static inoptr nexti = i_tab;	/* added inoptr. 26.12.97  HFB */

	if (dev < 0 || dev >= NDEVS)
		panic("i_open: Bad dev");

	new = 0;
	ifnot(ino) {		/* Want a new one */
		new = 1;
		ifnot(ino = i_alloc(dev)) {
			udata.u_error = ENOSPC;
			return (NULLINODE);
		}
	}

	if (ino < ROOTINODE
	    || ino >= (swizzle16(fs_tab[dev].s_isize) - 2) * 8) {
		printf("i_open: bad inode number\n");
		return (NULLINODE);
	}


	nindex = NULLINODE;
	j = (inoptr) nexti;
	for (i = 0; i < ITABSIZE; ++i) {
		nexti = (inoptr) j;
		if (++j >= i_tab + ITABSIZE)
			j = i_tab;

		ifnot(j->c_refs)
		    nindex = j;

		if (j->c_dev == dev && j->c_num == ino) {
			nindex = j;
			goto found;
		}
	}

	/* Not already in table. */

	ifnot(nindex) {		/* No unrefed slots in inode table */
		udata.u_error = ENFILE;
		return (NULLINODE);
	}

	buf = (struct dinode *) bread(dev, (ino >> 3) + 2, 0);
	memcpy(&(nindex->c_node), &(buf[ino & 0x07]), 64);
	brelse((bufptr) buf);

	nindex->c_dev = dev;
	nindex->c_num = ino;
	nindex->c_magic = CMAGIC;

      found:
	if (new) {
		if (nindex->c_node.i_nlink
		    || swizzle16(nindex->c_node.i_mode) & F_MASK)
			goto badino;
	} else {
		ifnot(nindex->c_node.i_nlink
		      && swizzle16(nindex->c_node.i_mode) & F_MASK)
		    goto badino;
	}

	++nindex->c_refs;
	return (nindex);

      badino:
	printf("i_open: bad disk inode\n");
	return (NULLINODE);
}



/* Ch_link modifies or makes a new entry in the directory for the name
 * and inode pointer given. The directory is searched for oldname.  When
 * found, it is changed to newname, and it inode # is that of *nindex.
 * A oldname of "" matches a unused slot, and a nindex of NULLINODE
 * means an inode # of 0.  A return status of 0 means there was no
 * space left in the filesystem, or a non-empty oldname was not found,
 * or the user did not have write permission.
 */

static int ch_link(inoptr wd, char *oldname, char *newname, inoptr nindex)
{
	struct direct curentry;

	/* Search the directory for the desired slot. */

	udata.u_offset = 0;

	for (;;) {
		udata.u_count = 32;
		udata.u_base = (char *) &curentry;
		readi(wd);

		/* Read until EOF or name is found */
		/* readi() advances udata.u_offset */
		if (udata.u_count == 0
		    || namecomp(oldname, curentry.d_name))
			break;
	}

	if (udata.u_count == 0 && *oldname)
		return (0);	/* Entry not found */

	memcpy(curentry.d_name, newname, 30);

	{
		int i;

		for (i = 0; i < 30; ++i)
			if (curentry.d_name[i] == '\0')
				break;
		for (; i < 30; ++i)
			curentry.d_name[i] = '\0';
	}

	if (nindex)
		curentry.d_ino = swizzle16(nindex->c_num);
	else
		curentry.d_ino = 0;

	/* If an existing slot is being used, we must back up the file offset */
	if (udata.u_count)
		udata.u_offset -= 32;

	udata.u_count = 32;
	udata.u_base = (char *) &curentry;
	udata.u_sysio = 1;	/*280 */
	if (writei(wd))
		return 0;


	setftime(wd, A_TIME | M_TIME | C_TIME);	/* Sets c_dirty */

	/* Update file length to next block */
	if (swizzle32(wd->c_node.i_size) & 511)
		wd->c_node.i_size =
		    swizzle32(swizzle32(wd->c_node.i_size) + 512 -
			      (swizzle32(wd->c_node.i_size) & 511));

	return (1);
}



/* Filename is given a path name, and returns a pointer to the
 * final component of it.
 */

static char *filename(char *path)
{
	register char *ptr;

	ptr = path;
	while (*ptr)
		++ptr;
	while (*ptr != '/' && ptr-- > path);
	return (ptr + 1);
}

static void filename_2(char *path, char *name)
{
	register char *ptr;

	ptr = path;
	while (*ptr)
		++ptr;
	while (*ptr != '/' && ptr-- > path);
	memcpy(name, ptr + 1, FILENAME_LEN);
	name[FILENAME_LEN] = 0;
}


/* Namecomp compares two strings to see if they are the same file name.
 * It stops at 30 chars or a null or a slash. It returns 0 for difference.
 */

static int namecomp(char *n1, char *n2)
{
	register int n;

	n = 30;
	while (*n1 && *n1 != '/') {
		if (*n1++ != *n2++)
			return (0);
		ifnot(--n)
		    return (-1);
	}
	return (*n2 == '\0' || *n2 == '/');
}


/* Newfile is given a pointer to a directory and a name, and creates
 * an entry in the directory for the name, dereferences the parent,
 * and returns a pointer to the new inode.  It allocates an inode
 * number, and creates a new entry in the inode table for the new
 * file, and initializes the inode table entry for the new file.
 * The new file will have one reference, and 0 links to it.  Better
 * Better make sure there isn't already an entry with the same name.
 */

static inoptr newfile(inoptr pino, char *name)
{
	register inoptr nindex;
	register int j;

	ifnot(nindex = i_open(pino->c_dev, 0))
	    goto nogood;

	/* BIG FIX:  user/group setting was missing  SN *//*280 */
	nindex->c_node.i_uid = swizzle16(udata.u_euid);	/*280 */
	nindex->c_node.i_gid = swizzle16(udata.u_egid);	/*280 */

	nindex->c_node.i_mode = swizzle16(F_REG);	/* For the time being */
	nindex->c_node.i_nlink = swizzle16(1);
	nindex->c_node.i_size = 0;
	for (j = 0; j < 20; j++)
		nindex->c_node.i_addr[j] = 0;
	wr_inode(nindex);

	ifnot(ch_link(pino, "", filename(name), nindex)) {
		i_deref(nindex);
		goto nogood;
	}

	i_deref(pino);
	return (nindex);

      nogood:
	i_deref(pino);
	return (NULLINODE);
}


/* Check the given device number, and return its address in the mount
 * table.  Also time-stamp the superblock of dev, and mark it modified.
 * Used when freeing and allocating blocks and inodes.
 */

static fsptr getdev(int devno)
{
	register fsptr dev;

	dev = fs_tab + devno;
	if (devno < 0 || devno >= NDEVS || !dev->s_mounted)
		panic("getdev: bad dev");
	dev->s_fmod = 1;
	return (dev);
}


/* Returns true if the magic number of a superblock is corrupt.
 */

static int baddev(fsptr dev)
{
	return (swizzle16(dev->s_mounted) != SMOUNTED);
}


/* I_alloc finds an unused inode number, and returns it, or 0
 * if there are no more inodes available.
 */

static unsigned i_alloc(int devno)
{
	fsptr dev;
	blkno_t blk;
	struct dinode *buf;
	register int j;
	register int k;
	unsigned ino;

	if (baddev(dev = getdev(devno)))
		goto corrupt;

      tryagain:
	if (dev->s_ninode) {
		int i;

		ifnot(dev->s_tinode)
		    goto corrupt;
		i = swizzle16(dev->s_ninode);
		ino = swizzle16(dev->s_inode[--i]);
		dev->s_ninode = swizzle16(i);
		if (ino < 2 || ino >= (swizzle16(dev->s_isize) - 2) * 8)
			goto corrupt;
		dev->s_tinode = swizzle16(swizzle16(dev->s_tinode) - 1);
		return (ino);
	}

	/* We must scan the inodes, and fill up the table */

	fuzix_sync();		/* Make on-disk inodes consistent */
	k = 0;
	for (blk = 2; blk < swizzle16(dev->s_isize); blk++) {
		buf = (struct dinode *) bread(devno, blk, 0);
		for (j = 0; j < 8; j++) {
			ifnot(buf[j].i_mode || buf[j].i_nlink)
			    dev->s_inode[k++] =
			    swizzle16(8 * (blk - 2) + j);
			if (k == 50) {
				brelse((bufptr) buf);
				goto done;
			}
		}
		brelse((bufptr) buf);
	}

      done:
	ifnot(k) {
		if (dev->s_tinode)
			goto corrupt;
		udata.u_error = ENOSPC;
		return (0);
	}

	dev->s_ninode = swizzle16(k);
	goto tryagain;

      corrupt:
	printf("i_alloc: corrupt superblock\n");
	dev->s_mounted = swizzle16(1);
	udata.u_error = ENOSPC;
	return (0);
}


/* I_free is given a device and inode number, and frees the inode.
 * It is assumed that there are no references to the inode in the
 * inode table or in the filesystem.
 */

static void i_free(int devno, unsigned ino)
{
	register fsptr dev;

	if (baddev(dev = getdev(devno)))
		return;

	if (ino < 2 || ino >= (swizzle16(dev->s_isize) - 2) * 8)
		panic("i_free: bad ino");

	dev->s_tinode = swizzle16(swizzle16(dev->s_tinode) + 1);
	if (swizzle16(dev->s_ninode) < 50) {
		int i = swizzle16(dev->s_ninode);
		dev->s_inode[i++] = swizzle16(ino);
		dev->s_ninode = swizzle16(i);
	}
}


/* Blk_alloc is given a device number, and allocates an unused block
 * from it. A returned block number of zero means no more blocks.
 */

static blkno_t blk_alloc(int devno)
{
	register fsptr dev;
	register blkno_t newno;
	blkno_t *buf;		/*, *bread(); -- HP */
	register int j;
	int i;

	if (baddev(dev = getdev(devno)))
		goto corrupt2;

	if (swizzle16(dev->s_nfree) <= 0 || swizzle16(dev->s_nfree) > 50)
		goto corrupt;

	i = swizzle16(dev->s_nfree);
	newno = swizzle16(dev->s_free[--i]);
	dev->s_nfree = swizzle16(i);
	ifnot(newno) {
		if (dev->s_tfree != 0)
			goto corrupt;
		udata.u_error = ENOSPC;
		dev->s_nfree = swizzle16(swizzle16(dev->s_nfree) + 1);
		return (0);
	}

	/* See if we must refill the s_free array */

	ifnot(dev->s_nfree) {
		buf = (blkno_t *) bread(devno, newno, 0);
		dev->s_nfree = buf[0];
		for (j = 0; j < 50; j++) {
			dev->s_free[j] = buf[j + 1];
		}
		brelse((bufptr) buf);
	}

	validblk(devno, newno);

	ifnot(dev->s_tfree)
	    goto corrupt;
	dev->s_tfree = swizzle16(swizzle16(dev->s_tfree) - 1);

	/* Zero out the new block */
	buf = (blkno_t *) bread(devno, newno, 2);
	memset(buf, 0, 512);
	bawrite((bufptr) buf);
	return (newno);

      corrupt:
	printf("blk_alloc: corrupt\n");
	dev->s_mounted = swizzle16(1);
      corrupt2:
	udata.u_error = ENOSPC;
	return (0);
}


/* Blk_free is given a device number and a block number,
and frees the block. */

static void blk_free(int devno, blkno_t blk)
{
	register fsptr dev;
	register uint8_t *buf;
	int b;

	ifnot(blk)
	    return;

	if (baddev(dev = getdev(devno)))
		return;

	validblk(devno, blk);

	if (dev->s_nfree == swizzle16(50)) {
		buf = bread(devno, blk, 1);
		/* 50 entries in s_free + s_nfree */
		memcpy(buf, &(dev->s_nfree), 52 * sizeof(uint16_t));
		bawrite((bufptr) buf);
		dev->s_nfree = 0;
	}

	dev->s_tfree = swizzle16(swizzle16(dev->s_tfree) + 1);
	b = swizzle16(dev->s_nfree);
	dev->s_free[b++] = swizzle16(blk);
	dev->s_nfree = swizzle16(b);
}


/* Oft_alloc and oft_deref allocate and dereference (and possibly free)
 * entries in the open file table.
 */

static int oft_alloc(void)
{
	register int j;

	for (j = 0; j < OFTSIZE; ++j) {
		ifnot(of_tab[j].o_refs) {
			of_tab[j].o_refs = 1;
			of_tab[j].o_inode = NULLINODE;
			return (j);
		}
	}
	udata.u_error = ENFILE;
	return (-1);
}


static void oft_deref(int of)
{
	register struct oft *ofptr;

	ofptr = of_tab + of;
	if (!(--ofptr->o_refs) && ofptr->o_inode) {
		i_deref(ofptr->o_inode);
		ofptr->o_inode = NULLINODE;
	}
}


/* Uf_alloc finds an unused slot in the user file table.
 */

static int uf_alloc(void)
{
	register int j;

	for (j = 0; j < UFTSIZE; ++j) {
		if (udata.u_files[j] & 0x80) {	/* Portable, unlike  == -1 */
			return (j);
		}
	}
	udata.u_error = ENFILE;
	return (-1);
}


/* I_ref increases the reference count of the given inode table entry.
 */

static void i_ref(inoptr ino)
{
	if (++(ino->c_refs) == 2 * ITABSIZE) {	/* Arbitrary limit. *//*280 */
		printf("inode %u,", ino->c_num);	/*280 */
		panic("too many i-refs");
	}			/*280 */
}


/* I_deref decreases the reference count of an inode, and frees it from
 * the table if there are no more references to it.  If it also has no
 * links, the inode itself and its blocks (if not a device) is freed.
 */

static void i_deref(inoptr ino)
{
	unsigned int m;

	magic(ino);

	ifnot(ino->c_refs)
	    panic("inode freed.");

	/* If the inode has no links and no refs, it must have
	   its blocks freed. */

	ifnot(--ino->c_refs || ino->c_node.i_nlink) {
/*
 SN  (mcy)
*/

		m = swizzle16(ino->c_node.i_mode);
		if (((m & F_MASK) == F_REG) ||
		    ((m & F_MASK) == F_DIR) || ((m & F_MASK) == F_PIPE))
			f_trunc(ino);
	}
	/* If the inode was modified, we must write it to disk. */
	if (!(ino->c_refs) && ino->c_dirty) {
		ifnot(ino->c_node.i_nlink) {
			ino->c_node.i_mode = 0;
			i_free(ino->c_dev, ino->c_num);
		}
		wr_inode(ino);
	}
}


/* Wr_inode writes out the given inode in the inode table out to disk,
 * and resets its dirty bit.
 */

static void wr_inode(inoptr ino)
{
	struct dinode *buf;
	register blkno_t blkno;

	magic(ino);

	blkno = (ino->c_num >> 3) + 2;
	buf = (struct dinode *) bread(ino->c_dev, blkno, 0);
	memcpy(&buf[ino->c_num & 0x07], &ino->c_node, 64);
	bfree((bufptr) buf, 2);
	ino->c_dirty = 0;
}


/* isdevice(ino) returns true if ino points to a device */
static int isdevice(inoptr ino)
{
	return (swizzle16(ino->c_node.i_mode) & 020000);
}


/* F_trunc frees all the blocks associated with the file, if it
 * is a disk file.
 */

static void f_trunc(inoptr ino)
{
	int dev;
	int j;

	dev = ino->c_dev;

	/* First deallocate the double indirect blocks */
	freeblk(dev, swizzle16(ino->c_node.i_addr[19]), 2);

	/* Also deallocate the indirect blocks */
	freeblk(dev, swizzle16(ino->c_node.i_addr[18]), 1);

	/* Finally, free the direct blocks */
	for (j = 17; j >= 0; --j)
		freeblk(dev, swizzle16(ino->c_node.i_addr[j]), 0);

	memset((char *) ino->c_node.i_addr, 0, sizeof(ino->c_node.i_addr));

	ino->c_dirty = 1;
	ino->c_node.i_size = 0;
}


/* Companion function to f_trunc(). */
static void freeblk(int dev, blkno_t blk, int level)
{
	blkno_t *buf;
	int j;

	ifnot(blk)
	    return;

	if (level) {
		buf = (blkno_t *) bread(dev, blk, 0);
		for (j = 255; j >= 0; --j)
			freeblk(dev, swizzle16(buf[j]), level - 1);
		brelse((bufptr) buf);
	}

	blk_free(dev, blk);
}



/* Changes: blk_alloc zeroes block it allocates */
/*
 * Bmap defines the structure of file system storage by returning
 * the physical block number on a device given the inode and the
 * logical block number in a file.  The block is zeroed if created.
 */
static blkno_t bmap(inoptr ip, blkno_t bn, int rwflg)
{
	register int i;
	register bufptr bp;
	register int j;
	register blkno_t nb;
	int sh;
	int dev;

	if (fuzix_getmode(ip) == F_BDEV)
		return (bn);

	dev = ip->c_dev;

	/*
	 * blocks 0..17 are direct blocks
	 */
	if (bn < 18) {
		nb = swizzle16(ip->c_node.i_addr[bn]);
		if (nb == 0) {
			if (rwflg || (nb = blk_alloc(dev)) == 0)
				return (NULLBLK);
			ip->c_node.i_addr[bn] = swizzle16(nb);
			ip->c_dirty = 1;
		}
		return (nb);
	}

	/*
	 * addresses 18 and 19 have single and double indirect blocks.
	 * the first step is to determine how many levels of indirection.
	 */
	bn -= 18;
	sh = 0;
	j = 2;
	if (bn & 0xff00) {	/* bn > 255  so double indirect */
		sh = 8;
		bn -= 256;
		j = 1;
	}

	/*
	 * fetch the address from the inode
	 * Create the first indirect block if needed.
	 */
	ifnot(nb = swizzle16(ip->c_node.i_addr[20 - j])) {
		if (rwflg || !(nb = blk_alloc(dev)))
			return (NULLBLK);
		ip->c_node.i_addr[20 - j] = swizzle16(nb);
		ip->c_dirty = 1;
	}

	/*
	 * fetch through the indirect blocks
	 */
	for (; j <= 2; j++) {
		bp = (bufptr) bread(dev, nb, 0);
		/******
                if(bp->bf_error) {
                        brelse(bp);
                        return((blkno_t)0);
                }
                ******/
		i = (bn >> sh) & 0xff;
		if ((nb = swizzle16(((blkno_t *) bp)[i])) != 0)
			brelse(bp);
		else {
			if (rwflg || !(nb = blk_alloc(dev))) {
				brelse(bp);
				return (NULLBLK);
			}
			((blkno_t *) bp)[i] = swizzle16(nb);
			bawrite(bp);
		}
		sh -= 8;
	}
	return (nb);
}



/* Validblk panics if the given block number is not a valid
 *  data block for the given device.
 */
static void validblk(int dev, blkno_t num)
{
	register fsptr devptr;

	devptr = fs_tab + dev;

	if (devptr->s_mounted == 0)
		panic("validblk: not mounted");

	if (num < swizzle16(devptr->s_isize)
	    || num >= swizzle16(devptr->s_fsize))
		panic("validblk: invalid blk");
}


/* This returns the inode pointer associated with a user's file
 * descriptor, checking for valid data structures.
 */
static inoptr getinode(int uindex)
{
	register int oftindex;
	register inoptr inoindex;

	if (uindex < 0 || uindex >= UFTSIZE
	    || udata.u_files[uindex] & 0x80) {
		udata.u_error = EBADF;
		return (NULLINODE);
	}

	if ((oftindex = udata.u_files[uindex]) < 0 || oftindex >= OFTSIZE)
		panic("Getinode: bad desc table");

	if ((inoindex = of_tab[oftindex].o_inode) < i_tab ||
	    inoindex >= i_tab + ITABSIZE)
		panic("Getinode: bad OFT");

	magic(inoindex);

	return (inoindex);
}

/* This sets the times of the given inode, according to the flags.
 */
static void setftime(inoptr ino, int flag)
{
	time_t now;
	ino->c_dirty = 1;

	now = time(NULL);

	if (flag & A_TIME)
		ino->c_node.i_atime = swizzle32(now);
	if (flag & M_TIME)
		ino->c_node.i_mtime = swizzle32(now);
	if (flag & C_TIME)
		ino->c_node.i_ctime = swizzle32(now);
}


static int fuzix_getmode(inoptr ino)
{
	return (swizzle16(ino->c_node.i_mode) & F_MASK);
}


/* Fmount places the given device in the mount table with mount point ino.
 */
static int fmount(int dev, inoptr ino)
{
	uint8_t *buf;
	register struct filesys *fp;

	/* Dev 0 blk 1 */
	fp = fs_tab + dev;
	buf = bread(dev, 1, 0);
	memcpy(fp, buf, sizeof(struct filesys));
	brelse((bufptr) buf);

	/* See if there really is a filesystem on the device */
	if (fp->s_mounted == SMOUNTED_WRONGENDIAN)
		swizzling = 1;
	if (swizzle16(fp->s_mounted) != SMOUNTED ||
	    swizzle16(fp->s_isize) >= swizzle16(fp->s_fsize))
		return (-1);

	fp->s_mntpt = ino;
	if (ino)
		++ino->c_refs;

	return (0);
}

static void magic(inoptr ino)
{
	if (ino->c_magic != CMAGIC)
		panic("Corrupt inode");
}

static uint8_t *bread(int dev, blkno_t blk, int rewrite)
{
	register bufptr bp;

/*printf("Reading block %d\n", blk);*/

	bp = bfind(dev, blk);
	if (bp) {
		if (bp->bf_busy)
			panic("want busy block");
		goto done;
	}
	bp = freebuf();
	bp->bf_dev = dev;
	bp->bf_blk = blk;

	/* If rewrite is set, we are about to write over the entire block,
	   so we don't need the previous contents */

	ifnot(rewrite)
	    if (bdread(bp->bf_blk, bp->bf_data) == -1) {
		udata.u_error = EIO;
		return 0;
	}

      done:
	bp->bf_busy = 1;
	bp->bf_time = ++bufclock;	/* Time stamp it */
	return (bp->bf_data);
}


static void brelse(bufptr bp)
{
/*printf("Releasing block %d (0)\n", bp->bf_blk);*/
	bfree(bp, 0);
}

static void bawrite(bufptr bp)
{
/*printf("Releasing block %d (1)\n", bp->bf_blk);*/
	bfree(bp, 1);
}

static int bfree(bufptr bp, int dirty)
{
/*printf("Releasing block %d (%d)\n", bp->bf_blk, dirty);*/
	bp->bf_dirty |= dirty;
	bp->bf_busy = 0;

	if (dirty == 2) {	/* Extra dirty */
		if (bdwrite(bp->bf_blk, bp->bf_data) == -1)
			udata.u_error = EIO;
		bp->bf_dirty = 0;
		return (-1);
	}
	return (0);
}


/* This returns a busy block not belonging to any device, with
 * garbage contents.  It is essentially a malloc for the kernel.
 * Free it with brelse()!
 */
static uint8_t *tmpbuf(void)
{
	bufptr bp;
	bufptr freebuf();

	/*printf("Allocating temp block\n"); */
	bp = freebuf();
	bp->bf_dev = -1;
	bp->bf_busy = 1;
	bp->bf_time = ++bufclock;	/* Time stamp it */
	return (bp->bf_data);
}


static uint8_t *zerobuf(void)
{
	uint8_t *b = tmpbuf();
	memset(b, 0, 512);
	return (b);
}


static void bufsync(void)
{
	register bufptr bp;

	for (bp = bufpool; bp < bufpool + NBUFS; ++bp) {
		if (bp->bf_dev != -1 && bp->bf_dirty) {
			bdwrite(bp->bf_blk, bp->bf_data);
			if (!bp->bf_busy)
				bp->bf_dirty = 0;
		}
	}
}

static bufptr bfind(int dev, blkno_t blk)
{
	register bufptr bp;

	for (bp = bufpool; bp < bufpool + NBUFS; ++bp) {
		if (bp->bf_dev == dev && bp->bf_blk == blk)
			return (bp);
	}
	return (NULL);
}


static bufptr freebuf(void)
{
	register bufptr bp;
	register bufptr oldest;
	register int oldtime;

	/* Try to find a non-busy buffer and write out the data if it is dirty */
	oldest = NULL;
	oldtime = 0;
	for (bp = bufpool; bp < bufpool + NBUFS; ++bp) {
		if (bufclock - bp->bf_time >= oldtime && !bp->bf_busy) {
			oldest = bp;
			oldtime = bufclock - bp->bf_time;
		}
	}
	ifnot(oldest)
	    panic("no free buffers");

	if (oldest->bf_dirty) {
		if (bdwrite(oldest->bf_blk, oldest->bf_data) == -1)
			udata.u_error = EIO;
		oldest->bf_dirty = 0;
	}
	return (oldest);
}

static void bufinit(void)
{
	register bufptr bp;

	for (bp = bufpool; bp < bufpool + NBUFS; ++bp) {
		bp->bf_dev = -1;
	}
}


