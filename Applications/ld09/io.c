/* io.c - input/output and error modules for linker */

/* Copyright (C) 1994 Bruce Evans */

#include "syshead.h"
#include "const.h"
#include "type.h"
#include "globvar.h"
#include "version.h"

#define DRELBUFSIZE	3072
#define ERR		(-1)
#define ERRBUFSIZE	1024
#define INBUFSIZE	1024
#define OUTBUFSIZE	2048
#define TRELBUFSIZE	1024

#ifdef REL_OUTPUT
PRIVATE char *drelbuf;		/* extra output buffer for data relocations */
PRIVATE char *drelbufptr;	/* data relocation output buffer ptr */
PRIVATE char *drelbuftop;	/* data relocation output buffer top */
#endif
PRIVATE char *errbuf;		/* error buffer (actually uses STDOUT) */
PRIVATE char *errbufptr;	/* error buffer ptr */
PRIVATE char *errbuftop;	/* error buffer top */
PRIVATE int  errfil = STDOUT_FILENO;
PRIVATE char *inbuf;		/* input buffer */
PRIVATE char *inbufend;		/* input buffer top */
PRIVATE char *inbufptr;		/* end of input in input buffer */
PRIVATE int infd;		/* input file descriptor */
PRIVATE char *inputname;	/* name of current input file */
PRIVATE char *outbuf;		/* output buffer */
PRIVATE char *outbufptr;	/* output buffer ptr */
PRIVATE char *outbuftop;	/* output buffer top */
PRIVATE int outfd;		/* output file descriptor */
PRIVATE mode_t outputperms;	/* permissions of output file */
PRIVATE char *outputname;	/* name of output file */
PRIVATE char *refname;		/* name of program for error reference */
#ifdef REL_OUTPUT
PRIVATE char *trelbuf;		/* extra output buffer for text relocations */
PRIVATE char *trelbufptr;	/* text relocation output buffer ptr */
PRIVATE char *trelbuftop;	/* text relocation output buffer top */
PRIVATE int trelfd;		/* text relocation output file descriptor */
#endif
PRIVATE unsigned warncount;	/* count of warnings */

#ifdef MSDOS
#define off_t	long		/* NOT a typedef */
#endif

FORWARD void errexit P((char *message));
FORWARD void flushout P((void));
#ifdef REL_OUTPUT
FORWARD void flushtrel P((void));
#endif
FORWARD void outhexdigs P((bin_off_t num));
FORWARD void outputerror P((char *message));
FORWARD void put04x P((unsigned num));
FORWARD void putstrn P((char *message));
FORWARD void refer P((void));
FORWARD void stderr_out P((void));

PUBLIC void ioinit(progname)
char *progname;
{
    infd = ERR;
    if (*progname)
	refname = progname;	/* name must be static (is argv[0]) */
    else
	refname = "link";
    for(progname=refname; *progname; progname++)
       if(*progname=='/')
          refname=progname+1;

#ifdef REL_OUTPUT
    drelbuf = malloc(DRELBUFSIZE);
    drelbuftop = drelbuf + DRELBUFSIZE;
#endif
    errbuf = malloc(ERRBUFSIZE);
    errbufptr = errbuf;
    errbuftop = errbuf + ERRBUFSIZE;
    inbuf = malloc(INBUFSIZE);
    outbuf = malloc(OUTBUFSIZE);/* outbuf invalid if this fails but then */
				/* will not be used - tableinit() aborts */
    outbuftop = outbuf + OUTBUFSIZE;
#ifdef REL_OUTPUT
    trelbuf = malloc(TRELBUFSIZE);
    trelbuftop = trelbuf + TRELBUFSIZE;
#endif
}

PUBLIC void closein()
{
    if (infd != ERR && close(infd) < 0)
	inputerror("cannot close");
    infd = ERR;
}

PUBLIC void closeout()
{
#ifdef REL_OUTPUT
    unsigned nbytes;
#endif

    flushout();
#ifdef REL_OUTPUT
    flushtrel();
    nbytes = drelbufptr - drelbuf;
    if (write(trelfd, drelbuf, nbytes) != nbytes)
	outputerror("cannot write");
#endif
    if (close(outfd) == ERR)
	outputerror("cannot close");
#ifdef REL_OUTPUT
    if (close(trelfd) == ERR)
	outputerror("cannot close");
#endif
}

PUBLIC void errtrace(name, level)
char *name;
int level;
{
    while (level-- > 0)
	putbyte(' ');
    putstrn(name);
}

PUBLIC void executable()
{
    if (errcount)
        unlink(outputname);
#ifndef MSDOS
    else
	chmod(outputname, outputperms);
#endif
}

PUBLIC void flusherr()
{
    if( errbufptr != errbuf )
       write(errfil, errbuf, (unsigned) (errbufptr - errbuf));
    errbufptr = errbuf;
}

PRIVATE void stderr_out()
{
   if( errfil != STDERR_FILENO )
   {
      flusherr();
      errfil = STDERR_FILENO;
   }
}

PRIVATE void flushout()
{
    unsigned nbytes;

    nbytes = outbufptr - outbuf;
    if (write(outfd, outbuf, nbytes) != nbytes)
	outputerror("cannot write");
    outbufptr = outbuf;
}

#ifdef REL_OUTPUT
PRIVATE void flushtrel()
{
    unsigned nbytes;

    nbytes = trelbufptr - trelbuf;
    if (write(trelfd, trelbuf, nbytes) != nbytes)
	outputerror("cannot write");
    trelbufptr = trelbuf;
}
#endif

PUBLIC void openin(filename)
char *filename;
{
#if 0 /* XXX - this probably won't work with constructed lib names? */
    if (infd == ERR || strcmp(inputname, filename) != 0)
#endif
    {
	closein();
	inputname = filename;	/* this relies on filename being static */
#ifdef O_BINARY
	if ((infd = open(filename, O_BINARY|O_RDONLY)) < 0)
#else
	if ((infd = open(filename, O_RDONLY)) < 0)
#endif
	    inputerror("cannot open");
	inbufptr = inbufend = inbuf;
    }
}

PUBLIC void openout(filename)
char *filename;
{
    mode_t oldmask;

    outputname = filename;
#ifdef O_BINARY
    if ((outfd = open(filename, O_BINARY|O_WRONLY|O_CREAT|O_TRUNC, CREAT_PERMS)) == ERR)
#else
    if ((outfd = creat(filename, CREAT_PERMS)) == ERR)
#endif
	outputerror("cannot open");

#ifndef MSDOS
    /* Can't do this on MSDOS; it upsets share.exe */
    oldmask = umask(0); umask(oldmask);
    outputperms = ((CREAT_PERMS | EXEC_PERMS) & ~oldmask);
    chmod(filename, outputperms & ~EXEC_PERMS);
#endif

#ifdef REL_OUTPUT
    drelbufptr = drelbuf;
#endif
    outbufptr = outbuf;
#ifdef REL_OUTPUT
#ifdef O_BINARY
    if ((trelfd = open(filename, O_BINARY|O_WRONLY, CREAT_PERMS)) == ERR)
#else
    if ((trelfd = open(filename, O_WRONLY, CREAT_PERMS)) == ERR)
#endif
	outputerror("cannot reopen");
    trelbufptr = trelbuf;
#endif
}

PRIVATE void outhexdigs(num)
register bin_off_t num;
{
    if (num >= 0x10)
    {
	outhexdigs(num / 0x10);
	num %= 0x10;
    }
    putbyte(hexdigit[num]);
}

PRIVATE void put04x(num)
register unsigned num;
{
    putbyte(hexdigit[num / 0x1000]);
    putbyte(hexdigit[(num / 0x100) & 0x0F]);
    putbyte(hexdigit[(num / 0x10) & 0x0F]);
    putbyte(hexdigit[num & 0x0F]);
}

#ifdef LONG_OFFSETS

PUBLIC void put08lx(num)
register bin_off_t num;
{
    put04x(num / 0x10000);
    put04x(num % 0x10000);
}

#else /* not LONG_OFFSETS */

PUBLIC void put08x(num)
register bin_off_t num;
{
    putstr("0000");
    put04x(num);
}

#endif /* not LONG_OFFSETS */

PUBLIC void putbstr(width, str)
unsigned width;
char *str;
{
    unsigned length;
    
    for (length = strlen(str); length < width; ++length)
	putbyte(' ');
    putstr(str);
}

PUBLIC void putbyte(ch)
int ch;
{
    register char *ebuf;

    ebuf = errbufptr;
    if (ebuf >= errbuftop)
    {
	flusherr();
	ebuf = errbufptr;
    }
    *ebuf++ = ch;
    errbufptr = ebuf;
}

PUBLIC void putstr(message)
char *message;
{
    while (*message != 0)
	putbyte(*message++);
}

PRIVATE void putstrn(message)
char *message;
{
    putstr(message);
    putbyte('\n');
    flusherr(); errfil = STDOUT_FILENO;
}

PUBLIC int readchar()
{
    int ch;
	
    register char *ibuf;
    int nread;

    ibuf = inbufptr;
    if (ibuf >= inbufend)
    {
	ibuf = inbufptr = inbuf;
	nread = read(infd, ibuf, INBUFSIZE);
	if (nread <= 0)
	{
	    inbufend = ibuf;
	    return ERR;
	}
	inbufend = ibuf + nread;
    }
    ch = (unsigned char) *ibuf++;
    inbufptr = ibuf;
    return ch;
}

PUBLIC void readin(buf, count)
char *buf;
unsigned count;
{
    int ch;
    
    while (count--)
    {
	if ((ch = readchar()) < 0)
	    prematureeof();
	*buf++ = ch;
    }
}

PUBLIC bool_pt readineofok(buf, count)
char *buf;
unsigned count;
{
    int ch;
    
    while (count--)
    {
	if ((ch = readchar()) < 0)
	    return TRUE;
	*buf++ = ch;
    }
    return FALSE;
}

PUBLIC void seekin(offset)
unsigned long offset;
{
    inbufptr = inbufend = inbuf;
    if (lseek(infd, (off_t) offset, SEEK_SET) != offset)
	prematureeof();
}

PUBLIC void seekout(offset)
unsigned long offset;
{
    flushout();
    if (lseek(outfd, (off_t) offset, SEEK_SET) != offset)
	outputerror("cannot seek in");
}

#ifdef REL_OUTPUT
PUBLIC void seektrel(offset)
unsigned long offset;
{
    flushtrel();
    if (lseek(trelfd, (off_t) offset, SEEK_SET) != offset)
	outputerror("cannot seek in");
}
#endif

PUBLIC void writechar(ch)
int ch;
{
    register char *obuf;

    obuf = outbufptr;
    if (obuf >= outbuftop)
    {
	flushout();
	obuf = outbufptr;
    }
    *obuf++ = ch;
    outbufptr = obuf;
}

#ifdef REL_OUTPUT
PUBLIC void writedrel(buf, count)
register char *buf;
unsigned count;
{
    register char *rbuf;

    rbuf = drelbufptr;
    while (count--)
    {
	if (rbuf >= drelbuftop)
	    inputerror("data relocation buffer full while processing");
	*rbuf++ = *buf++;
    }
    drelbufptr = rbuf;
}
#endif

PUBLIC void writeout(buf, count)
register char *buf;
unsigned count;
{
    register char *obuf;

    obuf = outbufptr;
    while (count--)
    {
	if (obuf >= outbuftop)
	{
	    outbufptr = obuf;
	    flushout();
	    obuf = outbufptr;
	}
	*obuf++ = *buf++;
    }
    outbufptr = obuf;
}

#ifdef REL_OUTPUT
PUBLIC void writetrel(buf, count)
register char *buf;
unsigned count;
{
    register char *rbuf;

    rbuf = trelbufptr;
    while (count--)
    {
	if (rbuf >= trelbuftop)
	{
	    trelbufptr = rbuf;
	    flushtrel();
	    rbuf = trelbufptr;
	}
	*rbuf++ = *buf++;
    }
    trelbufptr = rbuf;
}
#endif

/* error module */

PRIVATE void errexit(message)
char *message;
{
    putstrn(message);
    exit(2);
}

PUBLIC void fatalerror(message)
char *message;
{
    refer();
    errexit(message);
}

PUBLIC void inputerror(message)
char *message;
{
    refer();
    putstr(message);
    putstr(" input file ");
    errexit(inputname);
}

PUBLIC void input1error(message)
char *message;
{
    refer();
    putstr(inputname);
    errexit(message);
}

PRIVATE void outputerror(message)
char *message;
{
    refer();
    putstr(message);
    putstr(" output file ");
    errexit(outputname);
}

PUBLIC void outofmemory()
{
    inputerror("out of memory while processing");
}

PUBLIC void prematureeof()
{
    inputerror("premature end of");
}

PUBLIC void redefined(name, message, archentry, deffilename, defarchentry)
char *name;
char *message;
char *archentry;
char *deffilename;
char *defarchentry;
{
    ++warncount;
    refer();
    putstr("warning: ");
    putstr(name);
    putstr(" redefined");
    putstr(message);
    putstr(" in file ");
    putstr(inputname);
    if (archentry != NUL_PTR)
    {
	putbyte('(');
	putstr(archentry);
	putbyte(')');
    }
    putstr("; using definition in ");
    putstr(deffilename);
    if (defarchentry != NUL_PTR)
    {
	putbyte('(');
	putstr(defarchentry);
	putbyte(')');
    }
    putstrn("");
}

PUBLIC void interseg(fname, aname, name)
char *fname, *aname, *name;
{
    ++errcount;
    refer();
    putstr("error in ");
    putstr(fname);
    if( aname ) 
    {
       putstr("(");
       putstr(aname);
       putstr(")");
    }
    putstr(" intersegment jump to ");
    if( name ) putstr(name);
    else       putstr("same file");

    putstrn("");
}

PRIVATE void refer()
{
    stderr_out();
    putstr(refname);
    putstr(": ");
}

PUBLIC void reserved(name)
char *name;
{
    ++errcount;
    stderr_out();
    putstr("incorrect use of reserved symbol: ");
    putstrn(name);
}

PUBLIC void size_error(seg, count, size)
int seg;
bin_off_t count;
bin_off_t size;
{
    refer();
    putstr("seg ");
    outhexdigs((bin_off_t) seg);
    putstr(" has wrong size ");
    outhexdigs(count);
    putstr(", supposed to be ");
    outhexdigs(size);
    errexit("");
}

PUBLIC void undefined(name)
char *name;
{
    ++errcount;
    stderr_out();
    putstr("undefined symbol: ");
    putstrn(name);
}

PUBLIC void usage()
{
    stderr_out();
    putstr("usage: ");
    putstr(refname);
#ifdef REL_OUTPUT
    errexit("\
 [-03NMdimrstz[-]] [-llib_extension] [-o outfile] [-Ccrtfile]\n\
       [-Llibdir] [-Olibfile] [-Ttextaddr] [-Ddataaddr] [-Hheapsize] infile...");
#else
    errexit("\
 [-03NMdimstz[-]] [-llib_extension] [-o outfile] [-Ccrtfile]\n\
       [-Llibdir] [-Olibfile] [-Ttextaddr] [-Ddataaddr] [-Hheapsize] infile...");
#endif
}

PUBLIC void version_msg()
{
    stderr_out();
#ifdef VERSION
    putstr("ld86 version: ");
    errexit(VERSION);
#else
    errexit("ld86 version unknown");
#endif
}

PUBLIC void use_error(message)
char *message;
{
    refer();
    putstrn(message);
    usage();
}
