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

#define putc(c, stream)	fputc((c), stream)
#define getc(stream)	fgetc(stream)

#define putchar(c)	fputc((c), stdout)
#define getchar()	fgetc(stdin)

extern char *gets(char *__s);
extern char *gets_s(char *__s, size_t __size);

extern int _putchar(int __c);
extern int _getchar(void);

#define ferror(fp)	(((fp)->mode&__MODE_ERR) != 0)
#define feof(fp)	(((fp)->mode&__MODE_EOF) != 0)
#define clearerr(fp)	((fp)->mode &= ~(__MODE_EOF|__MODE_ERR))
#define fileno(fp)	((fp)->fd)

/* These two call malloc */
extern int setvbuf(FILE *__stream, char *__buf, int __mode, size_t __size);
#define setlinebuf(__fp)	setvbuf((__fp), (char*)0, _IOLBF, 0)

/* These don't */
extern void setbuffer(FILE *__stream, char *__buf, size_t __size);
#define setbuf(__fp, __buf)	setbuffer((__fp), (__buf), BUFSIZ)

extern int fgetc(FILE *__stream);
extern int fputc(int, FILE *__stream);
extern int ungetc(int, FILE *__stream);

extern int fclose(FILE *__stream);
extern int fflush(FILE *__stream);
/* FIXME: this symbol needs to be __ */
#define stdio_pending(fp) ((fp)->bufread > (fp)->bufpos)
extern char *fgets(char *__s, size_t __size, FILE *__stream);
extern FILE *__fopen(const char *__path, int __fd, FILE * __stream, const char *__mode);

#define fopen(__file, __mode)	      __fopen((__file), -1, (FILE*)0, (__mode))
#define freopen(__file, __mode, __fp) __fopen((__file), -1, (__fp), (__mode))
#define fdopen(__file, __mode)	__fopen((char*)0, (__file), (FILE*)0, (__mode))

extern FILE *tmpfile(void);

extern int fputs(const void *__s, FILE *__stream);
extern int puts(const void *__s);

extern int fread(void *__ptr, size_t __size, size_t, FILE *__stream);
extern int fwrite(const void *__ptr, size_t __size, size_t __nmemb, FILE *__stream);

extern int fseek(FILE *__stream, long __offset, int __whence);
extern long ftell(FILE *__stream);
#define fseeko	fseek
#define ftello  ftell

extern int fgetpos(FILE *__stream, fpos_t *__pos);
extern int fsetpos(FILE *__stream, fpos_t *__pos);

extern int printf(const char *__fmt, ...);
extern int fprintf(FILE *__stream, const char *__fmt, ...);
extern int sprintf(char *__str, const char *__fmt, ...);
extern int snprintf(char *__str, size_t __size, const char *__fmt, ...);

extern int vprintf(const char*, va_list __ap);
extern int vfprintf(FILE *__stream, const char*, va_list __ap);
extern int _vfnprintf(FILE *__stream, size_t __size, const char *__fmt, va_list __ap);
extern int vsprintf(char *__str, const char *__fmt, va_list __ap);
extern int vsnprintf(char *__str, size_t __size, const char *__fmt, va_list __ap);

extern int scanf(const char *__fmt, ...);
extern int fscanf(FILE *__stream, const char *__fmt, ...);
extern int sscanf(const char *__str, const char *__fmt, ...);

extern int vscanf(const char *__fmt, va_list __ap);
extern int vfscanf(FILE *__stream, const char *__fmt, va_list __ap);
extern int vsscanf(const char *__str, const char *__fmt, va_list __ap);

extern void perror(const char *__s);

extern char *tmpnam(char *__buf);

extern int rename(const char *__oldname, const char *__newname);
extern void rewind(FILE *__stream);
extern FILE *popen(const char *__command, const char *__type);
extern int pclose(FILE *__stream);

extern char *cuserid(char *__buf);
#define L_cuserid	9
extern char *ctermid(char *__buf);
#define L_ctermid	9

extern int getw(FILE *__f);
extern int putw(int __n, FILE *__f);

extern long a64l(const char *__str64);
extern char *l64a(long __value);

#endif /* __STDIO_H */
