/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

/*
 *   print file with headings
 *  2+head+2+page[56]+5
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

int	ncol	= 1;
const char	*header;
int	col;
int	icol;
FILE	*file;
char	*bufp;
#define	BUFS	6720
char	buffer[BUFS];	/* for multi-column output */
char	obuf[BUFSIZ];
#define	FF	014
int	line;
char	*colp[72];
int	nofile;
char	isclosed[10];
FILE	*ifile[10];
char	**lastarg;
int	peekc;
int	fpage;
int	page;
int	colw;
int	nspace;
int	width	= 72;
int	length	= 66;
int	plength = 61;
int	margin	= 10;
int	ntflg;
int	mflg;
int	tabc;
char	*tty;
int	mode;


void done(void)
{

	if (tty)
		chmod(tty, mode);
	exit(0);
}

void onintr(int sig)
{

	if (tty)
		chmod(tty, mode);
	_exit(1);
}

void fixtty(void)
{
	struct stat sbuf;

	tty = ttyname(1);
	if (tty == 0)
		return;
	stat(tty, &sbuf);
	mode = sbuf.st_mode&0777;
	chmod(tty, 0600);
}



void nexbuf(void)
{
	register int n;
	register char *rbufp;

	rbufp = bufp;
	n = &buffer[BUFS] - rbufp;
	if (n>512)
		n = 512;
	if(feof(file) ||
	   (n=fread(rbufp,1,n,file)) <= 0){
		fclose(file);
		*rbufp = 0376;
	}
	else {
		rbufp += n;
		if (rbufp >= &buffer[BUFS])
			rbufp = buffer;
		*rbufp = 0375;
	}
	bufp = rbufp;
}

int tpgetc(int ai)
{
	register char **p;
	register int c, i;

	i = ai;
	if (mflg) {
		if((c=getc(ifile[i])) == EOF) {
			if (isclosed[i]==0) {
				isclosed[i] = 1;
				if (--nofile <= 0)
					return(0);
			}
			return('\n');
		}
		if (c==FF && ncol>0)
			c = '\n';
		return(c);
	}
loop:
	c = **(p = &colp[i]) & 0377;
	if (c == 0375) {
		nexbuf();
		c = **p & 0377;
	}
	if (c == 0376)
		return(0);
	(*p)++;
	if (*p >= &buffer[BUFS])
		*p = buffer;
	if (c==0)
		goto loop;
	return(c);
}

int pgetc(int i)
{
	register int c;

	if (peekc) {
		c = peekc;
		peekc = 0;
	} else
		c = tpgetc(i);
	if (tabc)
		return(c);
	switch (c) {

	case '\t':
		icol++;
		if ((icol&07) != 0)
			peekc = '\t';
		return(' ');

	case '\n':
		icol = 0;
		break;

	case 010:
	case 033:
		icol--;
		break;
	}
	if (c >= ' ')
		icol++;
	return(c);
}

void putcp(int c)
{
	if (page >= fpage)
		putchar(c);
}


void put(int ac)
{
	register int ns, c;

	c = ac;
	if (tabc) {
		putcp(c);
		if (c=='\n')
			line++;
		return;
	}
	switch (c) {

	case ' ':
		nspace++;
		col++;
		return;

	case '\n':
		col = 0;
		nspace = 0;
		line++;
		break;

	case 010:
	case 033:
		if (--col<0)
			col = 0;
		if (--nspace<0)
			nspace = 0;

	}
	while(nspace) {
		if (nspace>2 && col > (ns=((col-nspace)|07))) {
			nspace = col-ns-1;
			putcp('\t');
		} else {
			nspace--;
			putcp(' ');
		}
	}
	if (c >= ' ')
		col++;
	putcp(c);
}

void putpage(void)
{
	register int lastcol, i, c;
	int j;

	if (ncol==0) {
		while (line<plength) {
			while((c = tpgetc(0)) && c!='\n' && c!=FF)
				putcp(c);
			putcp('\n');
			line++;
			if (c==FF)
				break;
		}
		return;
	}
	colp[0] = colp[ncol];
	if (mflg==0) for (i=1; i<=ncol; i++) {
		colp[i] = colp[i-1];
		for (j = margin; j<length; j++)
			while((c=tpgetc(i))!='\n')
				if (c==0)
					break;
	}
	while (line<plength) {
		lastcol = colw;
		for (i=0; i<ncol; i++) {
			while ((c=pgetc(i)) && c!='\n')
				if (col<lastcol || tabc!=0)
					put(c);
			if (c==0)
				continue;
			if (tabc)
				put(tabc);
			else while (col<lastcol)
				put(' ');
			lastcol += colw;
		}
		while ((c = pgetc(ncol)) && c!='\n')
			put(c);
		put('\n');
	}
}

void mopen(char **ap)
{
	register char **p, *p1;

	p = ap;
	while((p1 = *p) && p++ <= lastarg) {
		if((ifile[nofile]=fopen(p1, "r")) == NULL){
			isclosed[nofile] = 1;
			nofile--;
		}
		else
			isclosed[nofile] = 0;
		if(++nofile>=10) {
			fprintf(stderr, "pr: Too many args\n");
			done();
		}
	}
}

void print(char *fp, char **argp)
{
	struct stat sbuf;
	register int sncol;
	register const char *sheader;
	register char *cbuf;
	char linebuf[150], *cp;

	if (ntflg)
		margin = 0;
	else
		margin = 10;
	if (length <= margin)
		length = 66;
	if (width <= 0)
		width = 72;
	if (ncol>72 || ncol>width) {
		fprintf(stderr, "pr: No room for columns.\n");
		done();
	}
	if (mflg) {
		mopen(argp);
		ncol = nofile;
	}
	colw = width/ncol;
	sncol = ncol;
	sheader = header;
	plength = length-5;
	if (ntflg)
		plength = length;
	if (--ncol<0)
		ncol = 0;
	if (mflg)
		fp = 0;
	if (fp) {
		if((file=fopen(fp, "r"))==NULL) {
			if (tty==NULL)
				fprintf(stderr, "pr: can't open %s\n", fp);
			ncol = sncol;
			header = sheader;
			return;
		}
		stat(fp, &sbuf);
	} else {
		file = stdin;
		time(&sbuf.st_mtime);
	}
	if (header == 0)
		header = fp?fp:"";
	cbuf = ctime(&sbuf.st_mtime);
	cbuf[16] = '\0';
	cbuf[24] = '\0';
	page = 1;
	icol = 0;
	colp[ncol] = bufp = buffer;
	if (mflg==0)
		nexbuf();
	while (mflg&&nofile || (!mflg)&&tpgetc(ncol)>0) {
		if (mflg==0) {
			colp[ncol]--;
			if (colp[ncol] < buffer)
				colp[ncol] = &buffer[BUFS];
		}
		line = 0;
		if (ntflg==0) {
			snprintf(linebuf, 150, "\n\n%s %s  %s Page %d\n\n\n",
				cbuf+4, cbuf+20, header, page);
			for(cp=linebuf;*cp;) put(*cp++);
		}
		putpage();
		if (ntflg==0)
			while(line<length)
				put('\n');
		page++;
	}
	fclose(file);
	ncol = sncol;
	header = sheader;
}

int main(int argc, char *argv[])
{
	int nfdone;

	setbuf(stdout, obuf);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, onintr);
	lastarg = &argv[argc-1];
	fixtty();
	for (nfdone=0; argc>1; argc--) {
		argv++;
		if (**argv == '-') {
			switch (*++*argv) {
			case 'h':
				if (argc>=2) {
					header = *++argv;
					argc--;
				}
				continue;

			case 't':
				ntflg++;
				continue;

			case 'l':
				length = atoi(++*argv);
				continue;

			case 'w':
				width = atoi(++*argv);
				continue;

			case 's':
				if (*++*argv)
					tabc = **argv;
				else
					tabc = '\t';
				continue;

			case 'm':
				mflg++;
				continue;

			default:
				ncol = atoi(*argv);
				continue;
			}
		} else if (**argv == '+') {
			fpage = atoi(++*argv);
		} else {
			print(*argv, argv);
			nfdone++;
			if (mflg)
				break;
		}
	}
	if (nfdone==0)
		print((char *)0, (char **)0);
	done();
}
