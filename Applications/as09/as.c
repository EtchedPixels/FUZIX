/* as.c - assembler */

/*
  usage: as source [-b [bin]] [-lm [list]] [-n name] [-o obj] [-s sym] [-guw]
  in any order (but no repeated file options)
*/

#include "syshead.h"
#include "const.h"
#include "type.h"
#include "byteord.h"
#include "macro.h"
#undef EXTERN
#define EXTERN
#include "file.h"
#include "flag.h"
#include "globvar.h"
#include "version.h"

char hexdigit[] = "0123456789ABCDEF";	/* XXX - ld uses lower case */

static struct block_s hid_blockstak[MAXBLOCK];	/* block stack */
static struct lc_s hid_lctab[NLOC];	/* location counter table */
static struct if_s hid_ifstak[MAXBLOCK];	/* if stack */
static struct schain_s hid_mcpar[MACPSIZ];	/* MACRO params */
static struct macro_s hid_macstak[MAXBLOCK];	/* macro stack */
static struct sym_s *hid_spt[SPTSIZ];	/* hash table */

static char * binfilename = 0;
static char * objfilename = 0;
static int keep_bad_output = 0;

static void initp1(void);
static int my_creat(char *name, char *message);
static void process_args(int argc, char *argv[]);
static void summary(fd_t fd);
static void summ_number(unsigned num);
static void usage(void);

int main(int argc, char *argv[])
{
    init_heap();
    initp1();
    initp1p2();
    inst_keywords();
    initbin();
    initobj();
    initsource();
    typeconv_init(INT_BIG_ENDIAN, LONG_BIG_ENDIAN);
    as_warn.global = TRUE;		/* constant */
    as_warn.semaphore = -1;
    last_pass=1;
    process_args(argc, argv);
    initscan();
    line_zero();

    assemble();			/* doesn't return, maybe use setjmp */

    /* NOTREACHED */
    return 0;
}

void as_abort(char *message)
{
    write(STDOUT, "as: ", 4);
    write(STDOUT, message, strlen(message));
    write(STDOUT, "\n", 1);
    exit(1);
}

void finishup(void)
{
    bintrailer();
    objtrailer();
    if (list.global ||symgen)
	gensym();		/* output to lstfil and/or symfil */
    if (list.global ||toterr != 0 || totwarn != 0)
	summary(lstfil);
    if (lstfil != STDOUT && (toterr != 0 || totwarn != 0))
	summary(STDOUT);
    statistics();

    /* If an output binary is in error remove it */
    close(binfil); binfil=0;
    close(objfil); objfil=0;
    if (toterr != 0 && !keep_bad_output) 
    {
       if(binfilename) unlink(binfilename);
       if(objfilename) unlink(objfilename);
    }

    exit(toterr != 0 ? 1 : 0);	/* should close output files and check */
}

/* initialise constant nonzero values */

static void initp1(void)
{
#if defined(I80386)
    idefsize = defsize = 2;	/* I think this is probably safer (RDB) */
#endif
    lctabtop = lctab + NLOC;
    lstfil = STDOUT;
    mapnum = 15;		/* default map number for symbol table */
    spt_top = (spt = hid_spt) + SPTSIZ;
}

/* initialise nonzero values which start same each pass */

void initp1p2(void)
{
    register struct lc_s *lcp;

    dirty_pass = 0;
    ifflag = TRUE;
    pedata = UNDBIT;		/* program entry point not defined */
    blockstak = hid_blockstak + MAXBLOCK;
    ifstak = hid_ifstak + MAXIF;
    macstak = hid_macstak + MAXMAC;
    macptop = (macpar = hid_mcpar) + MACPSIZ;
    lctabtop = (lcptr = lctab = hid_lctab) + NLOC;
    for (lcp = lctab; lcp < lctabtop; ++lcp)
    {
	lcp->data = lcdata = RELBIT;	/* lc relocatable until 1st ORG */
	lcp->lc = lc = 0;
    }
}

void line_zero(void)
{
   if( textseg >= 0 )
      ptext();
}

static int my_creat(char *name, char *message)
{
    int fd;

    if ((fd = creat(name, CREAT_PERMS)) < 0 || fd > 255)
	as_abort(message);
    return fd;
}

static void process_args(int argc, char *argv[])
{
    char *arg;
    bool_t isnextarg;
    char *nextarg = 0;
    int opened_file = 0;
    int flag_state;

#ifdef I80386
    setcpu(0xF);
#endif
    textseg = -1;

    if (argc <= 1)
	usage();
    do
    {
	arg = *++argv;
	if (arg[0] == '-' && arg[1] != '\0')
	{
	    flag_state = 1;
	    if (arg[2] == '-' && arg[3] == 0 )
	       flag_state = 0;
	    else
	    if (arg[2] != 0)
		usage();	/* no multiple options */
	    isnextarg = FALSE;
	    if (argc > 2)
	    {
		nextarg = argv[1];
		if (nextarg[0] != 0 && nextarg[0] != '-')
		    isnextarg = TRUE;
	    }
	    switch (arg[1])
	    {
	    case 'v':
	       outfd = STDOUT;
	       writes("as version: ");
#ifdef VERSION
	       writesn(VERSION);
#else
	       writesn("Unknown!");
#endif
	       exit(1);
#ifdef I80386
	    case '0': case '1': case '2':
		idefsize = defsize = 0x2;
		setcpu(arg[1]-'0');
		break;
	    case '3':
		idefsize = defsize = 0x4;
		setcpu(0xF);
		break;
	    case 'a':
		asld_compatible = flag_state;
		break;
#endif
	    case 'b':
		if (!isnextarg || binfil != 0)
		    usage();
		binfil = my_creat(binfilename=nextarg, "error creating binary file");
		binaryg = TRUE;
		--argc;
		++argv;
		break;
	    case 'g':
		globals_only_in_obj = flag_state;
		break;
#ifdef I80386
	    case 'j':
		jumps_long = flag_state;
		break;
	    case 'O':
		if( flag_state ) last_pass = 2;
		else             last_pass = 1;
		break;
#endif
	    case 'l':
		list.global = TRUE;
		goto get_any_list_file;
	    case 'm':
		maclist.global = TRUE;
	get_any_list_file:
		if (isnextarg)
		{
		    if (lstfil != STDOUT)
			usage();
		    lstfil = my_creat(nextarg, "error creating list file");
		    --argc;
		    ++argv;
		}
		break;
	    case 'n':
		if (!isnextarg)
		    usage();
		truefilename = nextarg;
		--argc;
		++argv;
		break;
	    case 'o':
		if (!isnextarg || objfil != 0)
		    usage();
		objectg = TRUE;
		objfil = my_creat(objfilename=nextarg, "error creating object file");
		--argc;
		++argv;
		break;
	    case 's':
		if (!isnextarg || symfil != 0)
		    usage();
		symgen = TRUE;
		symfil = my_creat(nextarg, "error creating symbol file");
		--argc;
		++argv;
		break;
	    case 't':
		if (!isnextarg || binfil != 0)
		    usage();
		textseg = atoi(nextarg); if(textseg>0) textseg+=BSSLOC;
		--argc;
		++argv;
		break;
	    case 'u':
		if( flag_state ) inidata = IMPBIT | SEGM;
		else             inidata = 0;
		break;
	    case 'w':
		if( flag_state ) as_warn.semaphore = -1;
		else             as_warn.semaphore = 0;
		break;
	    case 'k':
		keep_bad_output = 1;
		break;
	    default:
		usage();	/* bad option */
	    }
	}
	else if (infil != 0)
	    usage();		/* no multiple sources */
	else
	{
	    if (strlen(arg) > FILNAMLEN)
		as_abort("source file name too long");
	    infiln = infil0 = 1;
	    infil = open_input(strcpy(filnamptr, arg));
	    opened_file = 1;
	}
    }
    while (--argc != 1);
    if( !opened_file )
    {
       infiln = infil0 = 1;
       infil = open_input(strcpy(filnamptr, "-"));
    }
#ifdef I80386
    origcpuid = cpuid;
#endif
    inidata = (~binaryg & inidata) | (RELBIT | UNDBIT);
}				/* IMPBIT from inidata unless binaryg */

static void summary(int fd)
{
    outfd = fd;
    writenl();
    summ_number(toterr);
    writesn(" errors");
    summ_number(totwarn);
    writesn(" warnings");
}

static void summ_number(unsigned num)
{
    /* format number like line numbers */
    char buf[16];
    *build_number(num, LINUM_LEN, buf) = 0;
    writes(buf);
}

static void usage(void)
{
    as_abort(
#ifdef I80386
"usage: as [-03agjuwO] [-b [bin]] [-lm [list]] [-n name] [-o obj] [-s sym] src");
#else
    "usage: as [-guw] [-b [bin]] [-lm [list]] [-n name] [-o obj] [-s sym] src");
#endif
}
