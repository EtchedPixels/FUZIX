/* ld.c - linker for Introl format (6809 C) object files 6809/8086/80386 */

/* Copyright (C) 1994 Bruce Evans */

#include "syshead.h"
#include "const.h"
#include "byteord.h"
#include "type.h"
#include "globvar.h"

#define MAX_LIBS	(NR_STDLIBS + 5)
#ifdef MC6809
#define NR_STDLIBS	1
#else
#define NR_STDLIBS	0
#endif

PUBLIC bin_off_t text_base_value = 0;	/* XXX */
PUBLIC bin_off_t data_base_value = 0;	/* XXX */
PUBLIC bin_off_t heap_top_value  = 0;	/* XXX */
PUBLIC int headerless = 0;
#ifndef VERY_SMALL_MEMORY
PUBLIC int v7 = 0;
#endif
#ifndef MSDOS
PUBLIC int cpm86 = 0;
#endif
PUBLIC char hexdigit[] = "0123456789abcdef";

PRIVATE bool_t flag[128];
PRIVATE char *libs[MAX_LIBS] = {
#ifdef MC6809
    "/usr/local/lib/m09/",
#endif
    0
};
PRIVATE int lastlib = NR_STDLIBS;

FORWARD char *buildname P((char *pre, char *mid, char *suf));
FORWARD char *expandlib P((char *fn));

PRIVATE char *buildname(pre, mid, suf)
char *pre;
char *mid;
char *suf;
{
    char *name;

    name = ourmalloc(strlen(pre) + strlen(mid) + strlen(suf) + 1);
    strcpy(name, pre);
    strcat(name, mid);
    strcat(name, suf);
    return name;
}

PRIVATE char *expandlib(fn)
char *fn;
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

PUBLIC int main(argc, argv)
int argc;
char **argv;
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
#ifndef MC6809
    flag['3'] = sizeof(char *) >= 4;
#endif
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
#ifdef REL_OUTPUT
	    case 'B':		/* Broken -r for dosemu. */
#endif
	    case '0':		/* use 16-bit libraries */
	    case '3':		/* use 32-bit libraries */
	    case 'M':		/* print symbols linked */
	    case 'i':		/* separate I & D output */
	    case 'm':		/* print modules linked */
	    case 's':		/* strip symbols */
	    case 'z':		/* unmapped zero page */
	    case 'N':		/* Native format a.out */
	    case 'd':		/* Make a headerless outfile */
#ifndef MSDOS
	    case 'c':		/* Write header in CP/M-86 format */
#endif
	    case 'y':		/* Use a newer symbol table */
#ifndef VERY_SMALL_MEMORY
	    case '7':		/* Produce a UNIX v7 a.out header */
#endif
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
	    case 'H':		/* heap top address */
		if (arg[2] == 0 && ++argn >= argc)
		    usage();
		errno = 0;    
		if (arg[2] == 0 )
		   heap_top_value = strtoul(argv[argn], (char **)0, 16);
		else
		   heap_top_value = strtoul(arg+2, (char **)0, 16);
		if (errno != 0)
		    use_error("invalid heap top");
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

#ifdef BUGCOMPAT
    if( icount==1 && ( flag['r'] && !flag['N'] ) ) {
       flag['r'] = 0;
       flag['B'] = 1;
    }
#endif

#ifdef REL_OUTPUT
#ifndef MSDOS
    if( flag['r'] && !flag['N'] )
    {
       /* Do a relocatable link -- actually fake it with 'ar.c' */
       ld86r(argc, argv);
    }
#endif
#endif

#ifdef MSDOS
    /* MSDOS Native is special, we make a COM file */
    if( flag['N'] )
    {
       flag['N'] = 0;
       flag['d'] = 1;
       text_base_value = 0x100;
    }
#endif

    /* Headerless executables can't use symbols. */
    headerless = flag['d'];
    if( headerless ) flag['s'] = 1;

#ifndef VERY_SMALL_MEMORY
    /* UNIX seventh edition executables */
    v7 = flag['7'];
#endif

#ifndef MSDOS
    /* CP/M-86 executables can't use symbols. */
    cpm86 = flag['c'];
    if ( cpm86 ) flag['s'] = 1;
#endif

    linksyms(flag['r'] | flag['B']);
    if (outfilename == NUL_PTR)
	outfilename = "a.out";
#ifndef MSDOS
    if( flag['N'] )
       writebin(outfilename, flag['i'], flag['3'], flag['s'],
	     flag['z'] & flag['3']);
    else
#endif
    if( flag['B'] )
       write_dosemu(outfilename, flag['i'], flag['3'], flag['s'],
	  flag['z'] & flag['3']);
#if 0	/* FIXME */
    else
       write_elks(outfilename, flag['i'], flag['3'], flag['s'],
	     flag['z'], flag['y']);
#endif	     
    if (flag['m'])
	dumpmods();
    if (flag['M'])
	dumpsyms();
    flusherr();
    return errcount ? 1 : 0;
}
