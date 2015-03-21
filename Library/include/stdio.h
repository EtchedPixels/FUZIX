#ifndef __STDIO_H
#define __STDIO_H
#ifndef __TYPES_H
#include <types.h>
#endif
#include <stdarg.h>

#define BUFSIZE 	512	/* uzix buffer/block size */
#define BUFSIZELOG	9	/* uzix buffer/block size log2 */
#define BUFSIZ		256
#define PATHLEN 	512

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#define _IOFBF		0x00	/* full buffering */
#define _IOLBF		0x01	/* line buffering */
#define _IONBF		0x02	/* no buffering */
#define __MODE_BUF	0x03	/* Modal buffering dependent on isatty */

#define __MODE_FREEBUF	0x04	/* Buffer allocated with malloc, can free */
#define __MODE_FREEFIL	0x08	/* FILE allocated with malloc, can free */

#define __MODE_READ	0x10	/* Opened in read only */
#define __MODE_WRITE	0x20	/* Opened in write only */
#define __MODE_RDWR	0x30	/* Opened in read/write */

#define __MODE_READING	0x40	/* Buffer has pending read data */
#define __MODE_WRITING	0x80	/* Buffer has pending write data */

#define __MODE_EOF	0x100	/* EOF status */
#define __MODE_ERR	0x200	/* Error status */
#define __MODE_UNGOT	0x400	/* Buffer has been polluted by ungetc */

#define __MODE_IOTRAN	0

typedef off_t fpos_t;
/* when you add or change fields here, be sure to change the initialization
 * in stdio_init and fopen */
struct __stdio_file {
	uchar	*bufpos;	/* the next byte to write to or read from */
	uchar	*bufread;	/* the end of data returned by last read() */
	uchar	*bufwrite;	/* highest address writable by macro */
	uchar	*bufstart;	/* the start of the buffer */
	uchar	*bufend;	/* the end of the buffer; ie the byte after
				   the last malloc()ed byte */
	int	fd;		/* the file descriptor associated with the stream */
	int	mode;
	char	unbuf[8];	/* The buffer for 'unbuffered' streams */
	struct __stdio_file * next;
};

#define EOF	(-1)
#ifndef NULL
#define NULL	(0)
#endif

typedef struct __stdio_file FILE;

extern FILE stdin[1];
extern FILE stdout[1];
extern FILE stderr[1];

#define putc(c, stream) \
	(((stream)->bufpos >= (stream)->bufwrite) ? \
		fputc((c), (stream)) : \
		(uchar) (*(stream)->bufpos++ = (c)))
#define getc(stream)	\
	(((stream)->bufpos >= (stream)->bufread) ? \
		fgetc(stream) : \
		(*(stream)->bufpos++))

#define putchar(c)	putc((c), stdout)
#define getchar()	getc(stdin)

extern char *gets __P((char *));
extern char *gets_s __P((char *, size_t));

extern int _putchar __P((int));
extern int _getchar __P((void));

#define ferror(fp)	(((fp)->mode&__MODE_ERR) != 0)
#define feof(fp)	(((fp)->mode&__MODE_EOF) != 0)
#define clearerr(fp)	((fp)->mode &= ~(__MODE_EOF|__MODE_ERR))
#define fileno(fp)	((fp)->fd)

/* These two call malloc */
extern int setvbuf __P((FILE*, char*, int, size_t));
#define setlinebuf(__fp)	setvbuf((__fp), (char*)0, _IOLBF, 0)

/* These don't */
extern void setbuffer __P((FILE*, char*, size_t));
#define setbuf(__fp, __buf)	setbuffer((__fp), (__buf), BUFSIZ)

extern int fgetc __P((FILE*));
extern int fputc __P((int, FILE*));
extern int ungetc __P((int, FILE*));

extern int fclose __P((FILE*));
extern int fflush __P((FILE*));
#define stdio_pending(fp) ((fp)->bufread > (fp)->bufpos)
extern char *fgets __P((char*, size_t, FILE*));
extern FILE *__fopen __P((const char*, int, FILE*, const char*));

#define fopen(__file, __mode)	      __fopen((__file), -1, (FILE*)0, (__mode))
#define freopen(__file, __mode, __fp) __fopen((__file), -1, (__fp), (__mode))
#define fdopen(__file, __mode)	__fopen((char*)0, (__file), (FILE*)0, (__mode))

extern FILE *tmpfile __P((void));

extern int fputs __P((const void *, FILE*));
extern int puts __P((const void *));

extern int fread __P((void *, size_t, size_t, FILE *));
extern int fwrite __P((const void *, size_t, size_t, FILE *));

extern int fseek __P((FILE *fp, long offset, int whence));
extern long ftell __P((FILE *fp));
#define fseeko	fseek
#define ftello  ftell

extern int fgetpos __P((FILE *fp, fpos_t *pos));
extern int fsetpos __P((FILE *fp, fpos_t *pos));

extern int printf __P((const char*, ...));
extern int fprintf __P((FILE*, const char*, ...));
extern int sprintf __P((char*, const char*, ...));
extern int snprintf __P((char*, size_t, const char*, ...));

extern int vprintf __P((const char*, va_list));
extern int vfprintf __P((FILE*, const char*, va_list));
extern int _vfnprintf __P((FILE*, size_t, const char*, va_list));
extern int vsprintf __P((char*, const char*, va_list));
extern int vsnprintf __P((char*, size_t, const char*, va_list));

extern int scanf __P((const char*, ...));
extern int fscanf __P((FILE*, const char*, ...));
extern int sscanf __P((char*, const char*, ...));

extern int vscanf __P((const char*, va_list));
extern int vfscanf __P((FILE*, const char*, va_list));
extern int vsscanf __P((char*, const char*, va_list));

extern void perror __P((const char *__s));
extern char *strerror __P((int __errno));

extern char *tmpnam __P((char *buf));

extern int rename __P((const char *oldname, const char *newname));
extern void rewind __P((FILE *fp));
extern FILE *popen __P((const char *, const char *));
extern int pclose __P((FILE *));

#endif /* __STDIO_H */
