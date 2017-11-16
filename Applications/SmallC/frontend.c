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
 *	$1.c			preprocessor, may make $1.% or /dev/tty
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef CROSS
#define	CMD_AS	"/tmp/cross/as"
#define CMD_CC	"/tmp/cross/scc"
#define CMD_COPT "/tmp/cross/copt"
#define COPT_FILE "/tmp/cross/ccc-copt"
#define CMD_LD	"/tmp/cross/ld"
#define CMD_CPP	"/tmp/cross/cpp"
#define CRT0	"/tmp/cross/crt0.o"
#else
#define CMD_AS	"/bin/as"
#define CMD_CC	"/usr/lib/cc"
#define CMD_COPT "/usr/lib/copt"
#define COPT_FILE "/usr/lib/cc-copt"
#define CMD_LD	"/bin/ld"
#define CMD_CPP "/usr/lib/cpp"
#define CRT0	"/usr/lib/crt0.o"
#endif

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

int keep_temp;
int last_phase = 4;
int only_one_input;
char *target;
int strip;
int c_files;
int standalone;

#define MAXARG	64

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
	else {
		h->tail = o;
		h->head = o;
	}
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
		*rmptr = strdup(p);
		if (*rmptr++ == NULL)
			memory();
	}
	/* Traditionally compilers dump the work files in the current dir
	   even if the source is /foo/bar/baz.c - follow this */
	x = strrchr(p,'/');
	if (x == NULL)
		return p;
	return x+1;
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

static void run_command(void)
{
	pid_t pid, p;
	int status;

	fflush(stdout);

	pid = fork();
	if (pid == -1) {
		perror("fork");
		fatal();
	}
	if (pid == 0) {
//		printf("Run %s\n", arglist[0]);
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
	if (WIFSIGNALED(status) || WEXITSTATUS(status)) {
//		printf("cc: %s failed.\n", arglist[0]);
		fatal();
	}
}

static void redirect_in(const char *p)
{
	arginfd = open(p, O_RDONLY);
	if (arginfd == -1) {
		perror(p);
		fatal();
	}
}

static void redirect_out(const char *p)
{
	argoutfd = open(p, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (argoutfd == -1) {
		perror(p);
		fatal();
	}
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
	char *tmp;
	build_arglist(CMD_CC);
	redirect_in(path);
	tmp = strdup(pathmod(path, ".%", ".@", 0));
	if (tmp == NULL)
		memory();
	redirect_out(tmp);
	run_command();
	build_arglist(CMD_COPT);
	add_argument(COPT_FILE);
	redirect_in(tmp);
	redirect_out(pathmod(path, ".%", ".s", 2));
	run_command();
	free(tmp);
}

void convert_S_to_s(char *path)
{
	build_arglist(CMD_CPP);
	redirect_in(path);
	redirect_out(pathmod(path, ".S", ".s", 1));
	run_command();
}

void preprocess_c(char *path)
{
	build_arglist(CMD_CPP);

	add_argument_list("-I", &inclist);
	add_argument_list("-D", &deflist);
	add_argument(path);
	/* Weird one .. -E goes to stdout */
	if (last_phase != 1)
		redirect_out(pathmod(path, ".c", ".%", 0));
	run_command();
}

void link_phase(void)
{
	build_arglist(CMD_LD);
	add_argument("-b");
	add_argument("-c");
	add_argument("256");
	if (strip)
		add_argument("-s");
	add_argument("-o");
	add_argument(target);
	if (!standalone)
		add_argument(CRT0);
	add_argument_list(NULL, &objlist);
	add_argument_list(NULL, &liblist);
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
	if (i->type == TYPE_C_pp) {
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
		/* HACK should be TYPE_C once we split cpp */
		append_obj(&objlist, p, TYPE_C_pp);
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

int main(int argc, char *argv[])
{
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
					"-o can only be used once.\n");
				fatal();
			}
			if ((*p)[2])
				target = *p + 2;
			else
				target = *++p;
			break;
		case 'X':
			uniopt(*p);
			keep_temp = 1;
			break;
		default:
			usage();
		}
	}

	if (target == NULL)
		target = "a.out";
	if (only_one_input && c_files > 1)
		one_input();
	processing_loop();
	unused_files();
	return 0;
}
