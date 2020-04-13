/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* ANSIfied for FUZIX */

/*
 *	test expression
 *	[ expression ]
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#define EQ(a,b)	((tmp=a)==0?0:(strcmp(tmp,b)==0))

#define DIR 1
#define FIL 2
int ap;
int ac;
const char **av;
const char *tmp;

void synbad(const char *s1, const char *s2)
{
	write(2, "test: ", 6);
	write(2, s1, strlen(s1));
	write(2, s2, strlen(s2));
	write(2, "\n", 1);
	exit(255);
}

const char *nxtarg(int mt)
{
	if (ap >= ac) {
		if (mt) {
			ap++;
			return (0);
		}
		synbad("argument expected", "");
	}
	return (av[ap++]);
}

int tio(const char *a, int f)
{
	/* FIXME: should use stat and parse permissions */
	f = open(a, f);
	if (f >= 0) {
		close(f);
		return (1);
	}
	return (0);
}

int ftype(const char *f)
{
	struct stat statb;

	if (stat(f, &statb) < 0)
		return (0);
	if ((statb.st_mode & S_IFMT) == S_IFDIR)
		return (DIR);
	return (FIL);
}

int fsizep(const char *f)
{
	struct stat statb;
	if (stat(f, &statb) < 0)
		return (0);
	return (statb.st_size > 0);
}


int e3(void)
{
	int p1;
	register const char *a;
	const char *p2;
	int int1, int2;

	extern int expr(void);

	a = nxtarg(0);
	if (EQ(a, "(")) {
		p1 = expr();
		if (!EQ(nxtarg(0), ")"))
			synbad(") expected", "");
		return (p1);
	}

	if (EQ(a, "-r"))
		return (tio(nxtarg(0), 0));

	if (EQ(a, "-w"))
		return (tio(nxtarg(0), 1));

	if (EQ(a, "-d"))
		return (ftype(nxtarg(0)) == DIR);

	if (EQ(a, "-f"))
		return (ftype(nxtarg(0)) == FIL);

	if (EQ(a, "-s"))
		return (fsizep(nxtarg(0)));

	if (EQ(a, "-t"))
		if (ap >= ac)
			return (isatty(1));
		else
			return (isatty(atoi(nxtarg(0))));

	if (EQ(a, "-n"))
		return (!EQ(nxtarg(0), ""));
	if (EQ(a, "-z"))
		return (EQ(nxtarg(0), ""));

	p2 = nxtarg(1);
	if (p2 == 0)
		return (!EQ(a, ""));
	if (EQ(p2, "="))
		return (EQ(nxtarg(0), a));

	if (EQ(p2, "!="))
		return (!EQ(nxtarg(0), a));

	if (EQ(a, "-l")) {
		int1 = strlen(p2);
		p2 = nxtarg(0);
	} else {
		int1 = atoi(a);
	}
	int2 = atoi(nxtarg(0));
	if (EQ(p2, "-eq"))
		return (int1 == int2);
	if (EQ(p2, "-ne"))
		return (int1 != int2);
	if (EQ(p2, "-gt"))
		return (int1 > int2);
	if (EQ(p2, "-lt"))
		return (int1 < int2);
	if (EQ(p2, "-ge"))
		return (int1 >= int2);
	if (EQ(p2, "-le"))
		return (int1 <= int2);

	synbad("unknown operator ", p2);
}

int e2(void)
{
	if (EQ(nxtarg(0), "!"))
		return (!e3());
	ap--;
	return (e3());
}

int e1(void)
{
	int p1;

	p1 = e2();
	if (EQ(nxtarg(1), "-a"))
		return (p1 & e1());
	ap--;
	return (p1);
}

int expr(void)
{
	int p1;

	p1 = e1();
	if (EQ(nxtarg(1), "-o"))
		return (p1 | expr());
	ap--;
	return (p1);
}


int main(int argc, const char *argv[])
{

	ac = argc;
	av = argv;
	ap = 1;
	if (EQ(argv[0], "[")) {
		if (!EQ(argv[--ac], "]"))
			synbad("] missing", "");
	}
	argv[ac] = 0;
	if (ac <= 1)
		exit(1);
	exit(expr()? 0 : 1);
}
