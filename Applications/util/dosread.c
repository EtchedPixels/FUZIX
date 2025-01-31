/* dos{dir|read|write|del} - {list|read|write|del} MS-DOS disks	 Author: M. Huisjes */

/* dosdir - list MS-DOS directories to stdout
 * doswrite - write stdin/file to DOS-file
 * dosread - read DOS-file to stdout/file
 * dosdel - delete DOS-file
 *
 * Author: Michiel Huisjes.
 *
 * Usage: dos... [-lra] drive [ms-dos file/dir] [file]
 *	  l: Give long listing.
 *	  r: List recursively.
 *	  a: Set ASCII bit.
 */

/* no assertions  - let's save memory! */
#define NDEBUG
#undef CACHE_ROOT

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef MSX_UZIX_TARGET
/* these are true numbers, but you can increase if you have memory */
#define MAX_CLUSTER_SIZE	8192
#define MAX_ROOT_ENTRIES	256
#else
#ifdef PC_UZIX_TARGET
/* due to limitation of memory... */
#define MAX_CLUSTER_SIZE	2048
#define MAX_ROOT_ENTRIES	256
#else
#define MAX_CLUSTER_SIZE	4096
#define MAX_ROOT_ENTRIES	512
#endif
#endif
#define FAT_START		512L	/* After bootsector */
#define ROOTADDR		(FAT_START + 2L * fat_size)
#define clus_add(cl_no)		((long) (((long) cl_no - 2L) \
				 * (long) cluster_size \
				 + data_start \
			        ))
struct dir_entry {
  unsigned char d_name[8];
  unsigned char d_ext[3];
  unsigned char d_attribute;
  unsigned char d_reserved[10];
  unsigned short d_time;
  unsigned short d_date;
  unsigned short d_cluster;
  unsigned long d_size;
};

typedef struct dir_entry DIRECTORY;

#define NOT_USED	0x00
#define ERASED		0xE5
#define DIR		0x2E
#define DIR_SIZE	(sizeof (struct dir_entry))
#define SUB_DIR		0x10
#define NIL_DIR		((DIRECTORY *) 0)

#define LAST_CLUSTER12	0xFFF
#define LAST_CLUSTER	0xFFFF
#define FREE		0x000
#define BAD		0xFF0
#define BAD16		0xFFF0

#define FCLOSE		if (fdo != stdout) fclose(fdo)
#define FABORT(x)	FCLOSE; exit(x)

typedef int BOOL;

#define TRUE	1
#define FALSE	0
#define NIL_PTR	((char *) 0)

#define DOS_TIME	315532800L	/* 1970 - 1980 */

#define READ			0
#define WRITE			1

#define FIND	3
#define LABEL	4
#define ENTRY	5
#define find_entry(d, e, p)	directory(d, e, FIND, p)
#define list_dir(d, e, f)	directory(d, e, f, NIL_PTR)
#define label()			directory(root, root_entries, LABEL, NIL_PTR)
#define new_entry(d, e)		directory(d, e, ENTRY, NIL_PTR)

#define is_dir(d)		((d)->d_attribute & SUB_DIR)

#define STD_OUT			1

char	*cmnd;

static int disk;	/* File descriptor for disk I/O */

#ifdef CACHE_ROOT
static DIRECTORY root[MAX_ROOT_ENTRIES];
#else
static DIRECTORY *root = NULL;
#endif
static DIRECTORY save_entry;
static char drive[] = "/dev/dosX";
static char drive2[] = "/dev/fdX";
#define DRIVE_NR	(sizeof (drive) - 2)
static char buffer[MAX_CLUSTER_SIZE], *device = drive, path[128];
static long data_start,root_address, fat_start;
static long mark;	/* offset of directory entry to be written */
static unsigned short total_clusters, cluster_size, root_entries, sub_entries;
static unsigned long fat_size;
static char *unixfile;
FILE *fdo;

static BOOL Rflag, Lflag, Aflag, dos_read, dos_write, dos_dir, dos_del, fat_16 = 0;
static BOOL big_endian;

/* maximum size of a cooked 12bit FAT. Also Size of 16bit FAT cache
 * if not enough memory for whole FAT
 */
#ifdef MSX_UZIX_TARGET
#define COOKED_SIZE		512
#else
#define COOKED_SIZE		8192
#endif
/* raw FAT. Only used for 12bit FAT to make conversion easier 
 */
static unsigned char	*raw_fat;
/* Cooked FAT. May be only part of the FAT for 16 bit FATs
 */
static unsigned short	*cooked_fat;
/* lowest and highest entry in fat cache
 */
static unsigned short	fat_low = USHRT_MAX,
			fat_high = 0;
static BOOL		fat_dirty = FALSE;
static unsigned int	cache_size;
static unsigned long 	rawfat_size;


/* Prototypes. */
void usage(const char *prog_name);
unsigned c2u2(const unsigned char *ucarray);
unsigned long c4u4(const unsigned char *ucarray);
void determine(void);
int main(int argc, char *argv []);
DIRECTORY *directory(DIRECTORY *dir, int entries, BOOL function, char *pathname);
void extract(DIRECTORY *entry);
void delete(DIRECTORY *entry);
void make_file(DIRECTORY *dir_ptr, int entries, char *name);
void fill_date(DIRECTORY *entry);
char *make_name(register DIRECTORY *dir_ptr, short dir_fl);
int fill(char *buff, size_t size);
void xmodes(int mode);
void show(DIRECTORY *dir_ptr, char *name);
void free_blocks(void);
DIRECTORY *read_cluster(unsigned int cluster);
unsigned short free_cluster(BOOL leave_fl);
void link_fat(unsigned int cl_1, unsigned int cl_2);
unsigned short next_cluster(unsigned int cl_no);
unsigned short clear_cluster(unsigned int cl_no);
char *slash(char *str);
void add_path(char *file, BOOL slash_fl);
void disk_io(BOOL op, unsigned long seek, void *address, unsigned bytes);
void flush_fat(void);
void read_fat(unsigned int cl_no);
BOOL free_range(unsigned short *first, unsigned short *last);
long lmin(long a, long b);

void usage(const char *prog_name)
{
  fprintf (stderr, "usage: %s ", prog_name);
  if (dos_dir) fprintf(stderr, "[-lr] drive [dir]\n");
  if (dos_read || dos_write) fprintf(stderr, "[-a] drive dosfile [file]\n");
  if (dos_del) fprintf(stderr, "drive dosfile\n");
  exit(1);
}

unsigned c2u2(const unsigned char *ucarray)
{
  return ucarray[0] + (ucarray[1] << 8);	/* parens vital */
}

unsigned long c4u4(const unsigned char *ucarray)
{
  return ucarray[0] + ((unsigned long) ucarray[1] << 8) +
		      ((unsigned long) ucarray[2] << 16) +
		      ((unsigned long) ucarray[3] << 24);
}

void determine(void)
{
  struct dosboot {
	unsigned char cjump[2];	/* unsigneds avoid bugs */
	unsigned char nop;
	unsigned char name[8];
	unsigned char cbytepers[2];	/* don't use shorts, etc */
	unsigned char secpclus;		/* to avoid struct member */
	unsigned char creservsec[2];	/* alignment and byte */
	unsigned char fats;		/* order bugs */
	unsigned char cdirents[2];
	unsigned char ctotsec[2];
	unsigned char media;
	unsigned char csecpfat[2];
	unsigned char csecptrack[2];
	unsigned char cheads[2];
	unsigned char chiddensec[2];
	unsigned char dos4hidd2[2];
	unsigned char dos4totsec[4];
	/* Char    fill[476]; */
  } boot;
  unsigned short boot_magic;	/* last of boot block */
  unsigned bytepers, reservsec, dirents;
  unsigned secpfat, secptrack, /*heads, */hiddensec;
  unsigned long totsec;
  unsigned char fat_info, fat_check;
  unsigned short endiantest = 1;
  int errcount = 0;

  big_endian = !(*(unsigned char *)&endiantest);

  /* Read Bios-Parameterblock */
  disk_io(READ, 0L, &boot, sizeof boot);
  disk_io(READ, 0x1FEL, &boot_magic, sizeof boot_magic);

  /* Convert some arrays */
  bytepers = c2u2(boot.cbytepers);
  reservsec = c2u2(boot.creservsec);
  dirents = c2u2(boot.cdirents);
  totsec = c2u2(boot.ctotsec);
  if (totsec == 0) totsec = c4u4(boot.dos4totsec);
  secpfat = c2u2(boot.csecpfat);
  secptrack = c2u2(boot.csecptrack);
/*  heads = c2u2(boot.cheads); */

  /* The `hidden sectors' are the sectors before the partition.
   * The calculation here is probably wrong (I think the dos4hidd2
   * bytes are the msbs), but that doesn't matter, since the
   * value isn't used anyway
   */
  hiddensec = c2u2(boot.chiddensec);
  if (hiddensec == 0) hiddensec = c2u2 (boot.dos4hidd2);

#ifdef PC_UZIX_TARGET
  /* Safety checking */
  if (boot_magic != 0xAA55) {
	fprintf (stderr, "%s: magic != 0xAA55\n", cmnd);
	++errcount;
  }
#endif
  
  /* Check sectors per track instead of inadequate media byte */
  if (secptrack < 15 &&		/* assume > 15 hard disk & wini OK */
#ifdef SECT10			/* BIOS modified for 10 sec/track */
      secptrack != 10 &&
#endif
#ifdef SECT8			/* BIOS modified for 8 sec/track */
      secptrack != 8 &&
#endif
      secptrack != 9) {
	fprintf (stderr, "%s: %d sectors per track not supported\n", cmnd, secptrack);
	++errcount;
  }
  if (bytepers == 0) {
	fprintf (stderr, "%s: bytes per sector == 0\n", cmnd);
	++errcount;
  }
  if (boot.secpclus == 0) {
	fprintf (stderr, "%s: sectors per cluster == 0\n", cmnd);
	++errcount;
  }
  if (boot.fats != 2 && (dos_write || dos_del)) {
	fprintf (stderr, "%s: fats != 2\n", cmnd);
	++errcount;
  }
  if (reservsec < 1) {
	fprintf (stderr, "%s: reserved < 1\n", cmnd);
	++errcount;
  }
  if (errcount != 0) {
	fprintf (stderr, "%s: Can't handle disk\n", cmnd);
	exit(2);
  }

  /* Calculate everything. */
  if (boot.secpclus == 0) boot.secpclus = 1;
  total_clusters =
	(totsec - boot.fats * secpfat - reservsec -
	 dirents * 32L / bytepers		    ) / boot.secpclus + 2;
  	/* first 2 entries in FAT aren't used */
  cluster_size = bytepers * boot.secpclus;
  fat_size = (unsigned long) secpfat * (unsigned long) bytepers;
  data_start = (long) (bytepers * reservsec) + (long) boot.fats * fat_size
	+ (long) dirents *32L;
  root_entries = dirents;
  sub_entries = boot.secpclus * bytepers / 32;
  fat_start = (long)(bytepers * reservsec);	/* After bootsector + reserved */
  root_address = (fat_start + 2L * fat_size);
  if (total_clusters > 4096) fat_16 = 1;

  /* Further safety checking */
  if (cluster_size > MAX_CLUSTER_SIZE) {
	fprintf (stderr, "%s: cluster size too big\n", cmnd);
	++errcount;
  }
#ifndef CACHE_ROOT
  if (dirents * DIR_SIZE > MAX_CLUSTER_SIZE) {
  	fprintf(stderr, "%s: root directory too big\n", cmnd);
  	++errcount;
  }
#endif
  
  disk_io(READ, fat_start, &fat_info, 1);
  disk_io(READ, fat_start + fat_size, &fat_check, 1);
  if (fat_check != fat_info) {
	fprintf (stderr, "%s: Disk type in FAT copy differs from disk type in FAT original.\n", cmnd);
	++errcount;
  }
  if (errcount != 0) {
	fprintf (stderr, "%s: Can't handle disk\n", cmnd);
	exit(2);
  }
}

int main(int argc, char *argv[])
{
  register char *arg_ptr = slash(argv[0]);
  DIRECTORY *entry;
  short idx = 1;
  char dev_nr = '0';

  fdo = stdout;
  cmnd = arg_ptr;	/* needed for error messages */
  if (!strcmp(arg_ptr, "dosdir"))
	dos_dir = TRUE;
  else if (!strcmp(arg_ptr, "dosread"))
	dos_read = TRUE;
  else if (!strcmp(arg_ptr, "doswrite"))
	dos_write = TRUE;
  else if (!strcmp(arg_ptr, "dosdel"))
	dos_del = TRUE;
  else {
	fprintf (stderr, "%s: Program should be named dosread, doswrite, dosdel or dosdir.\n", cmnd);
	exit(1);
  }

  if (argc == 1) usage(argv[0]);

  if (argv[1][0] == '-') {
	for (arg_ptr = &argv[1][1]; *arg_ptr; arg_ptr++) {
		if (*arg_ptr == 'l' && dos_dir) {
			Lflag = TRUE;
		} else if (*arg_ptr == 'r' && dos_dir) {
			Rflag = TRUE;
		} else if (*arg_ptr == 'a' && !dos_dir && !dos_del) {
			assert ('\n' == 10);
			assert ('\r' == 13);
			Aflag = TRUE;
		} else {
			usage(argv[0]);
		}
	}
	idx++;
  }
  if (idx == argc) usage(argv[0]);

  if (strlen(argv[idx]) > 1) {
  	if (*(argv[idx]+1) == ':') {
  		if ((dev_nr = toupper (*argv[idx])) < 'A' || dev_nr > 'Z')
  			usage(argv[0]);
  		dev_nr = dev_nr - 'A' + '0';
  		device = drive2;
  		device[7] = dev_nr;
  		if (*(argv[idx]+2) == '\0')
  			idx++;
  		else
  			argv[idx] = (argv[idx] + 2);
  	} else {
		device = argv[idx++];

		/* If the device does not contain a / we assume that it
		 * is the name of a device in /dev. Instead of prepending
		 * /dev/ we try to chdir there.
		 */
		if (strchr(device, '/') == NULL && chdir("/dev") < 0) {
			perror("/dev");
			exit(1);
		}
		dev_nr = device[strlen(device)-1];
	}
  } else {
	if ((dev_nr = toupper (*argv[idx++])) < 'A' || dev_nr > 'Z')
		usage(argv[0]);

	device[DRIVE_NR] = dev_nr;
  }
  if ((disk = open(device, (dos_write || dos_del) ? O_RDWR : O_RDONLY)) < 0) {
	fprintf (stderr, "%s: cannot open %s: %s\n",
		 cmnd, device, strerror (errno));
	exit(1);
  }
  determine();

#ifdef CACHE_ROOT
  disk_io(READ, root_address, root, 
  	root_entries > MAX_ROOT_ENTRIES ? 
  	DIR_SIZE * MAX_ROOT_ENTRIES : 
  	DIR_SIZE * root_entries);
  if (root_entries > MAX_ROOT_ENTRIES) {
	fprintf (stderr, "%s: root dir limited to %d files\n", cmnd, MAX_ROOT_ENTRIES);
  }
#endif

  if (dos_read || dos_write) {
  	unixfile = argv[idx+1];
  	if (unixfile == NIL_PTR) {
  		fdo = stdout;
  		if (dos_write) fdo = stdin;
  	} else {
  		if ((fdo = fopen(unixfile, dos_read ? "wb" : "rb")) == NULL) {
  			perror(argv[0]);
  			exit(1);
  		}
  	}
  }

  if (dos_dir && Lflag) {
	entry = label();
	fprintf (fdo, "Volume in drive %c ", dev_nr - '0' + 'A');
	if (entry == NIL_DIR)
		fprintf(fdo, "has no label.\n\n");
	else
		fprintf (fdo, "is %.11s\n\n", entry->d_name);
  }
  if (argv[idx] == NIL_PTR) {
	if (!dos_dir) usage(argv[0]);
	if (Lflag) fprintf (fdo, "Root directory:\n");
	list_dir(root, root_entries, FALSE);
	if (Lflag) free_blocks();
	fflush (fdo);
	FABORT(0);
  }
  for (arg_ptr = argv[idx]; *arg_ptr; arg_ptr++)
	if (*arg_ptr == '\\')	*arg_ptr = '/';
	else		     	*arg_ptr = toupper (*arg_ptr);
  if (*--arg_ptr == '/') *arg_ptr = '\0';	/* skip trailing '/' */

  add_path(argv[idx], FALSE);
  add_path("/", FALSE);

  if (dos_dir && Lflag) fprintf (fdo, "Directory %s:\n", path);

  entry = find_entry(root, root_entries, argv[idx]);

  if (dos_dir) {
	list_dir(entry, sub_entries, FALSE);
	if (Lflag) free_blocks();
  } else if (dos_read)
	extract(entry);
  else if (dos_del)
  	delete(entry);
  else {
	if (entry != NIL_DIR) {
		fflush (fdo);
		if (is_dir(entry))
			fprintf (stderr, "%s: %s is a directory.\n", cmnd, path);
		else
			fprintf (stderr, "%s: %s already exists.\n", cmnd, argv[idx]);
		exit(1);
	}
	add_path(NIL_PTR, TRUE);

	if (*path) make_file(find_entry(root, root_entries, path),
			  sub_entries, slash(argv[idx]));
	else
		make_file(root, root_entries, argv[idx]);
  }

  close(disk);
  fflush (fdo);
  if (fdo != stdout) fclose(fdo);
  return(0);
}


/* General directory search routine.
 * 
 * dir:
 *	Points to one or more directory entries
 *	if dir == root, when ROOT_CACHE is defined, dir points to the 
 *	entire root directory; if ROOT_CACHE is not defined, root dir
 *	will be read from disk when needed. If dir != root, it points
 *	to a single directory entry describing the directory to be
 *	searched.
 *
 * entries:
 *	number of entries
 *	
 * function:
 *	FIND ... find pathname relative to directory dir.
 *	LABEL ... find first label entry in dir.
 *	ENTRY ... create a new empty entry.
 *	FALSE ... list directory
 *
 * pathname:
 *	name of the file to be found or directory to be listed.
 *	must be in upper case, pathname components must be
 *	separated by slashes, but can be longer than than 
 *	8+3 characters (The rest is ignored).
 */
DIRECTORY *directory(DIRECTORY *dir, int entries, int function, char *pathname)
{
  register DIRECTORY *dir_ptr = dir;
  DIRECTORY *mem = NIL_DIR;
  unsigned short cl_no = dir->d_cluster;
  unsigned short type, last = 0;
  char file_name[14];
  char dir_bkp[DIR_SIZE];
  char *name;
  int i = 0;

  if (dir != NULL) {
  	memcpy((char *)dir_bkp, (char *)dir, DIR_SIZE);
  	dir = (void *)dir_bkp;
  }
  if (function == FIND) {
  	while (*pathname == '/') pathname++;
	while (*pathname != '/' && *pathname != '.' && *pathname &&
	       i < 8) {
		file_name[i++] = *pathname++;
	}
	if (*pathname == '.') {
		int j = 0;
		file_name[i++] = *pathname++;
		while (*pathname != '/' && *pathname != '.' && *pathname &&
		       j++ < 3) {
			file_name[i++] = *pathname++;
		}
	}
	while (*pathname != '/' && *pathname) pathname++;
	file_name[i] = '\0';
  }
  do {
	if (dir != root) {
		mem = dir_ptr = read_cluster(cl_no);
		last = cl_no;
		cl_no = next_cluster(cl_no);
	}
#ifndef CACHE_ROOT
	 else {
		disk_io(READ, root_address, buffer, root_entries * DIR_SIZE);
		dir_ptr = (void *)buffer;
		cl_no = dir_ptr->d_cluster;		
	}
#endif	
	for (i = 0; i < entries; i++, dir_ptr++) {
		type = dir_ptr->d_name[0] & 0x0FF;
		if (!mem)
			mark = root_address + (long) i *(long) DIR_SIZE;
		else
			mark = clus_add(last) + (long) i *(long) DIR_SIZE;
		if (function == ENTRY) {
			if (type == NOT_USED || type == ERASED) return dir_ptr;
			continue;
		}
		if (type == NOT_USED) break;
		if (dir_ptr->d_attribute & 0x08) {
			if (function == LABEL) return dir_ptr;
			continue;
		}
		if (type == DIR || type == ERASED || function == LABEL)
			continue;
		type = is_dir(dir_ptr);
		name = make_name(dir_ptr,
				 (function == FIND) ?  FALSE : type);
		if (function == FIND) {
			if (strcmp(file_name, name) != 0) continue;
			if (!type) {
				if (dos_dir || *pathname) {
					fflush (stdout);
					fprintf (stderr, "%s: Not a directory: %s\n", cmnd, file_name);
					exit(1);
				}
			} else if (*pathname == '\0' && dos_read) {
				fflush (stdout);
				fprintf (stderr, "%s: %s is a directory.\n", cmnd, path);
				exit(1);
			}
			if (*pathname) {
				dir_ptr = find_entry(dir_ptr,
					 sub_entries, pathname + 1);
			}
			if (mem) {
				if (dir_ptr) {
					memcpy((char *)&save_entry, (char *)dir_ptr, DIR_SIZE);
					dir_ptr = &save_entry;
				}
			}
			return dir_ptr;
		} else {
			if (function == FALSE) {
				show(dir_ptr, name);
			} else if (type) {	/* Recursive */
				printf ( "Directory %s%s:\n", path, name);
				add_path(name, FALSE);
				list_dir(dir_ptr, sub_entries, FALSE);
				add_path(NIL_PTR, FALSE);
#ifndef CACHE_ROOT
				/* Re-read directory from disk */
				mem = dir_ptr = read_cluster(cl_no);
#endif
			}
		}
	}
  } while (cl_no != LAST_CLUSTER && mem);

  switch (function) {
      case FIND:
	if (dos_write && *pathname == '\0') return NIL_DIR;
	fflush (stdout);
	fprintf (stderr, "%s: Cannot find `%s'.\n", cmnd, file_name);
	exit(1);
      case LABEL:
	return NIL_DIR;
      case ENTRY:
	if (!mem) {
		fflush (stdout);
		fprintf (stderr, "%s: No entries left in root directory.\n", cmnd);
		exit(1);
	}
	cl_no = free_cluster(TRUE);
	link_fat(last, cl_no);
	link_fat(cl_no, LAST_CLUSTER);
	memset(buffer, 0, cluster_size);
	disk_io(WRITE, clus_add(cl_no), buffer, cluster_size);

	return new_entry(dir, entries);
      case FALSE:
	if (Rflag) {
		printf ("\n");
		list_dir(dir, entries, TRUE);
	}
  }
  return NULL;
}

void delete(DIRECTORY *entry)
{
  register unsigned short cl_no = entry->d_cluster;
 
  entry->d_name[0] = 0xe5;
  disk_io(WRITE, mark, entry, DIR_SIZE);
  while (cl_no != LAST_CLUSTER) cl_no = clear_cluster(cl_no);
  if (fat_dirty) flush_fat ();
}

void extract(DIRECTORY *entry)
{
  register unsigned short cl_no = entry->d_cluster;
  int rest, i;
  long size = entry->d_size;

  if (size == 0)	/* Empty file */
	return;

  do {
	disk_io(READ, clus_add(cl_no), buffer, cluster_size);
	rest = (size > (long) cluster_size) ? cluster_size : (short)size;

	if (Aflag) {
		for (i = 0; i < rest; i ++) {
			if (buffer [i] != '\r') fputc (buffer [i], fdo);
		}
		if (ferror (stdout)) {
			fprintf (stderr, "%s: cannot write file: %s\n",
				 cmnd, strerror (errno));
			FABORT(1);
		}
	} else {
		if (fwrite (buffer, 1, rest, fdo) != rest) {
			fprintf (stderr, "%s: cannot write file: %s\n",
				 cmnd, strerror (errno));
			FABORT(1);
		}
	}
	size -= (long) rest;
	cl_no = next_cluster(cl_no);
	if (cl_no == BAD16) {
		fflush (stdout);
		fprintf (stderr, "%s: reserved cluster value %x encountered.\n",
			 cmnd, cl_no);
		FABORT(1);
	}
  } while (size && cl_no != LAST_CLUSTER);

  if (cl_no != LAST_CLUSTER)
	fprintf (stderr, "%s: Too many clusters allocated for file.\n", cmnd);
  else if (size != 0)
	fprintf (stderr, "%s: Premature EOF: %ld bytes left.\n", cmnd,
		     entry->d_size);
}


/* Minimum of two long values
 */
long lmin (long a, long b)
{
	if (a < b) return a;
	else return b;
}


void make_file(DIRECTORY *dir_ptr, int entries, char *name)
{
  register DIRECTORY *entry = new_entry(dir_ptr, entries);
  register char *ptr;
  unsigned short cl_no = 0;
  int i, r;
  long size = 0L;
  unsigned short first_cluster, last_cluster;
  long chunk;
  char dir_bkp[DIR_SIZE];

  memcpy((char *)dir_bkp, (char *)entry, DIR_SIZE);
  entry = (DIRECTORY *)dir_bkp;
  memset (&entry->d_name[0], ' ', 11);    /* clear entry */
  for (i = 0, ptr = name; i < 8 && *ptr != '.' && *ptr; i++)
	entry->d_name[i] = *ptr++;
  while (*ptr != '.' && *ptr) ptr++;
  if (*ptr == '.') ptr++;
  for (i = 0; i < 3 && *ptr != '.' && *ptr; i++) entry->d_ext[i] = *ptr++;

  for (i = 0; i < 10; i++) entry->d_reserved[i] = '\0';
  entry->d_attribute = '\0';

  entry->d_cluster = 0;

  while (free_range (&first_cluster, &last_cluster)) {
	do {
		unsigned short	nr_clus;

		chunk = lmin ((long) (last_cluster - first_cluster + 1) *
			     		  cluster_size,
			      (long) MAX_CLUSTER_SIZE);
		r = fill(buffer, chunk);
		if (r == 0) goto done;
		nr_clus = (r + cluster_size - 1) / cluster_size;
		disk_io(WRITE, clus_add(first_cluster), buffer, r);

		for (i = 0; i < nr_clus; i ++) {
			if (entry->d_cluster == 0)
				cl_no = entry->d_cluster = first_cluster;
			else {
				link_fat(cl_no, first_cluster);
				cl_no = first_cluster;
			}
			first_cluster ++;
		}

		size += r;
	} while (first_cluster <= last_cluster);
  }
  fprintf (stderr, "%s: disk full. File truncated\n", cmnd);
done:
  if (entry->d_cluster != 0) link_fat(cl_no, LAST_CLUSTER);
  entry->d_size = size;
  fill_date(entry);
  disk_io(WRITE, mark, entry, DIR_SIZE);

  if (fat_dirty) flush_fat ();
}


#define SEC_MIN	60L
#define SEC_HOUR	(60L * SEC_MIN)
#define SEC_DAY	(24L * SEC_HOUR)
#define SEC_YEAR	(365L * SEC_DAY)
#define SEC_LYEAR	(366L * SEC_DAY)

unsigned short mon_len[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void fill_date(DIRECTORY *entry)
{
  time_t atime;
  unsigned long cur_time;
  unsigned short year = 0, month = 1, day, hour, minutes, seconds;
  int i;
  long tmp;

  time(&atime);
  cur_time = atime - DOS_TIME;
  for (;;) {
	tmp = (year % 4 == 0) ? SEC_LYEAR : SEC_YEAR;
	if (cur_time < tmp) break;
	cur_time -= tmp;
	year++;
  }
  day = (unsigned short) (cur_time / SEC_DAY);
  cur_time -= (long) day *SEC_DAY;

  hour = (unsigned short) (cur_time / SEC_HOUR);
  cur_time -= (long) hour *SEC_HOUR;

  minutes = (unsigned short) (cur_time / SEC_MIN);
  cur_time -= (long) minutes *SEC_MIN;

  seconds = (unsigned short) cur_time;

  mon_len[1] = (year % 4 == 0) ? 29 : 28;
  i = 0;
  while (day >= mon_len[i]) {
	month++;
	day -= mon_len[i++];
  }
  day++;

  entry->d_date = (year << 9) | (month << 5) | day;
  entry->d_time = (hour << 11) | (minutes << 5) | seconds;
}

char *make_name(DIRECTORY *dir_ptr, short dir_fl)
{
  static char name_buf[14];
  register char *ptr = name_buf;
  short i;

  for (i = 0; i < 8; i++) *ptr++ = dir_ptr->d_name[i];

  while (*--ptr == ' ');
  assert (ptr >= name_buf);

  ptr++;
  if (dir_ptr->d_ext[0] != ' ') {
	*ptr++ = '.';
	for (i = 0; i < 3; i++) *ptr++ = dir_ptr->d_ext[i];
	while (*--ptr == ' ');
	ptr++;
  }
  if (dir_fl) *ptr++ = '/';
  *ptr = '\0';

  return name_buf;
}


int fill(char *buff, size_t size)
{
  static BOOL nl_mark = FALSE;
  char *last = &buff[size];
  char *begin = buff;
  register int c;

  while (buff < last) {
  	if (nl_mark) {
  		*buff ++ = '\n';
  		nl_mark = FALSE;
  	} else {
  		c = fgetc(fdo);
		if (c == EOF) break;
		if (Aflag && c == '\n') {
			*buff ++ = '\r';
			nl_mark = TRUE;
		} else {
			*buff++ = c;
		}
	}
  }

  return (buff - begin);
}

#define HOUR	0xF800		/* Upper 5 bits */
#define MIN	0x07E0		/* Middle 6 bits */
#define YEAR	0xFE00		/* Upper 7 bits */
#define MONTH	0x01E0		/* Mid 4 bits */
#define DAY	0x01F		/* Lowest 5 bits */

char *month[] = {
	 "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

void xmodes(int mode)
{
  printf ( "\t%c%c%c%c%c", (mode & SUB_DIR) ? 'd' : '-',
	     (mode & 02) ? 'h' : '-', (mode & 04) ? 's' : '-',
	     (mode & 01) ? '-' : 'w', (mode & 0x20) ? 'a' : '-');
}

void show(DIRECTORY *dir_ptr, char *name)
{
  register unsigned short e_date = dir_ptr->d_date;
  register unsigned short e_time = dir_ptr->d_time;
  unsigned short next;
  char bname[20];
  short i = 0;

  while (*name && *name != '/') bname[i++] = *name++;
  bname[i] = '\0';
  if (!Lflag) {
	fprintf (fdo, "%s\n", bname);
	return;
  }
  xmodes( (int) dir_ptr->d_attribute);
  fprintf (fdo, "\t%s%s", bname, strlen(bname) < 8 ? "\t\t" : "\t");
  i = 1;
  if (is_dir(dir_ptr)) {
	next = dir_ptr->d_cluster;
	while ((next = next_cluster(next)) != LAST_CLUSTER) i++;
	fprintf (fdo, "%8ld", (long) i * (long) cluster_size);
  } else
	fprintf (fdo, "%8ld", dir_ptr->d_size);
  fprintf (fdo, " %02d:%02d %2d %s %d\n", ((e_time & HOUR) >> 11),
	     ((e_time & MIN) >> 5), (e_date & DAY),
   month[((e_date & MONTH) >> 5) - 1], ((e_date & YEAR) >> 9) + 1980);
}

void free_blocks(void)
{
  register unsigned short cl_no;
  long nr_free = 0;
  long nr_bad = 0;

  for (cl_no = 2; cl_no < total_clusters; cl_no++) {
	switch (next_cluster(cl_no)) {
	    case FREE:	nr_free++;	break;
	    case BAD16:	nr_bad++;	break;
	}
  }

  fprintf (fdo, "Free space: %ld bytes.\n", nr_free * (long) cluster_size);
  if (nr_bad != 0)
	fprintf (fdo, "Bad sectors: %ld bytes.\n", nr_bad * (long) cluster_size);
}


DIRECTORY *read_cluster(unsigned int cluster)
{
  disk_io(READ, clus_add(cluster), buffer, cluster_size);

  return (DIRECTORY *)buffer;
}

static unsigned short cl_index = 2;

/* find a range of consecutive free clusters. Return TRUE if found
 * and return the first and last cluster in the |*first| and |*last|.
 * If no free clusters are left, return FALSE.
 *
 * Warning: Assumes that all of the range is used before the next call
 *	to free_range or free_cluster.
 */
BOOL free_range (unsigned short *first, unsigned short *last)
{
  while (cl_index < total_clusters && next_cluster(cl_index) != FREE)
	cl_index++;
  if (cl_index >= total_clusters) return FALSE;
  *first = cl_index;
  while (cl_index < total_clusters && next_cluster(cl_index) == FREE)
	cl_index++;
  *last = cl_index - 1;
  return TRUE;
}


/* find a free cluster.
 * Return the number of the free cluster or a number > |total_clusters|
 * if none is found.
 * If |leave_fl| is TRUE, the the program will be terminated if 
 * no free cluster can be found
 *
 * Warning: Assumes that the cluster is used before the next call
 *	to free_range or free_cluster.
 */
unsigned short free_cluster(BOOL leave_fl)
{
  while (cl_index < total_clusters && next_cluster(cl_index) != FREE)
	cl_index++;

  if (leave_fl && cl_index >= total_clusters) {
	fprintf (stderr, "%s: Diskette full. File not added.\n", cmnd);
	exit(1);
  }
  return cl_index++;
}

/* read a portion of the fat containing |cl_no| into the cache
 */
void read_fat (unsigned int cl_no)
{

  if (!cooked_fat) {
  	/* Read the fat for the first time. We have to allocate all the
  	 * buffers
  	 */
  	if (fat_16) {
		/* FAT consists of little endian shorts. Easy to convert
		 */
		if ((cooked_fat = malloc (fat_size)) == NULL) {
			/* Oops, FAT doesn't fit into memory, just read
			 * a chunk
			 */
			if ((cooked_fat = malloc (COOKED_SIZE)) == NULL) {
				fprintf (stderr, "%s: not enough memory for FAT cache.\n",
					 cmnd);
				exit (1);
			}
			cache_size = COOKED_SIZE / 2;
		} else {
			cache_size = fat_size / 2;
		}
	} else {
		/* 12 bit FAT. Difficult encoding, but small. Keep
		 * both raw FAT and cooked version in memory if possible.
		 */
		cooked_fat = malloc (total_clusters * sizeof (short));
		raw_fat = malloc (fat_size);
		if (cooked_fat == NULL || raw_fat == NULL) {
			if (cooked_fat != NULL) free(cooked_fat);
			if (raw_fat != NULL) free(raw_fat);
			/* Oops, FAT doesn't fit into memory, just read
			 * a chunk
			 */
			cooked_fat = malloc (COOKED_SIZE);
			raw_fat = malloc (COOKED_SIZE/sizeof (short)/2*3);
			if (cooked_fat == NULL || raw_fat == NULL) {
				fprintf (stderr, "%s: not enough memory for FAT cache.\n",
					 cmnd);
				exit (1);
			}
			cache_size = COOKED_SIZE / sizeof(short);
			rawfat_size = COOKED_SIZE / sizeof(short)/2*3;
		} else {
			cache_size = total_clusters;
			rawfat_size = fat_size;
		}
	}
  }
  fat_low = cl_no; /* / cache_size * cache_size; ??? */

  /* for FAT 12, round fat_low to a multiple of 2, so, when reading FAT from 
   * disk, we start at this cluster or previous one */
  if (!fat_16) if ((fat_low & 1) != 0) fat_low--;

  fat_high = fat_low + cache_size - 1;

  if (!fat_16) {
  	unsigned short	*cp;
  	unsigned char	*rp;
  	unsigned short	i;

	disk_io (READ, fat_start + fat_low * 3 / 2, raw_fat, rawfat_size);
	for (rp = raw_fat, cp = cooked_fat, i = 0;
	     i < cache_size;
	     rp += 3, i += 2) {
	     	*cp = *rp + ((*(rp + 1) & 0x0f) << 8);
	     	if (*cp == BAD) *cp = BAD16;
	     	else if (*cp == LAST_CLUSTER12) *cp = LAST_CLUSTER;
	     	cp ++;
	     	*cp = ((*(rp + 1) & 0xf0) >> 4) + (*(rp + 2) << 4);
	     	if (*cp == BAD) *cp = BAD16;
	     	else if (*cp == LAST_CLUSTER12) *cp = LAST_CLUSTER;
	     	cp ++;
	}
  } else {
	assert (sizeof (short) == 2);
	assert (CHAR_BIT == 8);		/* just in case */

	disk_io (READ, fat_start + fat_low * 2, (void *)cooked_fat, cache_size * 2);
	if (big_endian) {
		unsigned short	*cp;
		unsigned char	*rp;
		unsigned short	i;

		for (i = 0, rp = (unsigned char *)cooked_fat /* sic */, cp = cooked_fat;
		     i < cache_size;
		     rp += 2, cp ++, i ++) {
		     	*cp = c2u2 (rp);
		}
	}
  }
}

/* flush the fat cache out to disk
 */
void flush_fat(void)
{
  if (fat_16) {
	if (big_endian) {
		unsigned short	*cp;
		unsigned char	*rp;
		unsigned short	i;

		for (i = 0, rp = (unsigned char *)cooked_fat /* sic */, cp = cooked_fat;
		     i < cache_size;
		     rp += 2, cp ++, i ++) {
		     	*rp = *cp;
		     	*(rp + 1) = *cp >> 8;
		}
	}
	disk_io (WRITE, fat_start + fat_low * 2, (void *)cooked_fat, cache_size * 2);
	disk_io (WRITE, fat_start+ fat_size + fat_low * 2, (void *)cooked_fat, cache_size * 2);
  } else {
  	unsigned short	*cp;
  	unsigned char	*rp;
  	unsigned short	i;

	for (rp = raw_fat, cp = cooked_fat, i = 0;
	     i < cache_size;
	     rp += 3, cp += 2, i += 2) {
	     	*rp = *cp;
	     	*(rp + 1) = ((*cp & 0xf00) >> 8) |
	     		    ((*(cp + 1) & 0x00f) << 4);
	     	*(rp + 2) = ((*(cp + 1) & 0xff0) >> 4);
	}
	disk_io (WRITE, fat_start + fat_low * 3 / 2, raw_fat, rawfat_size);
	disk_io (WRITE, fat_start+ fat_size + fat_low * 3 / 2, raw_fat, rawfat_size);
  }
}

/* make cl_2 the successor of cl_1
 */
void link_fat(unsigned int cl_1, unsigned int cl_2)
{
  if (cl_1 < fat_low || cl_1 > fat_high) {
  	if (fat_dirty) flush_fat ();
  	read_fat (cl_1);
  }
  cooked_fat [cl_1 - fat_low] = cl_2;
  fat_dirty = TRUE;
}


unsigned short next_cluster(unsigned int cl_no)
{
  if (cl_no < fat_low || cl_no > fat_high) {
  	if (fat_dirty) flush_fat ();
  	read_fat (cl_no);
  }
  return cooked_fat [cl_no - fat_low];
}

/* free cluster cl_no in FAT and return its sucessor */
unsigned short clear_cluster(unsigned int cl_no)
{
  unsigned short old;
  
  if (cl_no < fat_low || cl_no > fat_high) {
  	if (fat_dirty) flush_fat ();
  	read_fat (cl_no);
  }
  old = cooked_fat[cl_no - fat_low];
  cooked_fat[cl_no - fat_low] = 0;
  fat_dirty = TRUE;
  return old;
}

char *slash(char *str)
{
  register char *result = str;

  while (*str)
	if (*str++ == '/') result = str;

  return result;
}

void add_path(char *file, BOOL slash_fl)
{
  register char *ptr = path;

  while (*ptr) ptr++;

  if (file == NIL_PTR) {
	if (ptr != path) ptr--;
	if (ptr != path) do {
			ptr--;
		} while (*ptr != '/' && ptr != path);
	if (ptr != path && !slash_fl) *ptr++ = '/';
	*ptr = '\0';
  } else
	strcpy (ptr, file);
}


void disk_io(BOOL op, unsigned long seek, void *address, unsigned bytes)
{
  unsigned int r;

  if (lseek(disk, seek, SEEK_SET) < 0L) {
	fflush (stdout);
	fprintf (stderr, "%s: Bad lseek: %s\n", cmnd, strerror (errno));
	exit(1);
  }
  if (op == READ)
	r = read(disk, (char *) address, bytes);
  else {
	r = write(disk, (char *) address, bytes);
  };

  if (r != bytes) {
  	fprintf (stderr, "%s: %s error: %s\n", op == READ ? "read" : "write", cmnd, strerror (errno));
  	exit (1);
  }
}

char dosread_c_rcs_id [] = 
	"$Id: dosread.c,v 2.0 2001/03/11 14:30:43 adrcunha Rel $";

/* $Log: dosread.c,v $
 * Revision 2.0  2001/03/11  14:30:43  adrcunha@dcc.unicamp.br
 * Fixed some bugs, added FAT12 cache, decreased memory usage, changed
 * drive number to letter on dir header, introduced some sanity check for
 * memory, added compilation directive CACHE_ROOT (with this, root dir is
 * loaded on memory at the beginning; without this, root dir is allways
 * read from disk when needed - it's slow, but saves memory).
 *
 * Revision 1.9  1999/11/28  14:32:37  adrcunha@dcc.unicamp.br
 * Dosread and doswrite can read/write to/from a file, not only stdin/stdout.
 * Added one more personality to the schizophrenic dosread: dosdel
 * Drives can be referenced as A:, B:, etc for /dev/fd0, /dev/fd1, etc
 * Dosfiles can be referenced after drive if drive is 'X:' (e.g., A:FOO.BAR)
 *
 * Revision 1.8  1994/05/14  21:53:08  hjp
 * filenames with more than 3 characters and extension work again.
 * removed debugging stuff and b_copy.
 *
 * Revision 1.7  1994/04/09  03:09:01  hjp
 * (posted to comp.os.minix)
 * merged branch 1.5.387 with stem.
 * changed treatment of drive parameter
 *
 * Revision 1.5.387.9  1994/04/09  02:07:51  hjp
 * Disk full no longer produces lost clusters but a truncated file.
 * Truncated file names to 8+3 before comparisons to avoid duplicate
 * files and filenames containing dots in the extension.
 * Replaced sbrk and brk by malloc and free (mixing brk and malloc causes
 * heap corruption which sometimes lead to core dumps. It may also have
 * been the cause of data corruption Kees reported).
 * Made global variables static and removed some unused ones.
 * Error messages now contain program name.
 *
 * Revision 1.5.387.8  1993/11/13  00:38:45  hjp
 * Posted to comp.os.minix and included in Minix-386vm 1.6.25.1.
 * Speed optimizations for 1.44 MB disks.
 * Replaced lowlevel I/O by stdio.
 * Simplified -a: Now only removes resp. adds CRs
 * Cleaned up.
 * 
 * Revision 1.5.387.1  1993/01/15  19:32:29  ast
 * Released with 1.6.24b
 */
