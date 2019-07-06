/*  A very simple, lite, version of tar for the Fuzix project

    Copyright(c) 2015 Brett M Gordon

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


    NOTES:
    * cannot extract symbolic links
    * hardlinks are added as normal files

    TODO:
    * remove stdio.h gunk
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <getopt.h>
#include <dirent.h>
/* #include <time.h> */
#include <errno.h>
#include <stdlib.h>

/* tar header format, with ustar extension */
struct header {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char cksum[8];
	char type;
	char lname[100];
	char ustar[6];
	char version[2];
	char uname[32];
	char gname[32];
	char major[8];
	char minor[8];
	char prefix[155];
	char pad[12];
} h;


uint8_t buffer[512];		/* buffer for data blocks */
int infile;			/* input file: a tar file */
int outfile;			/* output file: a tar file */
int verbose;			/* verbose flag */
int uflag;			/* ustar flag */
int bf = 1;                     /* blocking factor */
char *rec;                      /* alloced record buffer */
char *recptr;                   /* ptr to next write */
char *recend;                   /* ptr to end of buffer */
int recz;                       /* size of record in bytes */
char *ofile = NULL;		/* pointer to archive name from cmd line */
char key = 0;			/* key mode of command (t,x,c) */
int noreplace = 0;		/* replace old files? */
int cksum = 1;			/* don't worry about cksums */
struct stat astat;		/* stat of archive file */


/* read blk from record */
void blkread(void *buf) {
	int ret;
	if (recptr == recend) {
		ret = read(infile, rec, recz);
		if (ret < recz) {
			fprintf(stderr,"wrong blocking factor\n");
			exit(1);
		}
		recptr = rec;
	}
	memcpy(buf,recptr,512);
	recptr += 512;
}

/* write a block to record */
int blkwrite(char *buf) {
	int ret;
	memcpy(recptr, buf, 512);
	recptr += 512;
	if (recptr == recend) {
		ret = write(outfile, rec, recz);
		if (ret < recz) {
			fprintf(stderr,"wrong blocking factor\n");
			exit(1);
		}
		recptr = rec;
	}
	return 512;
}

/* flush any partial record */
int blkflush(void) {
	if (recptr != rec) {
		bzero(recptr, recend - recptr);
		return write(outfile, rec, recz);
	}
	return 0;
}

/* Return a number from an octal string */
static uint32_t b8get(char *ptr, unsigned int n)
{
	uint32_t ret = 0;
	while (*ptr && n) {
		ret *= 8;
		ret += (*ptr++) - '0';
		n--;
	}
	return ret;
}

/* Put a number in octal to string */
static void b8put(uint32_t x, char *ptr)
{
	int digit;

	*--ptr = 0;
	while (x) {
		digit = x & 7;
		x /= 8;
		*--ptr = '0' + digit;
	}
}

/* Print filename for error reporting */
static void pname(void)
{
	fprintf(stderr, "%s ", h.name);
	perror("");
}


/* Change owner/group of file to match tar file */
static void my_chown(void)
{
	int x = chown(h.name, b8get(h.uid, 8),
		      b8get(h.gid, 8));
}


/* Change mod of file to match tar file */
static void my_chmod(void)
{
	int x = chmod(h.name, b8get(h.mode, 8));
	if (x < 0)
		pname();
}

static void printusage(void)
{
	fprintf(stderr, "usage: tar [-cbkxntv] [-f archive] [file]...\n");
	exit(1);
}

/* skip data blocks of regular type file */
static void skip(void)
{
	uint32_t size = b8get(h.size, 12);
	uint32_t count = size / 512;
	uint16_t rem = size % 512;

	if (h.type == 0 || h.type == '0') {
		off_t x;
		if (rem)
			count++;
		x = lseek(infile, count * 512, SEEK_CUR);
		if (x < 0) {
			pname();
			exit(1);
		}

	}
}

/* Calculate sum of header */
static uint32_t cksum_calc(void)
{
	uint32_t acc = 0;
	unsigned char *ptr = (unsigned char *) h.name;
	while (ptr != (unsigned char *) h.cksum)
		acc += *ptr++;
	acc += 32 * 8;
	ptr = &(h.type);
	while (ptr != (unsigned char *) h.pad)
		acc += *ptr++;
	return acc;
}


/* Prints information in header to stdout (if applicable) */
static void printheader(void)
{
	char pad[10];
	int pindex = 10;
	unsigned int m;
	int x;

	pad[10] = 0;

	/* don't print if creating to stdout */
	if (!ofile && key == 'c')
		return;

	/* print verbosely */
	if (verbose && key == 't') {
		/* print type */
		switch (h.type) {
		case '0':
		case 0:
			printf("-");
			break;
		case '1':
			printf("h");
			break;
		case '2':
			printf("s");
			break;
		case '3':
			printf("c");
			break;
		case '4':
			printf("b");
			break;
		case '5':
			printf("d");
			break;
		case '6':
			printf("f");
			break;
		default:
			printf("?");
			break;
		}
		/* print mode */
		m = b8get(h.mode, 8);
		for (x = 0; x < 3; x++) {
			if (m & 1)
				pad[--pindex] = 'x';
			else
				pad[--pindex] = '-';
			m = m / 2;
			if (m & 1)
				pad[--pindex] = 'w';
			else
				pad[--pindex] = '-';
			m = m / 2;

			if (m & 1)
				pad[--pindex] = 'r';
			else
				pad[--pindex] = '-';
			m = m / 2;
		}
		printf(&(pad[pindex]));
		/* print uid gid */
		printf(" %4ld %4ld", b8get(h.uid, 8), b8get(h.gid, 8));
		/* print file size */
		printf(" %8ld ", b8get(h.size, 12));
		/* print maj/min */

		/* This make binary big... disabling */
		/*
		   printf( " %2ld,%2ld", b8get( h.major, 8 ),
		   b8get( h.minor, 8 ) );
		   {
		   char *s;
		   time_t time=b8get( h.mtime, 12 );
		   s=&( ctime( &time )[4] );
		   s[16]=0;
		   printf( " %-12s", s );
		   }
		 */

	}
	if (key == 't' || verbose)
		printf("%s\n", h.name);
}


/* process a file name
   This is called recursively as new subdirs are found */
static void storedir(char *name)
{
	struct stat s;
	int fd;
	int ret;
	DIR *dirstream;
	struct dirent *dir;
	char cname[100];
	uint32_t count;
	uint16_t rem;

	/* stat file */
	ret = stat(name, &s);
	if (ret < 0) {
		fprintf(stderr, "%s ", name);
		perror("");
		return;
	}

	/* check to make sure we're not trying to archive
	   the archive */
	if ((s.st_ino == astat.st_ino) && (s.st_dev == astat.st_dev))
		return;

	if (S_ISREG(s.st_mode)) {
		count = s.st_size / 512;
		rem = s.st_size % 512;
		if (rem)
			count++;
	}

	/* build a new header in mem */
	bzero(h.name, sizeof(h));
	memset(h.mode, '0', 57);
	memset(h.major, '0', 16);
	strcpy(h.name, name);
	b8put(s.st_mode & 07777, h.mode + 8);
	b8put(s.st_uid, h.uid + 8);
	b8put(s.st_gid, h.gid + 8);
	b8put(s.st_size, h.size + 12);
	b8put(s.st_mtime, h.mtime + 12);
	switch (s.st_mode & S_IFMT) {
	case S_IFREG:
		h.type = '0';
		break;
	case S_IFDIR:
		h.type = '5';
		if (S_ISDIR(s.st_mode))
			h.name[strlen(name)] = '/';
		break;
	case S_IFCHR:
		h.type = '3';
		b8put(s.st_rdev >> 8, h.major + 8);
		b8put(s.st_rdev & 255, h.minor + 8);
		break;
	case S_IFBLK:
		h.type = '4';
		b8put(s.st_rdev >> 8, h.major + 8);
		b8put(s.st_rdev & 255, h.minor + 8);
		break;
	case S_IFIFO:
		h.type = '6';
		break;
	default:
		fprintf(stderr, "unhandled file type error\n");
		exit(1);
	}

	strcpy(h.ustar, "ustar");
	b8put(0, h.version + 2);

	/* calculate checksum */
	h.cksum[7] = 32;
	b8put(cksum_calc(), h.cksum + 7);

	/* write header to file */
	ret = blkwrite(h.name);
	if (ret < 512) {
		pname();
		exit(1);
	}
	printheader();

	switch (s.st_mode & S_IFMT) {
	case S_IFREG:
		/* open subject file */
		fd = open(name, O_RDONLY);
		if (fd < 0) {
			fprintf(stderr, "%s ", name);
			perror("");
			return;
		}
		while (count--) {
			ret = read(fd, buffer, 512);
			if (ret < 0) {
				fprintf(stderr, "cannot read source file\n");
				exit(1);
			}
			if (ret < 512)
				bzero(buffer + ret, 512 - ret);
			ret = blkwrite(buffer);
		}
		close(fd);
		break;
	case S_IFDIR:
		/* open the directory */
		if ((dirstream = opendir(name)) == NULL) {
			pname();
			break;
		}
		while (dir = readdir(dirstream)) {
			/* dont arch .. or . */
			if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")
			    )
				continue;
			strncpy(cname, name, 100);
			strncat(cname, "/", 100);
			strncat(cname, dir->d_name, 100);
			{
				/* save state of files */
				uint32_t tell = telldir(dirstream);
				closedir(dirstream);
				/* recursive call to this dir */
				storedir(cname);
				/* restore state of files */
				if ((dirstream = opendir(name)) == NULL) {
					pname();
					break;
				}
				seekdir(dirstream, tell);
			}
		}
		closedir(dirstream);
		break;
	}
}


/* list all the files in an archive */
static void list(void)
{
	int x;
	int zcount = 2;


	if (ofile)
		infile = open(ofile, O_RDONLY);
	else
		infile = 0;
	if (infile < 0) {
		perror("Cannot open archive");
		exit(1);
	}

	while (1) {
		blkread(&h);
		/* check for zero block */
		if (h.name[0] == 0) {
			if (!--zcount)
				exit(0);
			continue;
		}

		uflag = !strncmp(h.ustar, "ustar", 5);

		printheader();

		skip();
	}			/* block while */
}

/* Attempt to make all parent directories */
static void makeparents(char *path)
{
	static char c[100];
	char *s = path;
	int l;
	while (1){
		char *e = index(s,'/');
		if (!e)
			return;
		l = e - path;
		memcpy(c,path,l);
		c[l] = 0;
		mkdir(c,0755);
		s = e + 1;
	}
}

/* Extract all file in archive */
static void extract(char *argv[])
{
	int x;
	int zcount = 2;

	if (ofile)
		infile = open(ofile, O_RDONLY);
	else
		infile = 0;

	if (infile < 0) {
		fprintf(stderr, "cannot open infile\n");
		exit(1);
	}

	while (1) {
		blkread(&h);
		/* check for zero block */
		if (h.name[0] == 0) {
			if (!--zcount)
				exit(0);
			continue;
		}

		/* check cksum */
		if (cksum && (cksum_calc() != b8get(h.cksum, 8))) {
			fprintf(stderr, "%s: bad chksum\n", h.name);
			exit(1);
		}

		/* remove trailing '/' */
		while (h.name[strlen(h.name) - 1] == '/')
			h.name[strlen(h.name) - 1] = 0;

		/* does entry match any cmd line args? */
		if (argv[optind]) {
			for (x = optind; argv[x]; x++) {
				if (!strcmp(h.name, argv[x]))
					goto cont;
			}
			skip();
			continue;
		}
	      cont:
		printheader();

		uflag = !strncmp(h.ustar, "ustar", 5);

		makeparents(h.name);

		switch (h.type) {
		case '1':	/* a hard link */
			x = link(h.lname, h.name);
			if (x < 0) {
				pname();
				break;
			}
			my_chmod();
			my_chown();
			break;
		case '2':	/* a soft link */
			fprintf(stderr, "soft links not supported\n");
			break;
		case '3':	/* a charactor device */
			x = mknod(h.name, b8get(h.mode, 8) | S_IFCHR, (b8get(h.major, 8) << 8) + b8get(h.minor, 8));
			if (x > 0) {
				pname();
				break;
			}
			my_chown();
			break;
		case '4':	/* a block device */
			x = mknod(h.name, b8get(h.mode, 8) | S_IFBLK, (b8get(h.major, 8) << 8) + b8get(h.minor, 8));
			if (x < 0) {
				pname();
				break;
			}
			my_chown();
			break;
		case '5':	/* a directory */
			x = mkdir(h.name, b8get(h.mode, 8));
			if (x) {
				pname();
				break;
			}
			my_chown();
			break;
		case '6':	/* a FIFO */
			x = mkfifo(h.name, b8get(h.mode, 8));
			if (x < 0) {
				pname();
				break;
			}
			my_chown();
			break;
		case '0':	/* regular file */
		case 0:
			{
				int i;
				uint16_t rem;
				/* get a printable size */
				uint32_t size = b8get(h.size, 12);
				uint32_t count;
				/* count how many blocks we need */
				count = size / 512;
				rem = size % 512;
				/* check for existance of file */
				if (noreplace && !access(h.name, F_OK)) {
					errno = EEXIST;
					pname();
					continue;
				}
				/* open output file */
				outfile = open(h.name, O_CREAT | O_WRONLY | O_TRUNC, 0700);
				if (outfile < 0) {
					pname();
					break;
				}
				/* send buffers to file */
				for (x = 0; x < count; x++) {
					blkread(buffer);
					i = write(outfile, buffer, 512);
					if (i < 512) {
						fprintf(stderr, "cannot write out file\n");
						exit(1);
					}
				}
				if (rem) {
					blkread(buffer);
					i = write(outfile, buffer, rem);
					if (i < rem) {
						fprintf(stderr, "cannot write out file\n");
						exit(1);
					}
				}
				close(outfile);
				my_chmod();
				my_chown();
				continue;
			}
			break;
		default:
			fprintf(stderr, "unsupported flag\n");
			break;
		}
	}			/* block while */
}


/* Create a archive */
static void create(char *argv[])
{
	int x;

	/* open outfile */
	if (ofile)
		outfile = open(ofile, O_CREAT | O_WRONLY, 0666);
	else
		outfile = 1;
	if (outfile < 0) {
		pname();
		exit(1);
	}
	/* get dev / inode numbers of created tar file for comparing to
	   added files, in order to skip adding a tar file to itself */
	if (fstat(outfile, &astat)) {
		pname();
		exit(1);
	}
	/* put each file on cmdline to file */
	while (argv[optind]) {
		char *s = argv[optind++];
		/* remove any trailing / */
		if (s[strlen(s) - 1] == '/')
			s[strlen(s) - 1] = 0;
		/* remove any leading / */
		if (s[0] == '/')
			s = s + 1;
		storedir(s);
	}
	/* write out 2 zero blocks */
	bzero(buffer, 512);
	x = blkwrite(buffer);
	x += blkwrite(buffer);
	if (x != 1024)
		perror("writing end of archive");
	/* close outfile */
	blkflush();
	close(outfile);
}


int main(int argc, char *argv[])
{
	int o;

	ofile = getenv("TAPE");
	
	while ((o = getopt(argc, argv, "xtcvnkf:b:")) > 0) {
		switch (o) {
		case 'x':
		case 't':
		case 'c':
			key = o;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'f':
			ofile = optarg;
			break;
		case 'k':
			noreplace = 1;
			break;
		case 'n':
			cksum = 0;
			break;
		case 'b':
			bf = atoi(optarg);
			if (bf < 1) {
				fprintf(stderr,"bad blocking factor\n");
				exit(1);
			}
			break;
		default:
			printusage();
		}
	}
	/* setup record buffer */
	recz = bf * 512;
	recptr = rec = sbrk(recz);
	if (rec == (void *)-1) {
		fprintf(stderr,"block buffer too large\n");
		exit(1);
	}
	recend = rec + recz;
	switch (key) {
	case 'x':
		recptr = recend;
		extract(argv);
		break;
	case 't':
		recptr = recend;
		list();
		break;
	case 'c':
		create(argv);
		break;
	default:
		fprintf(stderr, "tar: option x,c, or t must be used\n");
		exit(1);
	}
	return 0;
}
