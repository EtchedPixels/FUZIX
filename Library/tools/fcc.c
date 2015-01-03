#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 *	Wrap the sdcc compiler tools into something more Unixlike and without
 *	spraying support files everwhere
 */

#define FCC_DIR		    "/opt/fcc"

#define MODE_LINK		0
#define MODE_OBJ		1
#define MODE_CPP		2
#define MODE_ASM		3
static int mode = MODE_LINK;
static char *workdir;		/* Working directory */

static int verbose = 0;

struct arglist {
  struct arglist *next;
  char p[0];
};

static void not_null(const char *p)
{
  if (p == NULL) {
    fprintf(stderr, "required option parameter missing at end of command line.\n");
    exit(1);
  }
}

static struct arglist *arg_make(const char *p)
{
  struct arglist *x;

  not_null(p);  

  x = malloc(sizeof(struct arglist) + strlen(p) + 1);
  if (x == NULL) {
    fprintf(stderr, "Out of memory.\n");
    exit(1);
  }
  x->next = NULL;
  strcpy(x->p, p);
  return x;
}

struct arglist *srchead, *srctail;

static void add_source(const char *p)
{
  struct arglist *a = arg_make(p);
  if (srchead == NULL)
    srchead = a;
  else
    srctail->next = a;
  srctail = a;
}

struct arglist *libphead, *libptail;

static void add_library_path(const char *p)
{
  struct arglist *a = arg_make(p);
  if (libphead == NULL)
    libphead = a;
  else
    libptail->next = a;
  libptail = a;
}

struct arglist *libhead, *libtail;

static void add_library(const char *p)
{
  struct arglist *a = arg_make(p);
  if (libhead == NULL)
    libhead = a;
  else
    libtail->next = a;
  libtail = a;
}

struct arglist *includehead, *includetail;

static void add_include_path(const char *p)
{
  struct arglist *a = arg_make(p);
  if (includehead == NULL)
    includehead = a;
  else
    includetail->next = a;
  includetail = a;
}

static char *target;

static void set_target(const char *p)
{
  not_null(p);
  if (target != NULL) {
    fprintf(stderr, "-o cannot be used twice.\n");
    exit(1);
  }
  target = strdup(p);
}

static const char *opt;
static int optcode;

static void set_optimize(const char *p)
{
  not_null(p);
  if (opt != NULL) {
    fprintf(stderr, "fcc: -O cannot be used twice.\n");
    exit(1);
  }
  opt = strdup(p);
  if (*opt && sscanf(opt, "%d", &optcode) != 1) {
    fprintf(stderr, "fcc: -O requires a value\n");
    exit(1);
  }
  if (*opt == 0)
    optcode = 1;
}

static const char *map;

static void set_map(const char *p)
{
  not_null(p);
  if (map != NULL) {
    fprintf(stderr, "-M cannot be used twice.\n");
    exit(1);
  }
  map = strdup(p);
}

static const char *cpu;

static void set_cpu(const char *p)
{
  not_null(p);
  if (cpu != NULL) {
    fprintf(stderr, "-m cannot be used twice.\n");
    exit(1);
  }
  cpu = strdup(p);
  if (strcmp(cpu, "z80") && strcmp(cpu, "z180")) {
    fprintf(stderr,"fcc: only z80 and z180 targets currently handled.\n");
    exit(1);
  }
}

static int debug;
static int pedantic;
static int werror;
static int unschar;


static char *rebuildname(const char *r, const char *i, char *ext)
{
  char *p = malloc(strlen(r) + strlen(i) + strlen(ext) + 2);
  char *t;
  if (p == NULL) {
    fprintf(stderr, "Out of memory.\n");
    exit(1);
  }
  strcpy(p, r);
  strcat(p, i);
  t = strrchr(p, '.');
  if (t)
    strcpy(t + 1, ext);
  else {
    strcat(p, ".");
    strcat(p, ext);
  }
  return p;
}

static char *chopname(const char *i)
{
  char *p = strdup(i);
  char *t;
  if (p == NULL) {
    fprintf(stderr, "Out of memory.\n");
    exit(1);
  }
  t = strrchr(p, '.');
  if (t)
    *t = 0;
  return p;
}

static void autotarget(void)
{
  char *x;
  if (srchead == NULL)
    return;

  if (mode == MODE_LINK) {
    target = rebuildname("", srchead->p, "ihx");
    return;
  }
  if (mode != MODE_OBJ)
    return;
  target = rebuildname("", srchead->p, "rel");
}

#define MAX_ARG 256
static char *argvec[MAX_ARG+1];
static int argp = 0;

static void add_argument(const char *p)
{
  if (argp == MAX_ARG) {
    fprintf(stderr, "fcc: Too many arguments.\n");
    exit(1);
  }
  argvec[argp++] = (char *)p;
  if (verbose)
    printf("[%s] ", p);
}

static void add_option(const char *a, const char *b)
{
  char *x = malloc(strlen(a) + strlen(b) + 1);
  if (x == NULL) {
    fprintf(stderr, "Out of memory.\n");
    exit(1);
  }
  strcpy(x, a);
  strcat(x, b);
  add_argument(x);
}

static void add_argument_list(struct arglist *l)
{
  while(l) {
    add_argument(l->p);
    l = l->next;
  }
}

static void add_option_list(const char *a, struct arglist *l)
{
  while(l) {
    add_option(a, l->p);
    l = l->next;
  }
}  

static int do_command(void)
{
  pid_t pid, w;
  int status;

  if (verbose)
    printf("\n\n");
  argvec[argp] = NULL;
  pid = fork();
  switch (pid) {
    case 0:
      execvp(argvec[0], argvec);
      perror(argvec[0]);
      _exit(0);
    case -1:
      perror("fork");
      exit(1);
    default:
      while((w = wait(&status)) != pid);
  }
  return (WEXITSTATUS(status));
}
    
/*
 *	Stitch together an sdcc command.
 */

static void build_command(void)
{
  add_argument("sdcc");
  /* Set up the basic parameters we will inflict on the user */
  add_argument("--std-c99");
  add_option("-m", cpu?cpu:"z80");
  if (mode == MODE_LINK) {
    add_argument("--no-std-crt0");
    add_argument("--nostdlib");
    add_argument("--code-loc");
    add_argument("0x100");
    add_argument("--data-loc");
    add_argument("0x0");
  }
  /* Parse the specials */
  if (pedantic == 0)
    add_argument("--less-pedantic");
  if (werror == 1)
    add_argument("-Werror");
  if (unschar == 1)
    add_argument("-funsigned-char");
  if (debug == 1)
    add_argument("--debug");
  /* Turn -O1/2/3 into something meaningful */
  if (opt != NULL) {
    if (optcode > 0)
      add_argument("--max-allocs-per-node");
    if (optcode == 1)
      add_argument("30000");
    if (optcode == 2)
      add_argument("100000");
    if (optcode == 3)
      add_argument("250000");
  }
  /* Always size optimise */
  add_argument("--opt-code-size");
  /* Paths */
  add_option_list("-I", includehead);
  if (mode == MODE_LINK)
    add_option_list("-L", libphead);
  if (mode == MODE_CPP)
    add_argument("-E");
  if (mode == MODE_ASM)
    add_argument("-S");
  if (mode == MODE_OBJ) {
    if (srchead == NULL) {
      fprintf(stderr, "The -c option requires an input.\n");
      exit(1);
    }
    add_argument("-c");
  }
  if (mode == MODE_LINK) {
    if (target == NULL)
      autotarget();
    if (target == NULL) {
      fprintf(stderr, "no target.\n");
      exit(1);
    }
    add_option("-o", target);
    add_argument(FCC_DIR "/lib/crt0.rel");
  }
  if (srchead) {
    if (mode == MODE_OBJ)
      add_argument(srchead->p);
    else
      add_argument_list(srchead);
  }
  else {
    fprintf(stderr, "fcc: No sources specified.\n");
    exit(1);
  }
  if (mode == MODE_LINK)
    add_option_list("-l", libhead);
}

int main(int argc, const char *argv[]) {
  const char *p;
  char *t;
  int i;
  int ret;
  
  for(i = 1; i < argc; i++) {
    p = argv[i];
    if (*p != '-')
      add_source(p);
    else {
      switch(p[1]) {
        case 'V':
          verbose = 1;
          break;
        /* What are we doing */
        case 'c':
          mode = MODE_OBJ;
          break;
        case 'E':
          mode = MODE_CPP;
          break;
        case 'S':
          mode = MODE_ASM;
          break;
        case 'v':
          printf("fcc: front end to sdcc\n");
          add_argument("sdcc");
          add_argument("-v");
          do_command();
          exit(0);
        case 'l':
          add_library(p+2);
          break;
        case 'L':
          if (p[2] == 0)
            add_library_path(argv[++i]);
          else
            add_library_path(p+2);
          break;
        case 'I':
          if (p[2] == 0)
            add_include_path(argv[++i]);
          else
            add_include_path(p+2);
          break;
        case 'o':
          if (p[2] == 0)
            set_target(argv[++i]);
          else
            set_target(p + 2);
          break;
        case 'O':
          set_optimize(p + 2);
          break;
        case 'm':
          if (p[2] == 0)
            set_cpu(argv[++i]);
          else
            set_cpu(p + 2);
          break;
        case 'M':
          if (p[2] == 0)
            set_map(argv[++i]);
          else
            set_map(p + 2);
          break;
        case 'g':
          debug = 1;
          break;
        default:
          if (strcmp(p, "-Werror") == 0)
            werror = 1;
          else if (strcmp(p, "-funsigned-char") == 0)
            unschar = 1;
          else if (strcmp(p, "-pedantic") == 0)
            pedantic = 1;
          else {
            fprintf(stderr, "fcc: Unknown option '%s'.\n", p);
            exit(1);
          }
      }
    }
  }
  add_include_path(FCC_DIR "/include/");
  add_library_path(FCC_DIR "/lib/");
  add_library("c");

  if (mode == MODE_OBJ) {
    while (srchead) {
      build_command();
      ret = do_command();
      if (ret)
        break;
      srchead = srchead->next;
      argp = 0;
    }
  } else {
      build_command();
      ret = do_command();
  }  
  if (mode != MODE_LINK || ret)
    exit(ret);
  argp = 0;
  add_argument("makebin");
  add_argument("-p");
  add_argument("-s");
  add_argument("65535");
  add_argument(target);
  add_argument(t = rebuildname("", target, "bin"));
  ret = do_command();
  if (ret)
    exit(ret);
  argp = 0;
  add_argument(FCC_DIR "/bin/binman");
  add_argument(t);
  add_argument(rebuildname("", target, "map"));
  add_argument(chopname(target));
  ret = do_command();
  exit(ret);
}
