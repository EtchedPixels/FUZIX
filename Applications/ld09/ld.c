/* ld.c - linker for Introl format (6809 C) object files 6809/8086/80386 */

/* Copyright (C) 1994 Bruce Evans */

#include "syshead.h"
#include "const.h"
#include "byteord.h"
#include "type.h"
#include "globvar.h"

/* FIXME: probably want more ?? */
#define MAX_LIBS	5

bin_off_t text_base_value = 0;	/* XXX */
bin_off_t data_base_value = 0;	/* XXX */
int headerless = 0;
char hexdigit[] = "0123456789abcdef";

static bool_t flag[128];
static char *libs[MAX_LIBS] = {
    0
};
static int lastlib = 0;

static char *buildname(char *pre, char *mid, char *suf);
static char *expandlib(char *fn);

static char *buildname(char *pre, char *mid, char *suf)
{
    char *name;

    name = ourmalloc(strlen(pre) + strlen(mid) + strlen(suf) + 1);
    strcpy(name, pre);
    strcat(name, mid);
    strcat(name, suf);
    return name;
}

static char *expandlib(char *fn)
{
    char *path, *s;
    int i;

    for (i = lastlib - 1; i >= 0; --i)
    {
	path = ourmalloc(strlen(libs[i]) + strlen(fn) + 2);
	strcpy(path, libs[i]);
	s = path + strlen(path);
	if (s!=path && s[-1] != '/') strcat(path, "/");
	strcat(path, fn);
	if (access(path, R_OK) == 0)
	    return path;
	ourfree(path);
    }
    return NUL_PTR;
}

int main(int argc, char **argv)
{
    register char *arg;
    int argn;
    static char crtprefix[] = "crt";
    static char crtsuffix[] = ".o";
    char *infilename;
    static char libprefix[] = "lib";
    static char libsuffix[] = ".a";
    char *outfilename;
    char *tfn;
    int icount=0;

    ioinit(argv[0]);
    objinit();
    syminit();
    typeconv_init(INT_BIG_ENDIAN, LONG_BIG_ENDIAN);
    outfilename = NUL_PTR;
    for (argn = 1; argn < argc; ++argn)
    {
	arg = argv[argn];
	if (*arg != '-')
	{
	    readsyms(arg, flag['t']);
	    icount++;
	}
	else
	    switch (arg[1])
	    {
	    case 'v':
	       version_msg();
	    case 'r':		/* relocatable output */
	    case 't':		/* trace modules linked */
	       if (icount > 0) usage();
#ifdef HAS_32BIT
	    case '0':		/* use 16-bit libraries */
	    case '3':		/* use 32-bit libraries */
#endif
	    case 'M':		/* print symbols linked */
	    case 'i':		/* separate I & D output */
	    case 'm':		/* print modules linked */
	    case 's':		/* strip symbols */
	    case 'z':		/* unmapped zero page */
	    case 'd':		/* Make a headerless outfile */
		if (arg[2] == 0)
		    flag[(int) arg[1]] = TRUE;
		else if (arg[2] == '-' && arg[3] == 0)
		    flag[(int) arg[1]] = FALSE;
		else
		    usage();
		if (arg[1] == '0')	/* flag 0 is negative logic flag 3 */
		    flag['3'] = !flag['0'];
		break;
	    case 'C':		/* startfile name */
		tfn = buildname(crtprefix, arg + 2, crtsuffix);
		if ((infilename = expandlib(tfn)) == NUL_PTR)
		    infilename = tfn;
		    /*fatalerror(tfn);	* XXX - need to describe failure */
		readsyms(infilename, flag['t']);
		icount++;
		break;
	    case 'L':		/* library path */
		if (lastlib < MAX_LIBS)
		    libs[lastlib++] = arg + 2;
		else
		    fatalerror("too many library paths");
		break;
	    case 'O':		/* library file name */
		if ((infilename = expandlib(arg + 2)) == NUL_PTR)
		    infilename = arg+2;
		    /* fatalerror(arg);	* XXX */
		readsyms(infilename, flag['t']);
		break;
	    case 'T':		/* text base address */
		if (arg[2] == 0 && ++argn >= argc)
		    usage();
		errno = 0;    
		if (arg[2] == 0 )
		   text_base_value = strtoul(argv[argn], (char **)0, 16);
		else
		   text_base_value = strtoul(arg+2, (char **)0, 16);
		if (errno != 0)
		    use_error("invalid text address");
		break;
	    case 'D':		/* data base address */
		if (arg[2] == 0 && ++argn >= argc)
		    usage();
		errno = 0;    
		if (arg[2] == 0 )
		   data_base_value = strtoul(argv[argn], (char **)0, 16);
		else
		   data_base_value = strtoul(arg+2, (char **)0, 16);
		if (errno != 0)
		    use_error("invalid data address");
		break;
	    case 'l':		/* library name */
		tfn = buildname(libprefix, arg + 2, libsuffix);
		if ((infilename = expandlib(tfn)) == NUL_PTR)
		    infilename = tfn;
		    /* fatalerror(tfn);	* XXX */
		readsyms(infilename, flag['t']);
		icount+=2;
		break;
	    case 'o':		/* output file name */
		if (arg[2] != 0 || ++argn >= argc || outfilename != NUL_PTR)
		    usage();
		outfilename = argv[argn];
		break;
	    default:
		usage();
	    }
    }
    if(icount==0) usage();

    /* Headerless executables can't use symbols. */
    headerless = flag['d'];
    if( headerless ) flag['s'] = 1;

    linksyms(flag['r'] | flag['B']);
    if (outfilename == NUL_PTR)
	outfilename = "a.out";
    write_fuzix(outfilename, flag['i'], flag['3'], flag['s'],
	     flag['z']);
    if (flag['m'])
	dumpmods();
    if (flag['M'])
	dumpsyms();
    flusherr();
    return errcount ? 1 : 0;
}
