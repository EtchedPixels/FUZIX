/*
 * LEVEE, or Captain Video;  A vi clone
 *
 * Copyright (c) 1982-1997 David L Parsons
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by David L Parsons (orc@pell.chi.il.us).  My name may not be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.  THIS SOFTWARE IS PROVIDED
 * AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "levee.h"
#include "extern.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/*
 * do a newline and set flags.
 */
#define exprintln()	(zotscreen=YES),println()

PROC
plural(int num, char *string)
{
    printi(num);
    prints(string);
    if (num != 1)
	printch('s');
} /* plural */


PROC
clrmsg(void)
{
    mvcur(-1,0);
    strput(CE);
} /* clrmsg */


PROC
errmsg(char *msg)
{
    mvcur(-1,0);
    prints(msg);
    strput(CE);
} /* errmsg */


/* get a space-delimited token */
char *execstr;			/* if someone calls getarg in the	*/
				/* wrong place, death will come...	*/
char *PROC
getarg(void)
{
    char *rv;
    rv = execstr;
    while (*execstr && !isspace(*execstr))
	++execstr;
    if (*execstr) {
	*execstr++ = 0;
	while (isspace(*execstr))
	    ++execstr;
    }
    return (*rv) ? rv : NULL;
} /* getarg */


PROC
version()
/* version: print which version of levee we are... */
{
    errmsg("levee ");prints(ED_NOTICE);printch(ED_REVISION);
} /* version */


PROC
args(void)
/* args: print the argument list */
{
    register i;
    mvcur(-1,0);
    for (i=0; i < argc; i++) {
	if (curpos.x+strlen(argv[i]) >= COLS)
	    exprintln();
	else if (i > 0)
	    printch(' ');
	if (pc == i) {			/* highlight the current filename.. */
#if ST|FLEXOS
	    strput("\033p");
#else
	    printch('[');
#endif
	    prints(argv[i]);
#if ST|FLEXOS
	    strput("\033q");
#else
	    printch(']');
#endif
	}
	else
	    prints(argv[i]);
    }
} /* args */
    
PROC
setcmd(void)
{
    bool no,b;
    int len,i;
    register char *arg;
    char *num;
    register struct variable *vp;
    
    if (arg = getarg()) {
	do {
	    if (*arg != 0) {
		if (num = strchr(arg, '=')) {
		    b = NO;
		    *num++ = 0;
		}
		else {				/* set [no]opt */
		    b = YES;
		    if (arg[0]=='n' && arg[1]=='o') {
			arg += 2;
			no = NO;
		    }
		    else
			no = YES;
		}
		for(vp=vars;vp->u && strcmp(arg,vp->v_name)
				  && strcmp(arg,vp->v_abbr); vp++)
		    ;
		if (!vp->u || vp->v_flags & V_CONST) {
		    errmsg("Can't set ");
		    prints(arg);
		}
		else {
		    int j;
                    
                    if (b)
			if (vp->v_tipe == VBOOL)
			    vp->u->valu = no;
			else
			    goto badsettype;
		    else if (vp->v_tipe == VSTR) {
			if (vp->u->strp)
			    free(vp->u->strp);
			vp->u->strp = (*num) ? strdup(num) : NULL;
		    }
		    else
			if (*num && (j=atoi(num)) >= 0)
			    vp->u->valu = j;
			else {
		  badsettype:
			    errmsg("bad set type");
			    continue;
			}
		    diddled |= vp->v_flags & V_DISPLAY;
		}
	    }
	} while (arg = getarg());
    }
    else {
	version(); exprintln();
	for(vp=vars;vp->u;vp++) {
	    switch (vp->v_tipe) {
		case VBOOL:
		    if (!vp->u->valu)
			prints("no");
		    prints(vp->v_name);
		    break;
		case VSTR:
		    if (!vp->u->strp)
			prints("no ");
		    prints(vp->v_name);
		    if (vp->u->strp) {
			mvcur(-1,10);
			prints("= ");
			prints(vp->u->strp);
		    }
		    break;
		default:
		    prints(vp->v_name);
		    mvcur(-1,10);
		    prints("= ");
		    printi(vp->u->valu);
		    break;
	    }
	    exprintln();
	}
    }
} /* setcmd */


/* print a macro */
PROC
printone(int i)
{
    if (i >= 0) {
	exprintln();
	printch(mbuffer[i].token);
	mvcur(-1,3);
	if (movemap[mbuffer[i].token] == INSMACRO)
	    prints("!= ");
	else
	    prints(" = ");
	prints(mbuffer[i].m_text);
    }
} /* printone */


/* print all the macros */
PROC
printall(void)
{
    int i;
    for (i = 0; i < MAXMACROS; i++)
	if (mbuffer[i].token != 0)
	    printone(i);
} /* printall */


/* :map ch text */
PROC
map(bool insert)
{
    register char *macro, c;
    int i;
    		/* get the macro */
    if ((macro=getarg()) == NULL) {
	printall();
	return 1;
    }
    if (strlen(macro) > 1) {
	errmsg("macros must be one character");
	return 0;
    }
    c = macro[0];
    if (*execstr == 0)
	printone(lookup(c));
    else {
	if ((i = lookup(0)) < 0)
	    errmsg("macro table full");
	else if (c == ESC || c == ':') {
	    errmsg("can't map ");
	    printch(c);
	}
	else if (*execstr != 0) {
	    undefine(lookup(c));
	    mbuffer[i].token = c;
	    mbuffer[i].m_text = strdup(execstr);
	    mbuffer[i].oldmap = movemap[c];
	    if (insert)
		movemap[c] = INSMACRO;
	    else
		movemap[c] = SOFTMACRO;
	}
    }
} /* map */


PROC
undefine(int i)
{
    register char *p;
    if (i >= 0) {
	movemap[mbuffer[i].token] = mbuffer[i].oldmap;
	mbuffer[i].token = 0;
	p = mbuffer[i].m_text;
	free(p);
	mbuffer[i].m_text = 0;
    }
} /* undefine */


PROC
unmap(void)
{
    int i;
    register char *arg;
    
    if (arg=getarg()) {
	if (strlen(arg) == 1) {
	    undefine(lookup(*arg));
	    return YES;
	}
	if (strcmp(arg,"all") == 0) {
	    for (i = 0; i < MAXMACROS; i++)
		if (mbuffer[i].token != 0)
			undefine(i);
	    return YES;
	}
    }
    return NO;
} /* unmap */


/* return argument # of a filename */
int PROC
findarg(char *name)
{
    int j;
    for (j = 0; j < argc; j++)
	if (strcmp(argv[j],name) == 0)
	    return j;
    return -1;
} /* findarg */


/* add a filename to the arglist */
int PROC
addarg(char *name)
{
    int where;
    if ((where = findarg(name)) < 0)
	return doaddwork(name, &argc, &argv);
    return where;
} /* addarg */


/* get a file name argument (substitute alt file for #) */
char * PROC
getname(void)
{
    extern int wilderr;
#if ST
    extern int mapslash;
    register char *p;
#endif
    register char *name;
    if (name = getarg()) {
	if (strcmp(name,"#") == 0)
	    if (*altnm)
		name = altnm;
	    else {
		errmsg("no alt name");
		wilderr++;
		return NULL;
	    }
#if ST
	if (mapslash)
	    for (p=name; *p; p++)
		if (*p == '/')
		    *p = '\\';
#endif
    }
    return name;
} /* getname */


/* CAUTION: these make exec not quite recursive */
int  high,low;		/* low && high end of command range */
bool affirm;		/* cmd! */
/* s/[src]/dst[/options] */
/* s& */
PROC
cutandpaste(void)
{
    bool askme  = NO,
	 printme= NO,
	 glob   = NO;
    int newcurr = -1;
    int oldcurr = curr;
    int  num;
    char delim;
    register char *ip;
    register char *dp;
    
    zerostack(&undo);
    ip = execstr;
    if (*ip != '&') {
	delim = *ip;
	ip = makepat(1+ip,delim);			/* get search */
	if (ip == NULL)
	    goto splat;
	dp = dst;
	while (*ip && *ip != delim) {
	    if (*ip == '\\' && ip[1] != 0)
		*dp++ = *ip++;
	    *dp++ = *ip++;
	}
	*dp = 0;
	if (*ip == delim) {
	    while (*++ip)
		switch (*ip) {
		    case 'q':
		    case 'c': askme = YES;	break;
		    case 'g': glob = YES;	break;
		    case 'p': printme= YES;	break;
		}
	}
    }
    if (*lastpatt == 0) {
splat:	errmsg("bad substitute");
	return 0;
    }
    fixupline(bseekeol(curr));
    num = 0;
    do {
	low = chop(low, &high, NO, &askme);
	if (low > -1) {
	    diddled = YES;
	    num++;
	    if (printme) {
		exprintln();
		writeline(-1,-1,bseekeol(low));
	    }
	    if (newcurr < 0)
		newcurr = low;
	    if (!glob)
		low = 1+fseekeol(low);
	}
    } while (low >= 0);
    if (num > 0) {
	exprintln();
	plural(num," substitution");
    }
    fixupline((newcurr > -1) ? newcurr : oldcurr);
} /* cutandpaste */


PROC
inputf(char *fname, bool newbuf)
{
    int onright,	/* text moved right for :read */
	fsize;		/* bytes read in */
    register FILE *f;


    if (newbuf)
	readonly = NO;

    zerostack(&undo);
    if (newbuf) {
	modified = NO;
	low = 0;
	high = SIZE;
    }
    else {		/* append stuff to the buffer */
	fixupline(bseekeol(curr));
	onright = bufmax-low;
#if MSDOS
	high = SIZE;
	high -= onright;
#else
	high = (SIZE-onright);
#endif
	if (onright > 0)
	    moveright(&core[low], &core[high], onright);
    }
    printch('"');
    prints(fname);
    prints("\" ");
    if ((f=fopen(fname, "r")) == NULL) {
	prints("[No such file]");
	fsize = 0;
	if (newbuf)
	    newfile = YES;
    }
    else {
	if (addfile(f, low, high, &fsize))
	    plural(fsize," byte");
	else if (fsize < 0) {
	    prints("[read error]");
	    fsize = 0;
	}
	else {
	    prints("[overflow]");
	    readonly = YES;
	}
	fclose(f);
	if (newbuf)
	    newfile = NO;
    }
    if (newbuf) {
	fillchar(contexts, sizeof(contexts), -1);
	bufmax = fsize;
    }
    else {
	insert_to_undo(&undo, low, fsize);
	modified = YES;
	if (onright > 0)
	    moveleft(&core[high], &core[low+fsize], onright);
    }
    if (*startcmd) {
	count = 1;
	if (*findparse(startcmd,&curr,low) != 0 || curr < 0)
	    curr = low;
	*startcmd = 0;
    }
    else
	curr = low;
    diddled = YES;
} /* inputf */


/* Change a file's name (for autocopy). */
PROC
backup(char *name)
{
    char back[80];
    char *p;

    strcpy(back, name);
#if UNIX
    strcat(back, "~");
#else
    p = strrchr(basename(back), '.');
    if (p)
	strcpy(1+p, ",bkp");
    else
	strcat(back, ".bkp");
#endif
    
    unlink(back);
    rename(name, back);
} /* backup */


bool PROC
outputf(char *fname)
{
    bool whole;
    register FILE *f;
    int status;
    zerostack(&undo);		/* undo doesn't survive past write */
    if (high < 0)
	high = (low < 0) ? bufmax : (1+fseekeol(low));
    if (low < 0)
	low  = 0;
    printch('"');
    prints(fname);
    prints("\" ");
    whole = (low == 0 && high >= bufmax-1);
    if (whole && autocopy)
	backup(fname);
    if (f=fopen(fname, "w")) {
	status = putfile(f, low, high);
	fclose(f);
	if (status) {
	    plural(high-low," byte");
	    if (whole)
		modified = NO;
	    return(YES);
	}
	else {
	    prints("[write error]");
	    unlink(fname);
	}
    }
    else
	prints(fisro);
    return(NO);
} /* outputf */


PROC
oktoedit(int writeold)
/* check and see if it is ok to edit a new file */
/* writeold;	automatically write out changes? */
{
    if (modified && !affirm)
	if (readonly) {
	    errmsg(fisro);
	    return NO;
	}
	else if (writeold && *filenm) {
	    if (!outputf(filenm))
		return NO;
	    printch(',');
	}
	else {
	    errmsg(fismod);
	    return NO;
	}
    return YES;
} /* oktoedit */


/* write out all || part of a file */
bool PROC
writefile(void)
{
    register char *name;
    
    if ((name=getname()) == NULL)
	name = filenm;
    if (*name) {
	if (outputf(name)) {
	    addarg(name);
	    return YES;
	}
	else
	    strcpy(altnm, name);
    }
    else
	errmsg("no file to write");
    return NO;
}


PROC
editfile(void)
{
    register char *name = NULL;	/* file to edit */
    char **myargv;
    int myargc;
    int i, newpc;
    if ((name = getarg()) && *name == '+') {
	strcpy(startcmd, (name[1])?(1+name):"$");
	name = getarg();
    }
    myargc=0;
    if (name)
	do {
	    if (!expandargs(name, &myargc, &myargv))
		return 0;
	} while (name=getarg());
    if (myargc == 0) {
	if (*filenm)
	    name = filenm;
	else
	    errmsg("no file to edit");
    }
    else if ((newpc = addarg(myargv[0])) >= 0) {
	name = argv[pc = newpc];
	for (i=1; i < myargc && doaddwork(myargv[i], &argc, &argv) >= 0; i++)
	    ;
    }
    killargs(&myargc, &myargv);
    if (name && oktoedit(NO))
	doinput(name);
}


PROC
doinput(char *name)
{
    inputf(name, YES);
    strcpy(altnm, filenm);
    strcpy(filenm, name);
}


PROC
toedit(int count)
{
    if (count > 1) {
	printi(count);
	prints(" files to edit; ");
    }
}


PROC
readfile(void)
{
    register char *name;
    
    if (name=getarg())
	inputf(name,NO);
    else
	errmsg("no file to read");
}


PROC
nextfile(bool prev)
{
    register char *name = NULL;
    int newpc=pc,
	what,
	myargc=0;
    char **myargv;
    bool newlist = NO;
    
    if (prev == 0)
	while (name=getarg())
	    if (!expandargs(name, &myargc, &myargv))
		return 0;
    
    if (oktoedit(autowrite)) {
	if (prev || (myargc == 1 && strcmp(myargv[0],"-") == 0)) {
	    if (pc > 0) {
		newpc = pc-1;
	    }
	    else {
		prints("(no prev files)");
		goto killem;
	    }
	}
	else if (myargc == 0) {
	    if (pc < argc-1) {
		newpc = 1+pc;
	    }
	    else {
		prints("(no more files)");
		goto killem;
	    }
	}
	else if (myargc > 1 || (newpc = findarg(myargv[0])) < 0) {
	    toedit(myargc);
	    newpc = 0;
	    newlist++;
	}
	doinput(newlist ? myargv[0] : argv[newpc]);
	pc = newpc;
	if (newlist) {
	    killargs(&argc, &argv);
	    argc = myargc;
	    argv = myargv;
	}
	else
    killem: if (!prev)
		killargs(&myargc, &myargv);
    }
}


/*
 * set up low, high; set dot to low
 */
PROC
fixupline(int dft)
{
    if (low < 0)
	low = dft;
    if (high < 0)
	high = fseekeol(low)+1;
    else if (high < low) {		/* flip high & low */
	int tmp;
	tmp = high;
	high = low;
	low = tmp;
    }
    if (low >= ptop && low < pend) {
	setpos(skipws(low));
	yp = setY(curr);
    }
    else {
	curr = low;
	diddled = YES;
    }
}


PROC
whatline(void)
{
    printi(to_line((low < 0) ? (bufmax-1) : low));
    if (high >= 0) {
	printch(',');
	printi(to_line(high));
    }
}


PROC
print(void)
{
    do {
	exprintln();
	writeline(-1, 0, low);
	low = fseekeol(low) + 1;
    } while (low < high);
    exprintln();
}

/* move to different line */
/* execute lines from a :sourced || .lvrc file */


bool PROC
do_file(char *fname, exec_type *mode, bool *noquit)
{
    char line[120];
    register FILE *fp;
    
    if ((fp = fopen(fname,"r")) != NULL) {
	indirect = YES;
	while (fgets(line,120,fp) && indirect) {
	    strtok(line, "\n");
	    if (*line != 0)
		exec(line,mode,noquit);
	}
	indirect = YES;
	fclose(fp);
	return YES;
    }
    return NO;
}


PROC doins(bool flag)
{
    int i;
    curr = low;
    exprintln();
    low = insertion(1,setstep[flag],&i,&i,NO)-1;
    if (low >= 0)
	curr = low;
    diddled = YES;
}


/* figure out a address range for a command */
char * PROC findbounds(char *ip)
{
    ip = findparse(ip, &low, curr);	/* get the low address */
    if (low >= 0) {
	low = bseekeol(low);		/* at start of line */
	if (*ip == ',') {		/* high address? */
	    ip++;
	    count = 0;
	    ip = findparse(ip, &high, curr);
	    if (high >= 0) {
		high = fseekeol(high);
		return(ip);
	    }
	}
	else
	    return(ip);
    }
    return(0);
}


/* parse the command line for lineranges && a command */
PROC parse(char *inp)
{
    int j,k;
    char cmd[80];
    low = high = ERR;
    affirm = 0;
    if (*inp == '%') {
	moveright(inp, 2+inp, 1+strlen(inp));
	inp[0]='1';
	inp[1]=',';
	inp[2]='$';
    }
    while (isspace(*inp))
	++inp;
    if (strchr(".$-+0123456789?/`'", *inp))
	if (!(inp=findbounds(inp))) {
	    errmsg("bad address");
	    return ERR;
	}
    while (isspace(*inp))
	++inp;
    j = 0;
    while (isalpha(*inp))
	cmd[j++] = *inp++;
    if (*inp == '!') {
	if (j == 0)
	    cmd[j++] = '!';
	else
	    affirm++;
	inp++;
    }
    else if (*inp == '=' && j == 0)
	cmd[j++] = '=';
    while (isspace(*inp))
	++inp;
    execstr = inp;
    if (j==0)
	return EX_CR;
    for (k=0; excmds[k]; k++)
	if (strncmp(cmd, excmds[k], j) == 0)
	    return k;
    return ERR;
}


/* inner loop of execmode */
PROC
exec(char *cmd, exec_type *mode, bool *noquit)
{
    int  what;
    bool ok;
    
    what = parse(cmd);
    ok = YES;
    if (diddled) {
	lstart = bseekeol(curr);
	lend = fseekeol(curr);
    }
    switch (what) {
	case EX_QUIT:				/* :quit */
	    if (affirm || what == lastexec || !modified)
		*noquit = NO;
	    else
		errmsg(fismod);
	    break;
	case EX_READ:				/* :read */
	    clrmsg();
	    readfile();
	    break;
	case EX_EDIT:				/* :read, :edit */
	    clrmsg();
	    editfile();
	    break;
	case EX_WRITE:
	case EX_WQ :				/* :write, :wq */
	    clrmsg();
	    if (readonly && !affirm)
		prints(fisro);
	    else if (writefile() && what==EX_WQ)
		*noquit = NO;
	    break;
	case EX_PREV:
	case EX_NEXT:				/* :next */
	    clrmsg();
	    nextfile(what==EX_PREV);
	    break;
	case EX_SUBS:				/* :substitute */
	    cutandpaste();
	    break;
	case EX_SOURCE:				/* :source */
	    if ((cmd = getarg()) && !do_file(cmd, mode, noquit)) {
		errmsg("cannot open ");
		prints(cmd);
	    }
	    break;
	case EX_XIT:
	    clrmsg();
	    if (modified)
		if (readonly) {
		    prints(fisro);
		    break;
		}
		else if (!writefile())
		    break;

	    if (!affirm && (argc-pc > 1)) {	/* any more files to edit? */
		printch('(');
		plural(argc-pc-1," more file");
		prints(" to edit)");
	    }
	    else
		*noquit = NO;
	    break;
	case EX_MAP:
	    map(affirm);
	    break;
	case EX_UNMAP:
	    ok = unmap();
	    break;
	case EX_FILE:				/* :file */
	    if (cmd=getarg()) {		/* :file name */
		strcpy(altnm, filenm);
		strcpy(filenm, cmd);
		pc = addarg(filenm);
	    }
	    wr_stat();
	    break;
	case EX_SET:				/* :set */
	    setcmd();
	    break;
	case EX_CR:
	case EX_PR:				/* :print */
	    fixupline(bseekeol(curr));
	    if (what == EX_PR)
		print();
	    break;
	case EX_LINE:				/* := */
	    whatline();
	    break;
	case EX_DELETE:
	case EX_YANK:				/* :delete, :yank */
	    yank.lines = YES;
	    fixupline(lstart);
	    zerostack(&undo);
	    if (what == EX_DELETE)
		ok = deletion(low,high);
	    else
		ok = doyank(low,high);
	    diddled = YES;
	    break;
	case EX_PUT:				/* :put */
	    fixupline(lstart);
	    zerostack(&undo);
	    ok = putback(low, &high);
	    diddled = YES;
	    break;
	case EX_VI:				/* :visual */
	    *mode = E_VISUAL;
	    if (*execstr) {
		clrmsg();
		nextfile(NO);
	    }
	    break;
	case EX_EX:
	    *mode = E_EDIT;			/* :execmode */
	    break;
	case EX_INSERT:
	case EX_OPEN:				/* :insert, :open */
	    if (indirect)
		ok = NO;		/* kludge, kludge, kludge!!!!!!!!!! */
	    else {
		zerostack(&undo);
		fixupline(lstart);
		doins(what == EX_OPEN);
	    }
	    break;
	case EX_CHANGE:				/* :change */
	    if (indirect)
		ok = NO;		/* kludge, kludge, kludge!!!!!!!!!! */
	    else {
		zerostack(&undo);
		yank.lines = YES;
		fixupline(lstart);
		if (deletion(low,high))
		    doins(YES);
		else
		    ok = NO;
	    }
	    break;
	case EX_UNDO:				/* :undo */
	    low = fixcore(&high);
	    if (low >= 0) {
		diddled = YES;
		curr = low;
	    }
	    else ok = NO;
	    break;
	case EX_ARGS:				/* :args */
	    args();
	    break;
	case EX_VERSION:			/* version */
	    version();
	    break;
	case EX_ESCAPE:			/* shell escape hack */
	    zotscreen = YES;
	    exprintln();
	    if (*execstr) {
#if ZTERM
		zclose();
#endif
#if FLEXOS|UNIX
		fixcon();
#else
		allowintr();
#endif
//FIXME!!		system(execstr);
#if FLEXOS|UNIX
		initcon();
#else
		nointr();
#endif
	    }
	    else
		prints("incomplete shell escape.");
	    break;
	case EX_REWIND:
	    clrmsg();
	    if (argc > 0 && oktoedit(autowrite)) {
		pc = 0;
		doinput(argv[0]);
	    }
	    break;
	default:
	    prints(":not an editor command.");
	    break;
    }
    lastexec = what;
    if (!ok) {
	errmsg(excmds[what]);
	prints(" error");
    }
}
