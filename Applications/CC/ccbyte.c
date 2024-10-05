/*
 *	It's easiest to think of what cc does as a sequence of four
 *	conversions. Each conversion produces the inputs to the next step
 *	and the number of types is reduced. If the step is the final
 *	step for the conversion then the file is generated with the expected
 *	name but if it will be consumed by a later stage it is a temporary
 *	scratch file.
 *
 *	Stage 1: (-c -o overrides object name)
 *
 *	Ending			Action
 *	$1.S			preprocessor - may make $1.s
 *	$1.s			nothing
 *	$1.c			preprocessor, no-op or /dev/tty
 *	$1.o			nothing
 *	$1.a			nothing (library)
 *
 *	Stage 2: (not -E)
 *
 *	Ending			Action
 *	$1.s			nothing
 *	$1.%			cc, opt - make $1.s
 *	$1.o			nothing
 *	$1.a			nothing (library)
 *
 *	Stage 3: (not -E or -S)
 *
 *	Ending			Action
 *	$1.s			assembler - makes $1.o
 *	$1.o			nothing
 *	$1.a			nothing (library)
 *
 *	Stage 4: (run if no -c -E -S)
 *
 *	ld [each .o|.a in order] [each -l lib in order] -lc
 *	(with -b -o $1 etc)
 *
 *	TODO:
 *
 *	Platform specifics
 *	Search library paths for libraries (or pass to ld and make ld do it)
 *	Turn on temp removal once confident
 *	Split I/D
 */

#undef DEBUG

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define BINPATH		"/opt/ccbyte/bin/"
#define LIBPATH		"/opt/ccbyte/lib/"
#define INCPATH		"/opt/ccbyte/include/"

#define CMD_AS		BINPATH"asbyte"
#define CMD_CC0		LIBPATH"cc0"
#define CMD_CC1		LIBPATH"cc1.byte"
#define CMD_CC2		LIBPATH"cc2.byte"
#define CMD_COPT	LIBPATH"copt"
#define CMD_CPP		LIBPATH"cpp"
#define CMD_LD		BINPATH"ldbyte"
#define CRT0		LIBPATH"crt0.o"
#define LIBC		LIBPATH"libc.a"
#define LIBCPU		LIBPATH"libbyte.a"
#define COPTRULES	LIBPATH"rules.byte"

struct obj {
	struct obj *next;
	char *name;
	uint8_t type;
#define TYPE_S			1
#define TYPE_C			2
#define TYPE_s			3
#define TYPE_C_pp		4
#define TYPE_O			5
#define TYPE_A			6
	uint8_t used;
};

struct objhead {
	struct obj *head;
	struct obj *tail;
};

struct objhead objlist;
struct objhead liblist;
struct objhead inclist;
struct objhead deflist;
struct objhead libpathlist;
struct objhead ccargs;		/* Arguments to pass on to the compiler */

int keep_temp;
int last_phase = 4;
int only_one_input;
char *target;
int strip;
int c_files;
int standalone;
char *cpu = "0";
int mapfile;
/* TODO: OS_FUZIX won't work until ld is taught about literal segments */
#define OS_NONE		0
#define OS_FUZIX	1
int targetos = OS_NONE;
int fuzixsub;
char optimize = '0';
char *codeseg;

#define MAXARG	512

int arginfd, argoutfd;
char *arglist[MAXARG];
char **argptr;
char *rmlist[MAXARG];
char **rmptr = rmlist;

static void remove_temporaries(void)
{
	char **p = rmlist;
	while (p < rmptr) {
		if (keep_temp == 0)
			unlink(*p);
		free(*p++);
	}
	rmptr = rmlist;
}

static void fatal(void)
{
	remove_temporaries();
	exit(1);
}

static void memory(void)
{
	fprintf(stderr, "cc: out of memory.\n");
	fatal();
}

static char *xstrdup(char *p, int extra)
{
	char *n = malloc(strlen(p) + extra + 1);
	if (n == NULL)
		memory();
	strcpy(n, p);
	return n;
}

static void append_obj(struct objhead *h, char *p, uint8_t type)
{
	struct obj *o = malloc(sizeof(struct obj));
	if (o == NULL)
		memory();
	o->name = p;
	o->next = NULL;
	o->used = 0;
	o->type = type;
	if (h->tail)
		h->tail->next = o;
	else
		h->head = o;
	h->tail = o;
}

static char *pathmod(char *p, char *f, char *t, int rmif)
{
	char *x = strrchr(p, '.');
	if (x == NULL) {
		fprintf(stderr, "cc: no extension on '%s'.\n", p);
		fatal();
	}
//	if (strcmp(x, f)) {
//		fprintf(stderr, "cc: internal got '%s' expected '%s'.\n",
//			p, t);
//		fatal();
//	}
	strcpy(x, t);
	if (last_phase > rmif) {
		*rmptr++ = xstrdup(p, 0);
	}
	return p;
}

static void add_argument(char *p)
{
	if (argptr == &arglist[MAXARG]) {
		fprintf(stderr, "cc: too many arguments to command.\n");
		fatal();
	}
	*argptr++ = p;
}

static void add_argument_list(char *header, struct objhead *h)
{
	struct obj *i = h->head;
	while (i) {
		if (header)
			add_argument(header);
		add_argument(i->name);
		i->used = 1;
		i = i->next;
	}
}

static char *resolve_library(char *p)
{
	static char buf[512];
	struct obj *o = libpathlist.head;
	if (strchr(p, '/') || strchr(p, '.'))
		return p;
	while(o) {
		snprintf(buf, 512, "%s/lib%s.a", o->name, p);
		if (access(buf, 0) == 0)
			return xstrdup(buf, 0);
		o = o->next;
	}
	return NULL;
}

/* This turns -L/opt/foo/lib  -lfoo -lbar into resolved names like
   /opt/foo/lib/libfoo.a */
static void resolve_libraries(void)
{
	struct obj *o = liblist.head;
	while(o != NULL) {
		char *p = resolve_library(o->name);
		if (p == NULL) {
			fprintf(stderr, "cc: unable to find library '%s'.\n", o->name);
			exit(1);
		}
		add_argument(p);
		o = o->next;
	}
}

static void run_command(void)
{
	pid_t pid, p;
	int status;

	fflush(stdout);

	*argptr = NULL;

	pid = fork();
	if (pid == -1) {
		perror("fork");
		fatal();
	}
	if (pid == 0) {
#ifdef DEBUG
		{
			char **p = arglist;
			printf("[");
			while(*p)
				printf("%s ", *p++);
			printf("]\n");
		}
#endif
		fflush(stdout);
		if (arginfd != -1) {
			dup2(arginfd, 0);
			close(arginfd);
		}
		if (argoutfd != -1) {
			dup2(argoutfd, 1);
			close(argoutfd);
		}
		execv(arglist[0], arglist);
		perror("execv");
		exit(255);
	}
	if (arginfd)
		close(arginfd);
	if (argoutfd)
		close(argoutfd);
	while ((p = waitpid(pid, &status, 0)) != pid) {
		if (p == -1) {
			perror("waitpid");
			fatal();
		}
	}
	if (WIFSIGNALED(status)) {
		/* Scream loudly if it exploded */
		fprintf(stderr, "cc: %s failed with signal %d.\n", arglist[0],
			WTERMSIG(status));
		fatal();
	}
	/* Quietly exit if the stage errors. That means it has reported
	   things to the user */
	if (WEXITSTATUS(status))
		fatal();
}

static void redirect_in(const char *p)
{
	arginfd = open(p, O_RDONLY);
	if (arginfd == -1) {
		perror(p);
		fatal();
	}
#ifdef DEBUG
	printf("<%s\n", p);
#endif
}

static void redirect_out(const char *p)
{
	argoutfd = open(p, O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (argoutfd == -1) {
		perror(p);
		fatal();
	}
#ifdef DEBUG
	printf(">%s\n", p);
#endif
}

static void build_arglist(char *p)
{
	arginfd = -1;
	argoutfd = -1;
	argptr = arglist;
	add_argument(p);
}

void convert_s_to_o(char *path)
{
	build_arglist(CMD_AS);
	add_argument(path);
	run_command();
	pathmod(path, ".s", ".o", 5);
}

void convert_c_to_s(char *path)
{
	char *tmp, *t;
	char optstr[2];

	build_arglist(CMD_CC0);
	t = xstrdup(path, 0);
	tmp = pathmod(t, ".c", ".%", 0);
	redirect_in(tmp);
	t = xstrdup(path, 0);
	tmp = pathmod(t, ".%", ".@", 0);
	if (tmp == NULL)
		memory();
	redirect_out(tmp);
	run_command();
	build_arglist(CMD_CC1);
	redirect_in(tmp);
	tmp = pathmod(path, ".@", ".#", 0);
	redirect_out(tmp);
	run_command();
	build_arglist(CMD_CC2);
	/* The sym stuff is a bit hackish right now */
	add_argument(".symtmp");
	add_argument(cpu);
	optstr[0] = optimize;
	optstr[1] = '\0';
	add_argument(optstr);
	if (codeseg)
		add_argument(codeseg);
	redirect_in(tmp);
	if (optimize == '0') {
		redirect_out(pathmod(path, ".#", ".s", 2));
		run_command();
		free(t);
		return;
	}
	tmp = pathmod(path, ".#", ".^", 0);
	redirect_out(tmp);
	run_command();
	build_arglist(CMD_COPT);
	add_argument(COPTRULES);
	redirect_in(tmp);
	redirect_out(pathmod(path, ".#", ".s", 2));
	run_command();
	free(t);
}

void convert_S_to_s(char *path)
{
	char *tmp;
	build_arglist(CMD_CPP);
	add_argument("-E");
	add_argument(path);
	tmp = xstrdup(path, 0);
	redirect_out(pathmod(tmp, ".S", ".s", 1));
	run_command();
	pathmod(path, ".S", ".s", 5);
}

void preprocess_c(char *path)
{
	char *tmp;
	build_arglist(CMD_CPP);

	add_argument_list("-I", &inclist);
	add_argument_list("-D", &deflist);
	add_argument("-E");
	add_argument(path);
	/* Weird one .. -E goes to stdout */
	tmp = xstrdup(path, 0);
	if (last_phase != 1)
		redirect_out(pathmod(tmp, ".c", ".%", 0));
	run_command();
}

void link_phase(void)
{
	build_arglist(CMD_LD);
	switch (targetos) {
		case OS_FUZIX:
			switch(fuzixsub) {
			case 0:
				break;
			case 1:
				add_argument("-b");
				add_argument("-C");
				add_argument("256");
				break;
			case 2:
				add_argument("-b");
				add_argument("-C");
				add_argument("512");
				break;
			}
			break;
		case OS_NONE:
		default:
			add_argument("-b");
			add_argument("-C");
			add_argument("256");
			break;
	}
	if (strip)
		add_argument("-s");
	add_argument("-o");
	add_argument(target);
	if (mapfile) {
		/* For now output a map file. One day we'll have debug symbols
		   nailed to the binary */
		char *n = malloc(strlen(target) + 5);
		sprintf(n, "%s.map", target);
		add_argument("-m");
		add_argument(n);
	}
	if (!standalone) {
		/* Start with crt0.o, end with libc.a and support libraries */
		add_argument(CRT0);
		append_obj(&libpathlist, LIBPATH, 0);
		append_obj(&liblist, LIBC, TYPE_A);
	}
	append_obj(&liblist, LIBCPU, TYPE_A);
	add_argument_list(NULL, &objlist);
	resolve_libraries();
	run_command();
}

void sequence(struct obj *i)
{
//	printf("Last Phase %d\n", last_phase);
//	printf("1:Processing %s %d\n", i->name, i->type);
	if (i->type == TYPE_S) {
		convert_S_to_s(i->name);
		i->type = TYPE_s;
		i->used = 1;
	}
	if (i->type == TYPE_C) {
		preprocess_c(i->name);
		i->type = TYPE_C_pp;
		i->used = 1;
	}
	if (last_phase == 1)
		return;
//	printf("2:Processing %s %d\n", i->name, i->type);
	if (i->type == TYPE_C_pp || i->type == TYPE_C) {
		convert_c_to_s(i->name);
		i->type = TYPE_s;
		i->used = 1;
	}
	if (last_phase == 2)
		return;
//	printf("3:Processing %s %d\n", i->name, i->type);
	if (i->type == TYPE_s) {
		convert_s_to_o(i->name);
		i->type = TYPE_O;
		i->used = 1;
	}
}

void processing_loop(void)
{
	struct obj *i = objlist.head;
	while (i) {
		sequence(i);
		remove_temporaries();
		i = i->next;
	}
	if (last_phase < 4)
		return;
	link_phase();
	/* And clean up anything we couldn't wipe earlier */
	last_phase = 255;
	remove_temporaries();
}

void unused_files(void)
{
	struct obj *i = objlist.head;
	while (i) {
		if (!i->used)
			fprintf(stderr, "cc: warning file %s unused.\n",
				i->name);
		i = i->next;
	}
}

void usage(void)
{
	fprintf(stderr, "usage...\n");
	fatal();
}

char **add_macro(char **p)
{
	if ((*p)[2])
		append_obj(&deflist, *p + 2, 0);
	else
		append_obj(&deflist, *++p, 0);
	return p;
}

char **add_library(char **p)
{
	if ((*p)[2])
		append_obj(&liblist, *p + 2, TYPE_A);
	else
		append_obj(&liblist, *++p, TYPE_A);
	return p;
}

char **add_library_path(char **p)
{
	if ((*p)[2])
		append_obj(&libpathlist, *p + 2, 0);
	else
		append_obj(&libpathlist, *++p, 0);
	return p;
}


char **add_includes(char **p)
{
	if ((*p)[2])
		append_obj(&inclist, *p + 2, 0);
	else
		append_obj(&inclist, *++p, 0);
	return p;
}

void add_system_include(char *p)
{
	append_obj(&inclist, p, 0);
}

void dunno(const char *p)
{
	fprintf(stderr, "cc: don't know what to do with '%s'.\n", p);
	fatal();
}

void add_file(char *p)
{
	char *x = strrchr(p, '.');
	if (x == NULL)
		dunno(p);
	switch (x[1]) {
	case 'a':
		append_obj(&objlist, p, TYPE_A);
		break;
	case 's':
		append_obj(&objlist, p, TYPE_s);
		break;
	case 'S':
		append_obj(&objlist, p, TYPE_S);
		break;
	case 'c':
		append_obj(&objlist, p, TYPE_C);
		c_files++;
		break;
	case 'o':
		append_obj(&objlist, p, TYPE_O);
		break;
	default:
		dunno(p);
	}
}

void one_input(void)
{
	fprintf(stderr, "cc: too many files for -E\n");
	fatal();
}

void uniopt(char *p)
{
	if (p[2])
		usage();
}

static unsigned valid_cpu(char *name)
{
	/* Eventually I guess we'll make this pick the assembler to use */
	return 0;
}

int main(int argc, char *argv[]) {
	char **p = argv;
	signal(SIGCHLD, SIG_DFL);

	while (*++p) {
		/* filename or option ? */
		if (**p != '-') {
			add_file(*p);
			continue;
		}
		switch ((*p)[1]) {
			/* Don't link */
		case 'c':
			uniopt(*p);
			last_phase = 3;
			break;
			/* Don't assemble */
		case 'S':
			uniopt(*p);
			last_phase = 2;
			break;
			/* Only pre-process */
		case 'E':
			uniopt(*p);
			last_phase = 1;
			only_one_input = 1;
			break;
		case 'l':
			p = add_library(p);
			break;
		case 'I':
			p = add_includes(p);
			break;
		case 'L':
			p = add_library_path(p);
			break;
		case 'D':
			p = add_macro(p);
			break;
		case 'i':
/*                    split_id();*/
			uniopt(*p);
			break;
		case 'o':
			if (target != NULL) {
				fprintf(stderr,
					"cc: -o can only be used once.\n");
				fatal();
			}
			if ((*p)[2])
				target = *p + 2;
			else if (*p)
				target = *++p;
			else {
				fprintf(stderr, "cc: no target given.\n");
				fatal();
			}
			break;
		case 'O':
			if ((*p)[2]) {
				char o = (*p)[2];
				if (o >= '0' && o <= '3')
					optimize = o;
				else if (o == 's')
					optimize = 's';
				else {
					fprintf(stderr, "cc: unknown optimixation level.\n");
					fatal();
				}
			} else
				optimize = '1';
			break;
		case 's':	/* FIXME: for now - switch to getopt */
			standalone = 1;
			break;
		case 'X':
			uniopt(*p);
			keep_temp = 1;
			break;
		case 'm':
			if (!valid_cpu(*p + 2)) {
				fprintf(stderr, "cc: unknown CPU type.\n");
				fatal();
			}
			cpu = *p + 2;
			break;	
		case 'M':
			mapfile = 1;
			break;
		case 't':
			if (strcmp(*p + 2, "fuzix") == 0) {
				targetos = OS_FUZIX;
				fuzixsub = 0;
			}
			else if (strcmp(*p + 2, "fuzixrel1") == 0) {
				targetos = OS_FUZIX;
				fuzixsub = 1;
			}
			else if (strcmp(*p + 2, "fuzixrel2") == 0) {
				targetos = OS_FUZIX;
				fuzixsub = 2;
			} else {
				fprintf(stderr, "cc: only fuzix target types are known.\n");
				fatal();
			}
			break;
		case 'T':
			codeseg = *p + 2;
			break;
		default:
			usage();
		}
	}

	if (!standalone)
		add_system_include(INCPATH);

	if (target == NULL)
		target = "a.out";
	if (only_one_input && c_files > 1)
		one_input();
	processing_loop();
	unused_files();
	return 0;
}
