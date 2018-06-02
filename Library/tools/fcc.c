#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

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
static int valgrind = 0;

static unsigned int progbase = 0x0100;	/* Program base */

struct arglist {
  struct arglist *next;
  char p[0];
};

static void oom(void)
{
  fprintf(stderr, "Out of memory.\n");
  exit(1);
}

static char *mstrdup(const char *p)
{
  char *n = strdup(p);
  if (n == NULL)
    oom();
  return n;
}

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

struct arglist *machead, *mactail;

static void add_macro(const char *p)
{
  struct arglist *a = arg_make(p);
  if (machead == NULL)
    machead = a;
  else
    mactail->next = a;
  mactail = a;
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

struct arglist *extrahead, *extratail;

static void add_extra(const char *p)
{
  struct arglist *a = arg_make(p);
  if (extrahead == NULL)
    extrahead = a;
  else
    extratail->next = a;
  extratail = a;
}

static char *target;

static void set_target(const char *p)
{
  not_null(p);
  if (target != NULL) {
    fprintf(stderr, "-o cannot be used twice.\n");
    exit(1);
  }
  target = mstrdup(p);
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
  opt = mstrdup(p);
  if (*opt && sscanf(opt, "%d", &optcode) != 1) {
    fprintf(stderr, "fcc: -O requires a value\n");
    exit(1);
  }
  if (optcode > 4)
    optcode = 4;
  if (optcode < 0) {
    fprintf(stderr, "fcc: negative optimisation is silly.\n");
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
  map = mstrdup(p);
}

static const char *cpu;

static void set_cpu(const char *p)
{
  not_null(p);
  if (cpu != NULL) {
    fprintf(stderr, "-m cannot be used twice.\n");
    exit(1);
  }
  cpu = mstrdup(p);
  if (strcmp(cpu, "z80") && strcmp(cpu, "z180") && strcmp(cpu, "r2k") && strcmp(cpu, "r3ka")) {
    fprintf(stderr,"fcc: only Zilog z80, z180 and Rabbit r2k/r3ka targets currently handled.\n");
    exit(1);
  }
}

static const char *platform = "";

static void set_platform(const char *p)
{
  char *n;
  not_null(p);
  if (*platform) {
    fprintf(stderr, "-t cannot be used twice.\n");
    exit(1);
  }
  n = malloc(strlen(p) + 2);
  if (n == NULL)
    oom();
  sprintf(n, "-%s", p);
  platform = n;
  /* TRS80m1 don't need special libraries just a new base address */
  if (strcmp(platform, "-trs80m1") == 0) {
    platform="";
    progbase = 0x8000;
  }
  if (strcmp(platform, "-zx128") == 0) {
  /* ZX128 does because it can't use RST for syscalls */
    progbase = 0x8000;
  }
}

static int debug;
static int pedantic;
static int werror;
static int unschar;
static int nostdio = 0;


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
  t = strrchr(p, '/');
  if (t)
    t = strrchr(t, '.');
  else
    t = strrchr(p, '.');
  if (t)
    strcpy(t + 1, ext);
  else {
    strcat(p, ".");
    strcat(p, ext);
  }
  return p;
}

char *filebasename(char *path)
{
  /* The POSIX one can mangle the input - so given its trivial do it
     sanely ourselves */
  char *p = strrchr(path, '/');
  if (p == NULL)
    return path;
  /* We don't care about trailing slashes, we only work on files */
  return p + 1;
}
  

static char *chopname(const char *i)
{
  char *p = mstrdup(i);
  char *t;
  if (p == NULL) {
    fprintf(stderr, "Out of memory.\n");
    exit(1);
  }
  t = strrchr(p, '/');
  if (t) {
    t = strrchr(t + 1, '.');
    if (t)
      *t = 0;
  }
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
  if (WIFSIGNALED(status)) {
    fprintf(stderr, "%s exited with signal %s\n",
      argvec[0], strsignal(WTERMSIG(status)));
    return 255;
  }
  return (WEXITSTATUS(status));
}

/*
 *	The SDCC tool chain screws up if fed ./foo.o as a target so undo
 *	any ./ bit
 */
static char *undotslash(char *p)
{
  if (*p == '.' && p[1] == '/')
    return p + 2;
  return p;
}
    
/*
 *	Stitch together an sdcc command.
 */

static void build_command(void)
{
  char buf[128];

  if (valgrind == 1)
    add_argument("valgrind");

  add_argument("sdcc");
  if (verbose == 1)
    add_argument("-V");
  /* Set up the basic parameters we will inflict on the user */
  add_argument("--std-c99");
  add_option("-m", cpu?cpu:"z80");
  if (mode == MODE_LINK) {
    add_argument("--no-std-crt0");
    add_argument("--nostdlib");
    add_argument("--code-loc");
    snprintf(buf, sizeof(buf), "%d", progbase);
    add_argument(mstrdup(buf));
    add_argument("--data-loc");
    add_argument("0x0");
  }
  /* Parse the specials */
  if (pedantic == 0)
    add_argument("--less-pedantic");
  if (werror == 1)
    add_argument("--Werror");
  if (unschar == 1)
    add_argument("--funsigned-char");
  if (unschar == 2)
    add_argument("--fsigned-char");
  if (debug == 1)
    add_argument("--debug");
  /* Turn -O1/2/3/4 into something meaningful */
  if (opt != NULL) {
    if (optcode > 0)
      add_argument("--max-allocs-per-node");
    if (optcode == 1)
      add_argument("30000");
    if (optcode == 2 || optcode == 3)
      add_argument("100000");
    if (optcode == 4)
      add_argument("250000");
    if (optcode > 2)
      add_argument("-D__FUZIX_OPT_SPEED__");
  }
  /* Size optimise for all but high -O values */
  if (opt == NULL || optcode < 3)
    add_argument("--opt-code-size");
  /* Macros */
  add_argument("-D__FUZIX__");
  /* Suppress the warnings when sharing code across architectures */
  add_argument("-Ddouble=float");
  /* User provided macros */
  add_argument_list(machead);
  /* -Wc, options */
  add_argument_list(extrahead);
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
    if (srchead->next && target) {
      fprintf(stderr, "Cannot use -c together with -o with multiple input files.\n");
      exit(1);
    }
  }
  if (mode == MODE_LINK) {
    if (target == NULL)
      autotarget();
    if (target == NULL) {
      fprintf(stderr, "no target.\n");
      exit(1);
    }
    add_option("-o", undotslash(target));
    if (nostdio)
      snprintf(buf, sizeof(buf), FCC_DIR "/lib/crt0nostdio%s.rel", platform);
    else
      snprintf(buf, sizeof(buf), FCC_DIR "/lib/crt0%s.rel", platform);
    add_argument(mstrdup(buf));
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
  char buf[128];
  
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
        case 'D':
          add_macro(p);
          break;
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
        case 't':
          if (p[2] == 0)
            set_platform(argv[++i]);
          else
            set_platform(p + 2);
          break;
        case 'g':
          debug = 1;
          break;
        case 'W':	/* -Wc,-foo to pass on -foo directly */
          if (p[2] == 'c' && p[3] == ',') {
            add_extra(p + 4);
            break;
          }
          /* Fall through */
        default:
          if (strcmp(p, "-Werror") == 0)
            werror = 1;
          else if (strcmp(p, "-funsigned-char") == 0)
            unschar = 1;
          else if (strcmp(p, "-fsigned-char") == 0)
            unschar = 2;
          else if (strcmp(p, "--pedantic") == 0)
            pedantic = 1;
          else if (strcmp(p, "--nostdio") == 0)
            nostdio = 1;
          else if (strcmp(p, "--valgrind") == 0)
            valgrind = 1;
          else {
            fprintf(stderr, "fcc: Unknown option '%s'.\n", p);
            exit(1);
          }
      }
    }
  }
  add_include_path(FCC_DIR "/include/");
  add_library_path(FCC_DIR "/lib/");
  snprintf(buf, sizeof(buf), "c%s", platform);
  add_library(buf);

  if (mode == MODE_OBJ) {
    while (srchead) {
      build_command();
      ret = do_command();
      if (ret)
        break;
      if (mode == MODE_OBJ && target) {
        char *orel = filebasename(rebuildname("", srchead->p, "rel"));
        if (rename(orel, target) == -1) {
          fprintf(stderr, "Unable to rename %s to %s.\n", orel, target);
          perror(srchead->p);
          exit(1);
        }
      }
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
  snprintf(buf, sizeof(buf), "%x", progbase);
  add_argument(buf);
  add_argument(t);
  add_argument(rebuildname("", target, "map"));
  add_argument(target);
  ret = do_command();
  exit(ret);
}
