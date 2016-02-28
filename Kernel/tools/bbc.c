/*
 *	Manipulate BBC micro disc images include Watford DDFS and
 *	to an extent HDFS image types. We don't handle Z80 images because
 *	the Z80 images are totally different. The Z80 disks have
 *	BIOS on sectors 0-7, CCP & BDOS on sectors 8-29 (CCP first)
 *	The disk itself is in CP/M format. The disk must be in FM mode 80 track
 *	and tracks 0-2 are used by the system image.
 *
 *	The Z80 images load the BIOS to E9F0, CCP and BDOS to D400
 *	The BIOS EB00 to EBF0 are copied to EAF0-EBE0 and EBF0-F1F0 to
 *	EBE0 to F1E0
 *
 *	To boot the byte at D400 must contain C3 and if so then a jump is
 *	made to EA00
 *
 *	The CP/M image has tracks 0-79 mapped to side 0, but 80-159 are mapped
 *	to side 1 in *reverse* order. 4K directory, 128 entries per disc, 512
 *	byte blocks, 256 byte physical sector size. Physical skew is
 *	00 11 44 55 88 99 22 33 66 77 (for the pairs)
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

/*
 *	Disk format (256 byte logical sectors)
 *
 *	Sector 0 is a header then 31 directory entries
 *	Sector 1 is a header then 31 file info headers
 *	Sector 2+ are data
 *
 *	Entry 0 is the header in each sector
 *	Entry 1 to 31 are the file entries
 *
 *	Each entry in sector 0 is
 *	char name[7]	space padded
 *	char prefix     directory a-z etc
 *
 *	The top bit of each byte is used for
 *	0	HDFS:sector bit 10
 *	1	HDFS length bit 18
 *	2	-
 *	3	HDFS file/dir
 *	4	HDFS not readable
 *	5	HDFS not writeable (WDFS length bit 18)
 *	6	HDFS not executable (WDFS sector bit 10)
 *	7	locked
 *
 *
 *	Sector 1 holds file info headers
 *
 *	0-1	load address bits 0-15
 *	2-3	execution address bits 0-15
 *	4-5	length bits 0-15
 *	6
 *		bit 0-1: sector bit 8/9
 *		bit 2-3: load bit 16/17
 *		bit 4-5: length bit 16/17
 *		bit 6-7: execution bit 16/17
 *	7	sector start b0-7
 *
 *	Sector 0 starts with an 8 byte block holding the
 *	title (space padded). HDFS uses bit 7 of byte 0 for
 *	bit 10 of total sectors otherwise top bits are 0
 *
 *	Sector 1 starts with an 8 byte block holding
 *	100-103 Last four bytes of title (space padded)
 *	104 disk cycle number (key number for HDFS)
 *	105 number of catalogue entries * 8
 *	106 bit 0/1 total sectors bit8-9
 *	    bit 2 0 (WDFS total sectors bit 10) (HDFS sides - 1)
 *	    bit 3 0 (HDFS 1 - HDFS detect)
 *	    bit 4-5 !boot option
 *	    bit 6-7 0
 *	107 total logical sectors b0-b7
 *
 *	Watford DFS may have a second catalogue in sector 2-3 if
 *	sector2 starts AA * 8
 *
 *	If so then 8-FF are 31 more directory entries
 *	108-1FF are 31 more file info entries
 *
 *	100-103 are 0
 *	104 is copy of disk cycle number
 *	105 number of catalogue entries *8
 *	106-107 copy of info in sector 1
 *
 */

struct bbc_file {
	char name[8];		/* 7 bytes */
	char prefix;
	uint32_t load;
	uint32_t exec;
	uint32_t length;
	uint32_t sector;
	uint32_t flags;
#define HDFS_DIR	1
#define NO_READ		2
#define NO_WRITE	4
#define	NO_EXECUTE	8
#define LOCKED		16
#define DIRTY		128	/* Entry needs writing back */
};

struct freelist {
	uint32_t start;
	uint32_t end;
};

struct bbc_disk {
	uint8_t type;
#define TYPE_INVALID	0
#define TYPE_DFS	1
#define TYPE_WDFS	2
#define TYPE_WDFS_2	3
#define TYPE_HDFS	4
	uint8_t num_dirents;
	uint8_t sides;
	uint8_t cycle;
	uint8_t plingboot;
	uint8_t flags;
#define DISK_CORRUPT_FREE	1
#define DATA_DIRTY		64	/* Data area needs writing back */
#define DISK_DIRTY		128	/* Hader needs writing back */
	uint32_t sectors;
	char label[13];		/* 12 byte label */
	struct bbc_file files[62];	/* Max allowed */
	struct freelist free[63];	/* one hole after each entry and one at the
					   front */
	int nextfree;
};

static const char *typestr[5] = {
	"Invalid",
	"DFS",
	"WDFS",
	"WDFS (2 directories)",
	"HDFS"
};

struct bbc_disk disk;
uint8_t *diskdata;
long disksize;
char *diskname;

int within(struct freelist *f, struct bbc_file *fn)
{
	uint32_t size = (fn->length + 255) / 256;
	if (f->start <= fn->sector && f->end >= fn->sector + size - 1)
		return 1;
	return 0;
}

void split(struct freelist *f, struct bbc_file *fn)
{
	uint32_t size = (fn->length + 255) / 256;
	struct freelist *n;

	if (f->start == fn->sector) {
		/* shrink current - may shrink to 0 */
		f->start += size;
		return;
	}
	if (fn->sector + size - 1 == f->end) {
		/* shrink current tail - may shrink to 0 */
		f->end = fn->sector - 1;
		return;
	}

	/* Grab a new entry for the tail space */
	n = disk.free + disk.nextfree++;
	n->start = fn->sector + size - 1;
	n->end = f->end;
	/* Space at front */
	f->end = fn->sector - 1;
}

void insert_used(struct bbc_file *fn)
{
	/* This must be within a current free entry or the disk is corrupt */
	struct freelist *f = disk.free;
	int n = 0;
	while (n < disk.nextfree) {
		if (within(f, fn)) {
			split(f, fn);
			return;
		}
		f++;
		n++;
	}
	/* Out of range */
	fprintf(stderr, "Corrupt sector range map when analyzing %c.%s\n",
		fn->prefix, fn->name);
	disk.flags |= DISK_CORRUPT_FREE;
}

/* First fit for now - best fit might be better FIXME ? */
uint32_t find_space(struct bbc_file * fn)
{
	uint32_t size = (fn->length + 255) / 256;
	struct freelist *f = disk.free;
	int n = 0;

	while (n < disk.nextfree) {
		if (f->end >= f->start && f->end - f->start >= size - 1) {
			return f->start;
		}
		f++;
		n++;
	}
	return 0;
}

void init_free_list(void)
{
	disk.nextfree = 1;
	if (disk.type == TYPE_WDFS_2)
		disk.free[0].start = 4;
	else
		disk.free[0].start = 2;
	disk.free[0].end = disk.sectors - 1;
}

void rebuild_free_list(void)
{
	struct bbc_file *f = disk.files;
	int i;
	init_free_list();
	for (i = 0; i < disk.num_dirents; i++) {
		if (*f->name && f->sector && f->length)
			insert_used(f);
		f++;
	}
}

void strim(char *c)
{
	while (*c)
		*c++ &= 0x7F;
}

void set_disk_type(void)
{
	disk.sides = 1;
	disk.num_dirents = 31;
	switch ((diskdata[0x106] >> 2) & 3) {
	case 0:
		/* Could also be WDFS but small so same as DFS */
		disk.type = TYPE_DFS;
		break;
	case 1:
		disk.type = TYPE_WDFS;
		break;
	case 3:
		disk.sides = 2;
		/* Fall through */
	case 2:
		disk.type = TYPE_HDFS;
	}
	if (memcmp(diskdata + 0x200, "\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA", 8)
	    == 0 && memcmp(diskdata + 0x300, "\0\0\0\0", 4) == 0) {
		disk.type = TYPE_WDFS_2;
		disk.num_dirents = 62;
	}
}

void load_headers(void)
{
	uint8_t *p = diskdata;
	memcpy(disk.label, p, 8);
	memcpy(disk.label + 8, p + 0x100, 4);
	disk.label[12] = 0;
	strim(disk.label);
	disk.cycle = p[0x104];
	/* FIXME: 0x105 catalog entries * 8 ??? */
	disk.sectors = p[0x107];
	disk.sectors |= (p[0x106] & 3) << 8;
	disk.plingboot = (p[0x106] & 0x30) >> 4;
	if ((p[0] & 0x80) && disk.type == TYPE_HDFS)
		disk.sectors += 0x400;
	init_free_list();
}

void recode_headers(void)
{
	uint8_t *p = diskdata;
	memcpy(p, disk.label, 8);
	memcpy(p + 0x100, disk.label + 8, 4);
	p[0x0104] = disk.cycle;
	if (disk.type == TYPE_WDFS_2)
		p[0x204] = disk.cycle;
	p[0x106] &= ~0x30;
	p[0x106] |= (disk.plingboot) << 4;
	if (disk.type == TYPE_HDFS && disk.sectors >= 0x400)
		p[0x0] |= 0x80;
}

void load_directory(void)
{
	int i;
	for (i = 0; i < disk.num_dirents; i++) {
		uint8_t *p = diskdata + 0x08 + 0x08 * i;
		struct bbc_file *f = disk.files + i;
		uint8_t b;

		/* Skip second headers */
		if (i > 31)
			p = diskdata += 0x08;

		f->load = p[0x100] + (p[0x101] << 8);
		f->exec = p[0x102] + (p[0x103] << 8);
		f->length = p[0x104] + (p[0x105] << 8);
		f->sector = p[0x107];
		b = p[0x106];
		f->sector |= (b & 3) << 8;
		f->load |= (b & 0xC) << 14;
		f->length |= (b & 0x30) << 12;
		f->exec |= (b & 0xC0) << 10;

		if (f->load & 0x20000)
			f->load |= 0xFFFC0000;
		if (f->exec & 0x20000)
			f->exec |= 0xFFFC0000;

		memcpy(f->name, p, 7);
		f->name[8] = 0;
		f->prefix = p[7];
		if (p[7] & 0x80)
			f->flags |= LOCKED;
		if (p[6] & 0x80) {
			if (disk.type == TYPE_WDFS
			    || disk.type == TYPE_WDFS_2)
				f->sector += 0x400;
			else if (disk.type == TYPE_HDFS)
				f->flags |= NO_EXECUTE;
		}
		if (p[5] & 0x80) {
			if (disk.type == TYPE_WDFS
			    || disk.type == TYPE_WDFS_2)
				f->length += 0x40000;
			else if (disk.type == TYPE_HDFS)
				f->flags |= NO_WRITE;
		}
		if (p[4] & 0x80) {
			if (disk.type == TYPE_HDFS)
				f->flags |= NO_READ;
		}
		if (p[3] & 0x80) {
			if (disk.type == TYPE_HDFS)
				f->flags |= HDFS_DIR;
		}
		if (p[1] & 0x80) {
			if (disk.type == TYPE_HDFS)
				f->length += 0x40000;
		}
		if (p[0] & 0x80) {
			if (disk.type == TYPE_HDFS)
				f->sector += 0x400;
		}
		strim(f->name);
	}
	rebuild_free_list();
}

void recode_dirent(int i, struct bbc_file *f)
{
	uint8_t *p = diskdata + 0x08 + 0x08 * i;
	if (i > 31)
		p += 0x08;	/* Skip second header */

	memcpy(p, f->name, 7);
	p[7] = f->prefix;

	if (f->flags & LOCKED)
		p[7] |= 0x80;

	if (f->flags & NO_EXECUTE)
		p[6] |= 0x80;
	else if (disk.type != TYPE_HDFS && f->sector >= 0x0400)
		p[6] |= 0x80;

	if (f->flags & NO_WRITE)
		p[5] |= 0x80;
	else if (disk.type != TYPE_HDFS && f->length >= 0x40000)
		p[5] |= 0x80;

	if (f->flags & NO_READ)
		p[4] |= 0x80;

	if (f->flags & HDFS_DIR)
		p[3] |= 0x80;

	if (disk.type == TYPE_HDFS && f->sector >= 0x400)
		p[0] |= 0x80;

	p[0x100] = f->load & 0xFF;
	p[0x101] = (f->load >> 8) & 0xFF;
	p[0x102] = f->exec & 0xFF;
	p[0x103] = (f->exec >> 8) & 0xFF;
	p[0x104] = f->length & 0xFF;
	p[0x105] = (f->length >> 8) & 0xFF;
	p[0x106] = (f->sector >> 8) & 0x03;
	p[0x107] = f->sector & 0xFF;
	p[0x106] |= (f->load >> 14) & 0x0C;
	p[0x106] |= (f->length >> 12) & 0x30;
	p[0x106] |= (f->exec >> 10) & 0xC0;
}

void load_disk(void)
{
	set_disk_type();
	load_headers();
	load_directory();
}

struct bbc_file *lookup(const char *p)
{
	char buf[7];
	char prefix = 0;
	int i;

	if (!*p)
		return NULL;
	memset(buf, ' ', 7);
	if (p[1] == '.') {
		prefix = *p;
		p += 2;
	}
	if (!*p)
		return NULL;
	if (strlen(p) > 7)
		return NULL;
	memcpy(buf, p, strlen(p));

	for (i = 0; i < disk.num_dirents; i++) {
		struct bbc_file *f = disk.files + i;
		if (memcmp(buf, f->name, 7) == 0 &&
		    (prefix == 0 || prefix == f->prefix))
			return f;
	}
	return NULL;
}

struct bbc_file *lookup_free(const char *p)
{
	char buf[7];
	char prefix = '$';
	int i;

	if (!*p)
		return NULL;
	memset(buf, ' ', 7);
	if (p[1] == '.') {
		prefix = *p;
		p += 2;
	}
	if (!*p)
		return NULL;
	if (strlen(p) > 7)
		return NULL;
	memcpy(buf, p, strlen(p));
	strim(buf);		/* No top bits set */

	for (i = 0; i < disk.num_dirents; i++) {
		struct bbc_file *f = disk.files + i;
		if (f->sector == 0 || *f->name == 0) {
			memcpy(f->name, buf, 7);
			f->prefix = prefix;
			f->flags |= DIRTY;
			return f;
		}
	}
	return NULL;
}

void directory(int argc, char *argv[])
{
	int i;
	printf("Disk: %s (%d sectors, %d sides)\n",
	       disk.label, disk.sectors, disk.sides);
	printf("Type %s (%d directories, !boot %d, cycles %d)\n",
	       typestr[disk.type], disk.num_dirents, disk.plingboot,
	       disk.cycle);
	for (i = 0; i < disk.num_dirents; i++) {
		struct bbc_file *f = disk.files + i;
		if (f->sector && memcmp(f->name, "       ", 7)) {
			printf
			    ("%c.%s  %c%c%c%c%c    %-6d@%-6d        L%08X E%08X\n",
			     f->prefix, f->name,
			     f->flags & HDFS_DIR ? 'D' : ' ',
			     f->flags & NO_READ ? ' ' : 'r',
			     f->flags & NO_WRITE ? ' ' : 'w',
			     f->flags & NO_EXECUTE ? ' ' : 'x',
			     f->flags & LOCKED ? 'L' : ' ', f->length,
			     f->sector, f->load, f->exec);
		}
	}
}

void export(int argc, char *argv[])
{
	FILE *fp;
	uint8_t *p;
	struct bbc_file *f;

	if (argc != 5) {
		fprintf(stderr, "%s: export diskfile bbcname unixname.\n",
			argv[0]);
		exit(1);
	}
	f = lookup(argv[3]);
	if (f == NULL) {
		fprintf(stderr, "%s: '%s' not found.\n", argv[0], argv[3]);
		exit(1);
	}
	p = diskdata + 256 * f->sector;
	printf("Offset %p v %p\n", p, diskdata);
	fp = fopen(argv[4], "w");
	if (fp == NULL) {
		perror(argv[4]);
		exit(1);
	}
	if (fwrite(p, f->length, 1, fp) != 1) {
		perror("write");
		exit(1);
	}
	fclose(fp);
}

/* FIXME: we need to open this in the same directory as the
   original image so that rename is reliable */
FILE *fopen_tmp(void)
{
	FILE *fp = fopen(".tmpdisk", "w");
	if (fp == NULL) {
		perror(".tmpdisk");
		exit(1);
	}
	return fp;
}

void free_tmp(void)
{
	if (unlink(".tmpdisk"))
		perror("unlink");
}

void rename_tmp(void)
{
	if (rename(".tmpdisk", diskname)) {
		perror("rename");
		exit(1);
	}
}

void disk_writeback(void)
{
	FILE *fp;
	struct bbc_file *f;
	int hsize;
	int i;

	if (disk.flags & DISK_CORRUPT_FREE) {
		fprintf(stderr, "Corrupt disk: not rewriting.\n");
		exit(1);
	}

	fp = fopen_tmp();

	if (disk.type == TYPE_WDFS_2)
		hsize = 4 * 256;
	else
		hsize = 2 * 256;

	/* Always do this as we need the data in the new media */
	if (1 /*disk.flags & DATA_DIRTY */ ) {
		if (fseek(fp, hsize, 0)) {
			perror("fseek");
			free_tmp();
			exit(1);
		}
		if (fwrite
		    (diskdata + hsize, disk.sectors * 256 - hsize, 1,
		     fp) != 1) {
			perror("fwrite");
			free_tmp();
			exit(1);
		}
		disk.flags &= ~DATA_DIRTY;
		disk.flags |= DISK_DIRTY;
	}
	f = disk.files;
	for (i = 0; i < disk.num_dirents; i++) {
		if (f->flags & DIRTY) {
			disk.flags |= DISK_DIRTY;
			recode_dirent(i, f);
		}
		f++;
	}
	if (disk.flags & DISK_DIRTY) {
		disk.cycle++;
		recode_headers();
	}
	if (fseek(fp, 0L, 0)) {
		perror("fseek");
		free_tmp();
		exit(1);
	}
	if (fwrite(diskdata, hsize, 1, fp) != 1 || fclose(fp) == -1) {
		perror("fwrite");
		free_tmp();
		exit(1);
	}
	rename_tmp();
	disk.flags &= ~DISK_DIRTY;
}

void compact(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "%s compact diskfile.\n", argv[0]);
		exit(1);
	}
	fprintf(stderr, "%s: compact is not yet supported.\n", argv[0]);
	exit(1);
}

void setproperties(int argc, char *argv[])
{
	struct bbc_file *f;

	if (argc != 6) {
		fprintf(stderr,
			"%s: setprop diskfile bbcname load exec.\n",
			argv[0]);
		exit(1);
	}
	f = lookup(argv[3]);
	if (f == NULL) {
		fprintf(stderr, "%s: '%s' not found.\n", argv[0], argv[3]);
		exit(1);
	}
	f->load = atol(argv[4]);
	f->exec = atol(argv[5]);
	f->flags |= DIRTY;
	disk_writeback();
}

void deletefile(int argc, char *argv[])
{
	struct bbc_file *f;

	if (argc != 4) {
		fprintf(stderr, "%s: rm diskfile bbcname.\n", argv[0]);
		exit(1);
	}
	f = lookup(argv[3]);
	if (f == NULL) {
		fprintf(stderr, "%s: '%s' not found.\n", argv[0], argv[3]);
		exit(1);
	}
	memset(f->name, 0, 7);
	f->prefix = 0;
	f->sector = 0;
	f->flags |= DIRTY;
	disk_writeback();
	rebuild_free_list();
}

void import(int argc, char *argv[])
{
	struct bbc_file *f;
	FILE *fp;
	struct stat st;
	uint32_t len;

	if (argc != 5) {
		fprintf(stderr, "%s: import diskfile unixname bbcname.\n",
			argv[0]);
		exit(1);
	}
	f = lookup(argv[4]);
	if (f) {
		fprintf(stderr, "%s: '%s' already exists.\n", argv[0],
			argv[4]);
		exit(1);
	}
	f = lookup_free(argv[4]);
	if (f == NULL) {
		fprintf(stderr, "%s: unable to create file '%s'.\n",
			argv[0], argv[4]);
		exit(1);
	}

	f->load = 0;
	f->exec = 0;
	f->flags |= DIRTY;

	fp = fopen(argv[3], "r");
	if (fp == NULL) {
		perror(argv[3]);
		exit(1);
	}

	if (fstat(fileno(fp), &st) == -1) {
		perror("fstat");
		exit(1);
	}
	len = st.st_size;
	f->length = len;

	f->sector = find_space(f);
	if (f->sector == 0) {
		fprintf(stderr, "Disc full.\n");
		exit(1);
	}

	if (fread(diskdata + 256 * f->sector, len, 1, fp) != 1) {
		perror("read");
		exit(1);
	}
	fclose(fp);

	insert_used(f);
	disk.flags |= DATA_DIRTY;
	disk_writeback();
}

void setplingboot(int argc, char *argv[])
{
	if (argc != 4) {
		fprintf(stderr, "%s: boot diskfile mode .\n", argv[0]);
		exit(1);
	}
	disk.flags |= DISK_DIRTY;
	disk.plingboot = atoi(argv[3]) & 3;
	disk_writeback();
}

int main(int argc, char *argv[])
{
	FILE *f;
	struct stat st;
	if (argc < 3) {
		fprintf(stderr, "%s: op diskfile ...\n", argv[0]);
		exit(1);
	}
	f = fopen(argv[2], "r");
	if (f == NULL) {
		perror(argv[2]);
		exit(1);
	}
	if (fstat(fileno(f), &st) == -1) {
		perror("fstat");
		exit(1);
	}
	/* FIXME: non regular files */
	diskdata = malloc(st.st_size);
	if (diskdata == NULL) {
		fprintf(stderr, "%s: out of memory.\n", argv[0]);
		exit(1);
	}
	disksize = st.st_size;
	diskname = argv[2];
	if (fread(diskdata, 1, disksize, f) != disksize) {
		perror("read");
		exit(1);
	}
	fclose(f);
	load_disk();

	/* Some people pack incomplete SSD files and assume the rest is zero */
	if (disksize < disk.sectors * 256) {
		diskdata = realloc(diskdata, disk.sectors * 256);
		memset(diskdata + disksize, 0,
		       disk.sectors * 256 - disksize);
		disksize = disk.sectors * 256;
		printf("Expanded truncated SSD to %ld bytes\n", disksize);
	}
	if (strcmp(argv[1], "ls") == 0)
		directory(argc, argv);
	else if (strcmp(argv[1], "export") == 0)
		export(argc, argv);
	else if (strcmp(argv[1], "set") == 0)
		setproperties(argc, argv);
	else if (strcmp(argv[1], "rm") == 0)
		deletefile(argc, argv);
	else if (strcmp(argv[1], "import") == 0)
		import(argc, argv);
	else if (strcmp(argv[1], "boot") == 0)
		setplingboot(argc, argv);
	else
		fprintf(stderr, "%s: unknown operation '%s'.\n", argv[0],
			argv[1]);
	return 0;
}
