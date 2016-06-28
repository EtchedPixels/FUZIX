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
static char *drelbuf;		/* extra output buffer for data relocations */
static char *drelbufptr;	/* data relocation output buffer ptr */
static char *drelbuftop;	/* data relocation output buffer top */
#endif
static char *errbuf;		/* error buffer (actually uses STDOUT) */
static char *errbufptr;	/* error buffer ptr */
static char *errbuftop;	/* error buffer top */
static int  errfil = STDOUT_FILENO;
static char *inbuf;		/* input buffer */
static char *inbufend;		/* input buffer top */
static char *inbufptr;		/* end of input in input buffer */
static int infd;		/* input file descriptor */
static char *inputname;	/* name of current input file */
static char *outbuf;		/* output buffer */
static char *outbufptr;	/* output buffer ptr */
static char *outbuftop;	/* output buffer top */
static int outfd;		/* output file descriptor */
static mode_t outputperms;	/* permissions of output file */
static char *outputname;	/* name of output file */
static const char *refname;	/* name of program for error reference */
#ifdef REL_OUTPUT
static char *trelbuf;		/* extra output buffer for text relocations */
static char *trelbufptr;	/* text relocation output buffer ptr */
static char *trelbuftop;	/* text relocation output buffer top */
static int trelfd;		/* text relocation output file descriptor */
#endif
static unsigned warncount;	/* count of warnings */

static void errexit(char *message);
static void flushout(void);
#ifdef REL_OUTPUT
static void flushtrel(void);
#endif
static void outhexdigs(bin_off_t num);
static void outputerror(char *message);
static void put04x(unsigned num);
static void putstrn(const char *message);
static void refer(void);
static void stderr_out(void);

void ioinit(char *progname)
{
    infd = ERR;
    if (*progname)
	refname = progname;	/* name must be static (is argv[0]) */
    else
	refname = "link";
    for(progname=refname; *progname; progname++)
       if(*progname=='/')
          refname=progname+1;

    /* FIXME: we need a malloc with error check here */
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

void closein(void)
{
    if (infd != ERR && close(infd) < 0)
	inputerror("cannot close");
    infd = ERR;
}

void closeout()
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

void errtrace(char *name, int level)
{
    while (level-- > 0)
	putbyte(' ');
    putstrn(name);
}

void executable(void)
{
    if (errcount)
        unlink(outputname);
    else
	chmod(outputname, outputperms);
}

void flusherr(void)
{
    if( errbufptr != errbuf )
       write(errfil, errbuf, (unsigned) (errbufptr - errbuf));
    errbufptr = errbuf;
}

static void stderr_out(void)
{
   if( errfil != STDERR_FILENO )
   {
      flusherr();
      errfil = STDERR_FILENO;
   }
}

static void flushout(void)
{
    unsigned nbytes;

    nbytes = outbufptr - outbuf;
    if (write(outfd, outbuf, nbytes) != nbytes)
	outputerror("cannot write");
    outbufptr = outbuf;
}

#ifdef REL_OUTPUT
static void flushtrel(void)
{
    unsigned nbytes;

    nbytes = trelbufptr - trelbuf;
    if (write(trelfd, trelbuf, nbytes) != nbytes)
	outputerror("cannot write");
    trelbufptr = trelbuf;
}
#endif

void openin(char *filename)
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

void openout(char *filename)
{
    mode_t oldmask;

    outputname = filename;
#ifdef O_BINARY
    if ((outfd = open(filename, O_BINARY|O_WRONLY|O_CREAT|O_TRUNC, CREAT_PERMS)) == ERR)
#else
    if ((outfd = creat(filename, CREAT_PERMS)) == ERR)
#endif
	outputerror("cannot open");

    /* Can't do this on MSDOS; it upsets share.exe */
    oldmask = umask(0); umask(oldmask);
    outputperms = ((CREAT_PERMS | EXEC_PERMS) & ~oldmask);
    chmod(filename, outputperms & ~EXEC_PERMS);

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

static void outhexdigs(bin_off_t num)
{
    if (num >= 0x10)
    {
	outhexdigs(num / 0x10);
	num %= 0x10;
    }
    putbyte(hexdigit[num]);
}

static void put04x(unsigned num)
{
    putbyte(hexdigit[num / 0x1000]);
    putbyte(hexdigit[(num / 0x100) & 0x0F]);
    putbyte(hexdigit[(num / 0x10) & 0x0F]);
    putbyte(hexdigit[num & 0x0F]);
}

#ifdef LONG_OFFSETS

void put08lx(bin_off_t num)
{
    put04x(num / 0x10000);
    put04x(num % 0x10000);
}

#else /* not LONG_OFFSETS */

void put08x(bin_off_t num)
{
    putstr("0000");
    put04x(num);
}

#endif /* not LONG_OFFSETS */

void putbstr(unsigned width, char *str)
{
    unsigned length;
    
    for (length = strlen(str); length < width; ++length)
	putbyte(' ');
    putstr(str);
}

void putbyte(int ch)
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

void putstr(const char *message)
{
    while (*message != 0)
	putbyte(*message++);
}

static void putstrn(const char *message)
{
    putstr(message);
    putbyte('\n');
    flusherr(); errfil = STDOUT_FILENO;
}

int readchar(void)
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

void readin(char *buf, unsigned count)
{
    int ch;
    
    while (count--)
    {
	if ((ch = readchar()) < 0)
	    prematureeof();
	*buf++ = ch;
    }
}

bool_pt readineofok(char *buf, unsigned count)
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

void seekin(unsigned long offset)
{
    inbufptr = inbufend = inbuf;
    if (lseek(infd, (off_t) offset, SEEK_SET) != offset)
	prematureeof();
}

void seekout(unsigned long offset)
{
    flushout();
    if (lseek(outfd, (off_t) offset, SEEK_SET) != offset)
	outputerror("cannot seek in");
}

#ifdef REL_OUTPUT
void seektrel(unsigned long offset)
{
    flushtrel();
    if (lseek(trelfd, (off_t) offset, SEEK_SET) != offset)
	outputerror("cannot seek in");
}
#endif

void writechar(int ch)
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
void writedrel(char *buf, unsigned count)
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

void writeout(char *buf, unsigned count)
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
void writetrel(char *buf, unsigned count)
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

static void errexit(char *message)
{
    putstrn(message);
    exit(2);
}

void fatalerror(char *message)
{
    refer();
    errexit(message);
}

void inputerror(char *message)
{
    refer();
    putstr(message);
    putstr(" input file ");
    errexit(inputname);
}

void input1error(char *message)
{
    refer();
    putstr(inputname);
    errexit(message);
}

static void outputerror(char *message)
{
    refer();
    putstr(message);
    putstr(" output file ");
    errexit(outputname);
}

void outofmemory(void)
{
    inputerror("out of memory while processing");
}

void prematureeof(void)
{
    inputerror("premature end of");
}

void redefined(char *name, char *message, char *archentry,
		      char *deffilename, char *defarchentry)
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

void interseg(char *fname, char *aname, char *name)
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

static void refer(void)
{
    stderr_out();
    putstr(refname);
    putstr(": ");
}

void reserved(char *name)
{
    ++errcount;
    stderr_out();
    putstr("incorrect use of reserved symbol: ");
    putstrn(name);
}

void size_error(int seg, bin_off_t count, bin_off_t size)
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

void undefined(char *name)
{
    ++errcount;
    stderr_out();
    putstr("undefined symbol: ");
    putstrn(name);
}

void usage(void)
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

void version_msg(void)
{
    stderr_out();
#ifdef VERSION
    putstr("ld86 version: ");
    errexit(VERSION);
#else
    errexit("ld86 version unknown");
#endif
}

void use_error(char *message)
{
    refer();
    putstrn(message);
    usage();
}
