/*	cpmio.h	1.4	83/05/13	*/
#define INTSIZE 16		/* number of bits per integer on this particular machine */

extern int fid;

/* FIXME force packed */
extern struct directory {
	char status;		/* status of this entry; equals 0xe5 if */
	/* free to use, otherwise contains the */
	/* user number (owner) (0 - 15) */
	char name[8];		/* File name, padded with blanks */
	char ext[3];		/* file name extension, padded with blanks */
	char extno;		/* extent number */
	char notused[2];	/* unused */
	char blkcnt;		/* record count, number of 128 byte records */
	/* in this extent */
	char pointers[16];	/* pointers to the individual blocks */
} *dirbuf;

#define CPMSECSIZ 128		/* number of bytes per sector in CP/M terms */

/* FIXME: uninline for size */
#define blockno(i) (use16bitptrs?					\
		    (0xff & (int)fptr->c_dirp->pointers[2*(i)]) +	\
		    ((0xff & (int)fptr->c_dirp->pointers[2*(i)+1]) << 8): \
		    0xff & (int)fptr->c_dirp->pointers[i])

extern int dflag, cflag, iflag, tflag;
extern int blksiz;
extern int tracks;
extern int maxdir;

extern int seclth;
extern int sectrk;
extern int skew;
extern int restrk;		/* reserved tracks (for system) */

extern int *bitmap, *skewtab;
extern unsigned int bm_size;
extern int use16bitptrs;
/*	cpmfio.h	1.5	83/05/13	*/

#define C_NFILE		5	/* max number of concurrently open cp/m files */

typedef struct c_iobuf {
	int c_cnt;		/* bytes left in buffer */
	int c_blk;		/* block number within the current extent */
	/* (starting at 0) */
	int c_seccnt;		/* number of physical sectors left in */
	/* the current extent */
	int c_ext;		/* current extent's directory index */
	int c_extno;		/* extent number within current file */
	char *c_buf;		/* next character position */
	char *c_base;		/* location of buffer */
	short c_flag;		/* access mode (READ or WRITE) */
	struct directory *c_dirp;	/* pointer to the current */
	/* extent's directory entry */
} C_FILE;
extern C_FILE c_iob[C_NFILE];

#define c_getc(p)	(--(p)->c_cnt>=0 ? *(p)->c_buf++&0377 : c_fillbuf(p))
#define c_putc(x,p)	(--(p)->c_cnt>=0 ? ((int)(*(p)->c_buf++=(unsigned)(x))) : c_flsbuf((unsigned)(x), p))

#define READ	0x01
#define WRITE	0x02
#define RW	0x03
#define MODFLG	0x08
#define BINARY	0x10

extern unsigned int alloc(void);
extern void dbmap(const char *str);
extern unsigned int blks_used(void);
extern void build_bmap(void);

extern int getblock(unsigned int blockno, char *buffer, int nsect);
extern int putblock(unsigned int blockno, char *buffer, int nsect);

extern int c_close(C_FILE * fptr);
extern C_FILE *c_creat(const char *name, const char *ext, int flag);
extern int c_fillbuf(C_FILE * fptr);
extern int c_flush(C_FILE * fptr);
extern int c_flsbuf(int c, C_FILE * fptr);
extern int c_write(C_FILE * fptr, char *buf, int cnt);
extern void cmdinp(char *cmd, int len);
extern int chkcmd(const char *cmd);
extern void help(void);
extern int namesep(const char *fname, char *name, char *ext);
extern void clean(char *str, int len);
extern C_FILE *c_open(const char *name, const char *ext, int mode);
extern void fnfound(const char *name, const char *ext);
extern void copyc(char *cmdline, int bin);
extern void copy(char *cpmfile, char *unixfile, int bin);
extern void copytext(C_FILE * cid, FILE * ufid);
extern void copybin(C_FILE * cid, FILE * ufid);
extern int number(int big);
extern void usage(void);
extern void delete(const char *cmdline);
extern void dispdir(void);
extern void getdir(void);
extern void savedir(void);
extern int searchdir(const char *name, const char *ext);
extern int creext(int curext);
extern int getnext(C_FILE * cur);
extern int ffc(int start, int len, long field);
extern void gen_sktab(void);
extern void dump(const char *cmdline);
extern void hexdump(C_FILE * fp);
extern void printline(FILE * piped, int *cbuf, int nc);
extern void interact(void);
extern void intrpt(int sig);
extern int putpsect(unsigned int tr, unsigned int sect, const char *buf);
extern int getpsect(unsigned int tr, unsigned int sect, char *buf);
extern int initcpm(const char *name);
extern void pip(char *cmdline, int bin);
extern void pipc(const char *unixfile, const char *cpmfile, int bin);
extern void piptext(C_FILE * cid, FILE * ufid);
extern void pipbin(C_FILE * cid, FILE * ufid);
extern int Rename(char *cmdline);
