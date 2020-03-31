#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/* To fix:
   reloc: rename __1 map to base.map
   remove target.lk target.noi .lst .sym
   if linking remove each .rel .asm (flag to keep .asm ?) */

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
static int relocatable = 1;
static int keep_temps = 0;

static unsigned int progbase = 0x0100;	/* Program base */

struct arglist {
  struct arglist *next;
  int flag;
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
  char *x;
  struct arglist *a = arg_make(p);
  x = strchr(p, '.');
  /* Don't clean up passed in binaries */
  if (x == NULL || strcmp(x, ".c"))
    a->flag = 0;
  else
    a->flag = 1;
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
static char *rootname;

static void set_target(const char *p)
{
  not_null(p);
  if (target != NULL) {
    fprintf(stderr, "-o cannot be used twice.\n");
    exit(1);
  }
  target = mstrdup(p);
  rootname = target;
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
static const char *libend;

static void set_cpu(const char *p)
{
  not_null(p);
  if (cpu != NULL) {
    fprintf(stderr, "-m cannot be used twice.\n");
    exit(1);
  }
  cpu = mstrdup(p);
  if (strcmp(cpu, "z80") && strcmp(cpu, "z180") && strcmp(cpu, "r2k") && strcmp(cpu, "r3ka") && strcmp(cpu, "ez80_z80")) {
    fprintf(stderr,"fcc: only Zilog z80, z180, ez80_z80 and Rabbit r2k/r3ka targets currently handled.\n");
    exit(1);
  }
  if (strcmp(cpu, "z80") && strcmp(cpu, "z180"))
    libend = cpu;
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
    return;
  }
  else if (strcmp(platform, "-zx128") == 0) {
  /* ZX128 does because it can't use RST for syscalls */
    progbase = 0x8000;
    return;
  }
  else {
    fprintf(stderr, "fcc: unknown platform '%s'.\n", p);
    exit(1);
  }
}

static int debug;
static int pedantic;
static int werror;
static int unschar;
static int nostdio;
static int dynlib;
static int nocrt0;


static char *extendname(const char *r, const char *i, char *ext)
{
  char *p = malloc(strlen(r) + strlen(i) + strlen(ext) + 2);
  char *t;
  if (p == NULL) {
    fprintf(stderr, "Out of memory.\n");
    exit(1);
  }
  strcpy(p, r);
  strcat(p, i);
  strcat(p, ".");
  strcat(p, ext);
  return p;
}

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
  if (*ext == 0) {
    *t = 0;
    return p;
  }
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
    rootname = rebuildname("", srchead->p, "");
    target = rebuildname("", srchead->p, "ihx");
    return;
  }
  if (mode != MODE_OBJ)
    return;
  rootname = rebuildname("", srchead->p, "");
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

static char *delvec[MAX_ARG+1];
static int delp = 0;

static char *latevec[MAX_ARG+1];
static int ldelp = 0;

static void delete_temporaries(void)
{
  int i ;
  if (keep_temps)
    return;

  for (i = 0; i < delp; i++) {
    if (unlink(delvec[i]) == 0 && verbose)
      printf("unlink %s\n", delvec[i]);
    else if (verbose)
        printf("unlink %s not found.\n", delvec[i]);
    delvec[i] = NULL;
  }
  delp = 0;
}

static void delete_late(void)
{
  int i ;
  for (i = 0; i < ldelp; i++) {
    if (unlink(latevec[i]) == 0 && verbose)
      printf("unlink %s\n", latevec[i]);
    else if (verbose)
        printf("unlink %s not found.\n", latevec[i]);
    delvec[i] = NULL;
  }
  delp = 0;
}

static void reset_arguments(void)
{
  delete_temporaries();
  argp = 0;
}

static void reset_arguments_nodelete(void)
{
  argp = 0;
}

static const char *add_delete(const char *p)
{
  if (delp == MAX_ARG) {
    fprintf(stderr, "fcc: Too many arguments.\n");
    exit(1);
  }
  delvec[delp++] = (char *)p;
  return p;
}

static const char *add_argument_delete(const char *p)
{
  add_delete(p);
  add_argument(p);
  return p;
}

static const char *add_delete_late(const char *p)
{
  if (ldelp == MAX_ARG) {
    fprintf(stderr, "fcc: Too many arguments.\n");
    exit(1);
  }
  latevec[ldelp++] = (char *)p;
  return p;
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

static void add_option_postfixed(const char *a, const char *b, const char *c)
{
  char *x = malloc(strlen(a) + strlen(b) + strlen(c) + 1);
  if (x == NULL) {
    fprintf(stderr, "Out of memory.\n");
    exit(1);
  }
  strcpy(x, a);
  strcat(x, b);
  strcat(x, c);
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

static void add_option_list_postfixed(const char *a, struct arglist *l,
                                                          const char *post)
{
  if (!post) {
    add_option_list(a, l);
    return;
  }
  while(l) {
    add_option_postfixed(a, l->p, post);
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
  status = WEXITSTATUS(status);
  if (status && verbose)
    fprintf(stderr, "%s exited with status %d\n", argvec[0], status);
  return status;
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
 *	Magic for relocations
 */

static char *relocmap(char *in, int pass)
{
  char *n;
  if (!relocatable)
    return in;
  /* tack it together */
  n = malloc(strlen(in) + 4);
  if (n == NULL)
    oom();
  sprintf(n, "%s__%1d", in, pass);
  return n;
}

/*
 *	Stitch together an sdcc command.
 */
static void build_command(int pass)
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
  add_argument("--fverbose-asm");
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
    const char *rp = "";
    const char *hp = "";
    char *tmp;
    if (relocatable) {
      if (cpu && strcmp(cpu, "ez80_z80") == 0)
        rp = "_ez80_z80_rel";
      else
        rp = "_z80_rel";
    } else {
      if (cpu && strcmp(cpu, "ez80_z80") == 0)
        hp = cpu;
    }
    if (target == NULL)
      autotarget();
    if (target == NULL) {
      fprintf(stderr, "no target.\n");
      exit(1);
    }
    add_option("-o", add_delete_late(extendname("",relocmap(undotslash(rootname), pass), "ihx")));
    if (nocrt0 == 0) {
      if (dynlib)
        snprintf(buf, sizeof(buf), FCC_DIR "/lib/lib0%s%s%s.rel", hp, platform, rp);
      else if (nostdio)
        snprintf(buf, sizeof(buf), FCC_DIR "/lib/crt0%s%snostdio%s.rel", hp, platform, rp);
      else
        snprintf(buf, sizeof(buf), FCC_DIR "/lib/crt0%s%s%s.rel", hp, platform, rp);
      rp = mstrdup(buf);
      add_argument(rp);
    }
  }
  if (srchead) {
    if (mode == MODE_OBJ) {
      add_argument(srchead->p);
      if (srchead->flag) {
        add_delete(rebuildname("", srchead->p, "asm"));
        add_delete(rebuildname("", srchead->p, "lst"));
        add_delete(rebuildname("", srchead->p, "sym"));
      }
    } else {
      struct arglist *l = srchead;
      while(l) {
        add_argument(l->p);
        if (srchead->flag) {
          add_delete(rebuildname("", l->p, "asm"));
          add_delete(rebuildname("", l->p, "lst"));
          add_delete_late(rebuildname("", l->p, "rel"));
          add_delete(rebuildname("", l->p, "sym"));
        }
        l = l->next;
      }
    }
  }
  else {
    fprintf(stderr, "fcc: No sources specified.\n");
    exit(1);
  }
  if (mode == MODE_LINK)
    add_option_list_postfixed("-l", libhead, libend);
}

int main(int argc, const char *argv[]) {
  const char *p;
  char *t;
  int i;
  int ret;
  char buf[128];

  atexit(delete_late);
  
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
        case 'R':
          relocatable = 0;
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
        case 'k':
          keep_temps = 1;
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
          else if (strcmp(p, "--dynlib") == 0)
            dynlib = 1;
          else if (strcmp(p, "--nocrt0") == 0)
            nocrt0 = 1;
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
      build_command(0);
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
      reset_arguments();
    }
  } else {
    if (!relocatable || mode != MODE_LINK) {
      build_command(0);
      ret = do_command();
    } else {
      /* FIXME: if we have C source input binary output then split this into
         two tasks - build to .rel, then link twice */
      /* Link it at 0x0100 */
      progbase = 0x0100;
      build_command(1);
      ret = do_command();
      if (ret)
        exit(ret);
      reset_arguments();
      /* And at 0x0200 */
      progbase = 0x0200;
      build_command(2);
      ret = do_command();
      /* Then post process it */
    }
  }  
  if (mode != MODE_LINK || ret)
    exit(ret);
  reset_arguments();


  if (relocatable) {
    char *r = relocmap(rootname, 1);
    add_argument("makebin");
    add_argument("-p");
    add_argument("-s");
    add_argument("65535");
    add_argument_delete(extendname("", r, "ihx"));
    add_argument_delete(t = extendname("", r, "bin"));
    add_delete_late(extendname("", r, "noi"));
    add_delete_late(extendname("", r, "lk"));
    //add_delete_late(extendname("", r, "map"));
    ret = do_command();
    if (ret)
      exit(ret);
    free(r);
    reset_arguments_nodelete();
    r = relocmap(rootname, 2);
    add_argument("makebin");
    add_argument("-p");
    add_argument("-s");
    add_argument("65535");
    add_argument_delete(extendname("", r, "ihx"));
    add_argument_delete(t = extendname("", r, "bin"));
    add_delete_late(extendname("", r, "noi"));
    add_delete_late(extendname("", r, "lk"));
    add_delete_late(extendname("", r, "map"));
    ret = do_command();
    if (ret)
      exit(ret);
    free(r);
    t = extendname("", rootname, "bin");
  } else {
    add_argument("makebin");
    add_argument("-p");
    add_argument("-s");
    add_argument("65535");
    add_argument(target);
    add_argument_delete(t = extendname("", rootname, "bin"));
    add_delete_late(extendname("", rootname, "noi"));
    add_delete_late(extendname("", rootname, "lk"));
    ret = do_command();
    if (ret)
      exit(ret);
  }
  reset_arguments_nodelete();
  if (relocatable) {
    add_argument(FCC_DIR "/bin/relocbin");
    t = extendname("", relocmap(rootname, 1), "bin");
    add_argument_delete(t);
    t = extendname("", relocmap(rootname, 2), "bin");
    add_argument_delete(t);
    add_argument(extendname("", relocmap(rootname, 1), "map"));
  } else {
    add_argument(FCC_DIR "/bin/binman");
    snprintf(buf, sizeof(buf), "%x", progbase);
    add_argument(buf);
    add_argument_delete(t);
    add_argument(extendname("", rootname, "map"));
  }
  add_argument(rootname);
  ret = do_command();
  reset_arguments();
  exit(ret);
}
