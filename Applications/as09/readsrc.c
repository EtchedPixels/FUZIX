/* readsrc.c - read source files for assembler */

#include "syshead.h"
#include "const.h"
#include "type.h"
#include "flag.h"
#include "file.h"
#include "globvar.h"
#include "macro.h"
#include "scan.h"
#undef EXTERN
#define EXTERN
#include "source.h"

/*
 * Ok, lots of hack & slash here.
 * 1) Added BIG buffer to load entire _primary_ file into memory.
 * 2) This means the primary file can be standard input.
 * 3) Fixed so 'get/include' processing now works.
 * 4) Altered for a 'normal' style buffer otherwise (MINIBUF)
 * 5) Have the option of completely unbuffered if you need the last Kb.
 *
 * RDB.
 */

#if !defined(__AS386_16__) && !defined(__m6809__)
#ifndef BIGBUFFER
#define BIGBUFFER	1	/* For most machines we have the memory */
#endif
#endif

#if !defined(__m6809__)
#ifndef MINIBUF
#define MINIBUF		1	/* Add in a reasonable buffer */
#endif
#endif

struct get_s			/* to record included files */
{
    fd_t fd;
    unsigned line;
    off_t position;
};

static char hid_filnambuf[FILNAMLEN + 1];	/* buffer for file name */

static struct get_s hid_getstak[MAXGET];	/* GET stack */
static struct get_s *getstak;	/* ptr */

#if BIGBUFFER == 1
static char *mem_start = 0, *mem_end;
#endif

static char hid_linebuf[LINLEN];	/* line buffer */
static char *eol_ptr;

static char *maclinebuf;
static char *maclineptr;

#if MINIBUF == 1
static void inp_seek(int fd, long posn);
static long inp_tell(int fd);
static int inp_line(int fd, char * buf, int size);
#endif

static void clearsource(void);
static void line_too_long(void);

static void clearsource(void)
{
}

static void line_too_long(void)
{
    symname = linebuf + (LINLEN - 1);	/* spot for the error */
    error(LINLONG);		/* so error is shown in column LINLEN - 1 */
}

/* initialise private variables */

void initsource(void)
{
    filnamptr = hid_filnambuf;
    getstak = hid_getstak;
    clearsource();		/* sentinel to invoke blank skipping */
}

fd_t open_input(char *name)
{
    fd_t fd;
#if BIGBUFFER == 1
    off_t filelength = -1;

    if( mem_start == 0 && strcmp(name, "-") == 0 )
       fd = 0;
    else
#endif
#ifdef O_BINARY
    if ((unsigned) (fd = open(name, O_RDONLY|O_BINARY)) > 255)
	as_abort("error opening input file");
#else
    if ((unsigned) (fd = open(name, O_RDONLY)) > 255)
	as_abort("error opening input file");
#endif

#if BIGBUFFER == 1
    if( mem_start == 0 )
    {
	size_t memsize = 0;
	int cc;

	if(fd)
	{
	   struct stat st;
	   if( fstat(fd, &st) >= 0 )
	      filelength = st.st_size;
           /* FIXME: check allowing for the +4 below */
	   if( filelength > (((unsigned)-1)>>1)-3 )
	   {
	      mem_end = mem_start = "\n\n";
	      goto cant_do_this;
 	   }
	}

	if (filelength > 0) {
	   if( (mem_start = malloc(filelength+4)) == 0 )
	   {
	      mem_end = mem_start = "\n\n";
	      goto cant_do_this;
	   }
	   memsize = filelength;

	   filelength = read(fd, mem_start, filelength);
	} else
	   filelength = 0;

	for(;;)
	{
	    if( filelength >= memsize )
	    {
		if (memsize > 16000)
		    mem_start = asrealloc(mem_start, (memsize+=16384)+4);
		else
		    mem_start = asrealloc(mem_start, (memsize+=memsize+32)+4);
	    }
	    cc = read(fd, mem_start+filelength,
			  (size_t)(memsize-filelength));
	    if( cc <= 0 ) break;
	    filelength+=cc;
	}

	*(mem_end=mem_start+filelength) = '\n';
	mem_end[1] = '\0';

	infiln = infil0 = 0;	/* Assemble from memory */
	if(fd) close(fd);
	fd = -1;
    }
cant_do_this:
#endif

    clearsource();
    return fd;
}

/*
  handle GET pseudo_op
  stack state of current file, open new file and reset global state vars
  file must be seekable for the buffer discard/restore method to work
*/

void pget(void)
{
    if (infiln >= MAXGET)
	error(GETOV);
    else
    {
	char save;

	skipline();
	listline();

	getstak->fd = infil;
	getstak->line = linum;
	if (infiln != 0)
#if MINIBUF == 1
	    getstak->position = inp_tell(infil);
#else
	    getstak->position = lseek(infil, 0L, 1);
#endif
	else
	    getstak->position = (off_t)eol_ptr;
	++getstak;
	++infiln;
	linum = 0;

	for(lineptr=symname; *lineptr != EOLCHAR; lineptr++)
	   if( *lineptr <= ' ' ) break;
	save = *lineptr; *lineptr = '\0';
	infil = open_input(symname);
	*lineptr = save;
	getsym();
    }
}

/* process end of file */
/* close file, unstack old file if current one is included */
/* otherwise switch pass 0 to pass 1 or exit on pass 2 */
/* end of file may be from phyical end of file or an END statement */

void pproceof(void)
{
    if (infiln != 0)
	close(infil);
    if (infiln == infil0)
	/* all conditionals must be closed before end of main file (not GETs) */
    {
	if (blocklevel != 0)
	    error(EOFBLOCK);
	if (iflevel != 0)
	    error(EOFIF);
	if (pass && (lcdata & UNDBIT))
	    error(EOFLC);
	lcptr->data = lcdata;
	lcptr->lc = lc;
    }
    /* macros must be closed before end of all files */
    if (macload)
	error(EOFMAC);
    if (linebuf != lineptr)
        listline();		/* last line or line after last if error */
    if (infiln != infil0)
    {
	--getstak;
	infil = getstak->fd;
	linum = getstak->line;
	if (--infiln == 0)
	    eol_ptr = (void*)getstak->position;
	else
#if MINIBUF == 1
	    inp_seek(infil, getstak->position);
#else
	    lseek(infil, getstak->position, 0);
#endif
    }
    else if (pass!=last_pass)
    {
	pass++;
	if( last_pass>1 && last_pass<30 && dirty_pass && pass==last_pass )
	   last_pass++;

	if( pass==last_pass )
	   objheader();		/* while pass 1 data all valid */
	binmbuf = 0;		/* reset zero variables */
	maclevel = iflevel = blocklevel =
	    totwarn = toterr = linum = macnum = 0;
	initp1p2();		/* reset other varaiables */
	if(pass==last_pass)
	   binaryc = binaryg;
#ifdef I80386
	defsize = idefsize;
	cpuid = origcpuid;
#endif
	if(pass==last_pass)
	{
	   list.current = list.global;
	   maclist.current = maclist.global;
	   as_warn.current = TRUE;
	   if (as_warn.semaphore < 0)
	       as_warn.current = FALSE;
	}

	if (infiln != 0)
	    infil = open_input(filnamptr);
        else
	    eol_ptr=0;

	if(pass==last_pass)
	   binheader();

	line_zero();
    }
    else
	finishup();
}

/*
  read 1 line of source.
  Source line ends with '\n', line returned is null terminated without '\n'.
  Control characters other than blank, tab and newline are discarded.
  Long lines (length > LINLEN) are truncated, and an error is generated.
  On EOF, calls pproceof(), and gets next line unless loading a macro.
  This is where macro lines are recursively expanded.
*/

void readline(void)
{
    int cc = 0;

    listpre = FALSE;		/* not listed yet */
    if (maclevel != 0)
    {
      register char *bufptr;	/* hold *bufptr in a reg char variable */
      register char *reglineptr;	/* if possible (not done here) */
      char *oldbufptr;
      struct schain_s *parameters;
      char paramnum;
      unsigned int remaining;	/* space remaining in line + 2 */
				/* value 0 not used except for temp predec */
				/* value 1 means error already gen */
				/* values 1 and 2 mean no space */

      for (; maclevel != 0;
	     macpar = macstak->parameters, ++macstak, --maclevel)
	if (*(bufptr = macstak->text) != ETB)
 /* nonempty macro, process it and return without continuing the for loop */
	{
	    if (!macflag)
	    {
		maclinebuf = linebuf;
		maclineptr = lineptr;
		macflag = TRUE;
	    }
	    remaining = LINLEN + 2;
	    lineptr = linebuf = reglineptr = hid_linebuf;
	    while (*bufptr++ != EOLCHAR)
	    {
		if (bufptr[-1] == MACROCHAR && *bufptr >= '0' && *bufptr <= '9')
		{
		    parameters = macstak->parameters;
		    for (paramnum = *bufptr++; paramnum-- != '0';)
			if ((parameters = parameters->next) == NUL_PTR)
			    break;
		    if (parameters != NUL_PTR)
		    {
			for (oldbufptr = bufptr, bufptr = parameters->string;
			     *bufptr++ != 0;)
			{
			    if (--remaining <= 1)
			    {
				if (remaining != 0)
				    line_too_long();
				remaining = 1;
				break;	/* forget rest, param on 1 line */
			    }
			    *reglineptr++ = bufptr[-1];
			}
			bufptr = oldbufptr;
		    }
		}
		else
		{
		    if (--remaining <= 1)
		    {
			if (remaining != 0)
			    line_too_long();
			remaining = 1;
		    }
		    else
			*reglineptr++ = bufptr[-1];
		}
	    }
	    macstak->text = bufptr;
#if 0
            *reglineptr = 0;
            printf("MLINE:%s.\n", lineptr);
#endif
	    *reglineptr = EOLCHAR;
	    return;
	}
    }
    if (macflag)
    {
	linebuf = maclinebuf;
	lineptr = maclineptr;
	macflag = FALSE;
    }
    /* End of macro expansion processing */

again:	/* On EOF for main or included files */
    ++linum;

#if BIGBUFFER == 1
    if( infiln == 0 )
    {
       if( eol_ptr == 0 ) eol_ptr = mem_start-1;
       else *eol_ptr = '\n';
       linebuf = lineptr = eol_ptr + 1;
       cc = (mem_end - linebuf);

       /* memchr not strchr 'cause some implementations of strchr are like:
	  memchr(x,y,strlen(x)); this is _BAD_ with BIGBUFFER
	*/
       if((eol_ptr = memchr(linebuf, '\n', cc)) == 0 && cc > 0)
          cc = -1;
    }
    else
#endif
    {
       lineptr = linebuf = hid_linebuf;
       *(hid_linebuf+sizeof(hid_linebuf)-2) = '\0';	/* Term */

#if MINIBUF == 1
       cc = inp_line(infil, linebuf, sizeof(hid_linebuf)-2);
       if( cc >= 0 )
          eol_ptr = linebuf+cc-1;
#else
       cc = read(infil, linebuf, sizeof(hid_linebuf)-2);
       if( cc > 0 )
       {
          eol_ptr = memchr(linebuf, '\n', cc);
	  if( eol_ptr == 0 )
	     eol_ptr = hid_linebuf+sizeof(hid_linebuf)-2;
	  else
	     lseek(infil, (long)(eol_ptr+1-hid_linebuf)-cc, 1);
       }
#endif
    }

    if( cc <= 0 )
    {
        if( cc < 0 ) as_abort("error reading input");

        clearsource();
        pproceof();
	listpre = FALSE;
        if (macload)
        {
	    symname = lineptr;
	    return;		/* macro not allowed across eof */
        }
        goto again;
    }

#if 0
    *eol_ptr = 0;
    printf("LINE:%s.\n", lineptr);
#endif
    *eol_ptr = EOLCHAR;
}

void skipline(void)
{
    if(macflag)
        lineptr = strchr(hid_linebuf, EOLCHAR);
    else
        lineptr = eol_ptr;
}

#if MINIBUF == 1
static char input_buf[1024];		/* input buffer */
static int  in_start=0, in_end=0;
static long ftpos = 0;
static int  lastfd = -1;

static int inp_line(int fd, char *buf, int size)
{
   int offt = 0;
   if( fd!=lastfd ) inp_seek(-1, 0L);
   for(;;)
   {
      if(in_start >= in_end)
      {
	 lastfd = -1;
         ftpos = lseek(fd, 0L, 1);
	 in_start = 0;
         in_end = read(fd, input_buf, sizeof(input_buf));
	 if( in_end <=0 ) return in_end;
	 lastfd = fd;
      }
      if( (buf[offt++] = input_buf[in_start++]) == '\n' || offt >= size )
         break;
   }
   return offt;
}

static long inp_tell(int fd)
{
   if( fd != lastfd )
      return lseek(fd, 0L, 1);
   else
      return ftpos + in_start;
}

static void inp_seek(int fd, long posn)
{
   if( lastfd != -1 )
      lseek(lastfd, ftpos+in_start, 0);
   lastfd = -1;
   in_end = 0;
   if( fd >= 0 )
      lseek(fd, posn, 0);
}

#endif
