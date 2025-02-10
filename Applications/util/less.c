/* A tiny and memory-efficient subset of the less(1) pager.
 * (c) 2025, Warren Toomey, GPL3 license.
 *
 * Known keys are q, b, f, u, d, j, k, g, <space>, <return>
 *
 * The program gets the terminal size using ioctl(... , TIOCGWINSZ).
 * If this is unavailable, is assumes 80 colums and 25 rows.
 *
 * Sequences such as x<backspace>x get converted to bold x.
 * Sequences such as x<backspace>_ get converted to underline x.
 *
 * Input lines are not cached in memory: we fseek() instead.
 * Input from stdin gets copied to a temporary file so that
 * we can fseek() on it. The temporary file goes away on exit().
 *
 * Limitations: if you go forward enough in a big file to
 * cause a malloc() failure, the program free()s memory,
 * and you won't be able to back to the start of the file.
 *
 */

#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <termcap.h>

#define DEVTTY "/dev/tty"	/* Name of the terminal device */
#define BUFLEN	512		/* Size of the line buffer */

int ttyfd;			/* The fd for the terminal */
int rows = 25, cols = 80;	/* The default size of the terminal */
int is_stdin = 0;		/* Is input from standard input? */

/* We keep track of line offsets from the input file */
/* in a doubly-linked list with the following nodes. */

struct lineposn {
  off_t offset;
  struct lineposn *prev;
  struct lineposn *next;
};

/* Global variables */
FILE *fp;				/* File to page through */
char buf[BUFLEN];			/* Line buffer */
struct lineposn *linehead = NULL;	/* Head of the line offset list */
struct lineposn *linetail = NULL;	/* Tail of the line offset list */
struct termios orig_termios;		/* Original blocking terminal setting */

/* Escape sequences, to be filled in */
char *cls = "";				/* Clear the screen */
char *home = "";			/* Move to top-left corner */
char *noattr = "";			/* No bold or underlining */
char *bold = "";			/* Bold on */
char *underline = "";			/* Underlining on */

/* This code can be used to test the situation */
/* where we run out of memory going forwards */
#undef MYMALLOC
#ifdef MYMALLOC
#define MAXMALLOCS 400
int malloc_cnt=0;

void *mymalloc(size_t size) {
  if (++malloc_cnt>=MAXMALLOCS) return(NULL);
  return(malloc(size));
}

void *mycalloc(size_t nmemb, size_t size) {
  if (++malloc_cnt>MAXMALLOCS) return(NULL);
  return(calloc(nmemb, size));
}

void myfree(void *ptr) {
  malloc_cnt--; free(ptr);
}

# define malloc mymalloc
# define calloc mycalloc
# define free   myfree
#endif

/* Print out an error message and die */
void error(char *msg) {
  fputs(msg, stderr); exit(1);
}

/* Set the terminal back to blocking and echo */
void reset_terminal() {
  tcsetattr(ttyfd, TCSANOW, &orig_termios);
}

/* Put the terminal into cbreak mode with no echo */
void set_cbreak() {
  struct termios t;

  /* Get the original terminal settings twice, */
  /* one for restoration later. */
  tcgetattr(ttyfd, &orig_termios);
  if (tcgetattr(ttyfd, &t) == -1)
    error("Cannot tcgetattr\n");

  t.c_lflag &= ~(ICANON | ECHO);
  t.c_lflag |= ISIG;
  t.c_iflag &= ~ICRNL;
  t.c_cc[VMIN] = 1;		/* Character-at-a-time input */
  t.c_cc[VTIME] = 0;		/* with blocking */

  if (tcsetattr(ttyfd, TCSAFLUSH, &t) == -1)
    error("Cannot tcsetattr\n");

  /* Ensure we reset the terminal when we exit */
  atexit(reset_terminal);
}

/* Get the terminal's size */
void get_termsize() {
#ifdef TIOCGWINSZ
  struct winsize winsize;

  /* If we can't get it, we default to 25/80 */
  if (ioctl(ttyfd, TIOCGWINSZ, &winsize) == -1) return;
  cols = winsize.ws_col;
  rows = winsize.ws_row;
#endif
}

/* Try to read from stdin to the */
/* temporary file. The first call */
/* here does a blocking read. */
/* Successive calls are non-blocking. */
void read_stdin() {
  int cnt;

  /* Write any input to the end of the temporary file */
  if (fseek(fp, 0, SEEK_END) == -1)
    error("fseek error in read_stdin\n");

  while (1) {
    /* The first read will block. After that, */
    /* we do non-blocking reads. */
    cnt = read(0, buf, BUFLEN);
    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

    /* If nothing was read, now give up */
    if (cnt <= 0) break;
    fwrite(buf, 1, cnt, fp);
  }
}

/* Free some nodes from the beginning of the */
/* linked list so that we can allocate more */
/* memory. This means that we cannot */
/* go backwards through the whole file. */
void free_some_list(struct lineposn *keep) {
  int i, freecnt= rows;
  struct lineposn *last, *this;

  /* Start at the head */
  this= linehead;
  for (i= 0; i < freecnt; i++) {

    /* Keep at least one node */
    if (this->next==NULL) break;

    /* Don't free the keep node */
    if (this == keep) break;

    /* Move up and free the one behind */
    last= this; this= this->next;
    this->prev=NULL; free(last);
  }

  /* Point the head at the new list start */
  linehead= this;
}

/* Given a node and a count, try to find */
/* the offset of count more lines in the */
/* input file. Build nodes for each line, */
/* and append them to the linked list. */
void append_offsets(struct lineposn *this, int count) {
  int i;
  struct lineposn *next;

  if (this == NULL) return;

  /* Try to read in the line from the input at this position */
  if (fseek(fp, this->offset, SEEK_SET) == -1)
    error("fseek error in append_offsets\n");
  if (fgets(buf, BUFLEN, fp) == NULL) return;

  /* Loop getting more line offsets */
  for (i = 0; i < count; i++) {
    /* Save the offset after the fgets() above */
    next = (struct lineposn *) calloc(1, sizeof(struct lineposn));
    if (next == NULL) {
      /* We are out of memory. Free up some */
      /* nodes at the start of the list */
      free_some_list(this);
      next = (struct lineposn *) calloc(1, sizeof(struct lineposn));
      if (next == NULL)
        error("calloc error in append_offsets\n");
    }
    next->offset = ftell(fp);
    next->prev = this;
    this->next = next;
    this = next;

    /* Now try to read another line in */
    if (fgets(buf, BUFLEN, fp) == NULL) break;
  }

  /* Make this the end of the list */
  linetail = this;
}

/* We keep a buffer of actual characters to print out */
/* and a corresponding buffer of character attributes. */
/* These get malloc()d to hold cols amount of data. */

char *linebuf = NULL;
char *attrbuf = NULL;

#define NOATTR		0	/* Character has no attributes */
#define ISBOLD		1	/* Character is bold */
#define ISUNDER		2	/* Character is underlined */

/* Print out a single line. The cursor has been positioned */
/* at the start of the correct line on the screen. */
/* We have to interpret any backspace character in the input buffer */
void paint_line(int new_line) {
  char *lineptr = linebuf;
  char *attrptr = attrbuf;
  char *bufptr;
  char *lineend = &linebuf[cols - 1];	/* One before the end of the linebuf */
  int thisattr = 0, lastattr = 0;
  lineend++;

  /* Read the line in from the file */
  if (fgets(buf, BUFLEN, fp) == NULL) return;
  bufptr = buf;

  /* Clear out the line and attribute buffers */
  memset(linebuf, 0, cols + 1);
  memset(attrbuf, 0, cols + 1);

  /* Deal with as many real characters as we can fit into the linebuf */
  while (lineptr < lineend) {
    /* End the loop when we hit a newline */
    if (*bufptr == '\n') break;

    /* Move backwards if we find a backspace. Don't go below zero. */
    if (*bufptr == '\b') {
      lineptr = (lineptr == linebuf) ? linebuf : lineptr - 1;
      attrptr = (attrptr == attrbuf) ? attrptr : attrptr - 1;
      bufptr++; continue;
    }

    /* If there is nothing yet in the linebuf, add this character */
    if (*lineptr == 0) {
      *(lineptr++) = *(bufptr++);
      attrptr++; continue;
    }

    /* Now the fun begins. We have a character already in the linebuf */
    /* at this position. If it's the same as this character, mark it */
    /* as being bold. */
    if (*lineptr == *bufptr) {
      *attrptr |= ISBOLD;
      bufptr++; lineptr++; attrptr++; continue;
    }

    /* If the character in the linebuf is an underscore, replace it */
    /* with this character and mark it as underlined. */
    if (*lineptr == '_') {
      *lineptr++ = *bufptr++;
      *attrptr |= ISUNDER; attrptr++; continue;
    }

    /* If this character is an underscore, keep the existing character */
    /* and mark it as underlined. */
    if (*bufptr == '_') {
      *attrptr |= ISUNDER;
      attrptr++; bufptr++; lineptr++; continue;
    }

    /* At this point, we have completely different characters; what to do? */
    /* Replace the old character with the new one and leave it at that. */
    *lineptr++ = *bufptr++; attrptr++;
  }

  /* Now print out the line, inserting VT100 escape sequences as required */
  for (lineptr = linebuf, attrptr = attrbuf; *lineptr; lineptr++, attrptr++) {

    /* If the last character's attributes differ from this one, */
    /* turn off all VT100 attributes */
    thisattr = *attrptr;
    if (lastattr != thisattr) {
      fputs(noattr, stdout);

      /* If this character is bold, send the VT100 bold sequence */
      if ((thisattr & ISBOLD) == ISBOLD) fputs(bold, stdout);

      /* If this character is underline, send the VT100 underline sequence */
      if ((thisattr & ISUNDER) == ISUNDER) fputs(underline, stdout);
    }

    /* Now put out the character and save the character's attributes */
    fputc(*lineptr, stdout); lastattr = thisattr;
  }

  /* Turn off attributes if the last character had them on */
  /* and put out a newline */
  if (lastattr != NOATTR) fputs(noattr, stdout);
  if (new_line) fputc('\n', stdout);
}

/* Given a node in the doubly linked list, */
/* output 'rows' lines from this point onwards. */
void paint_screen(struct lineposn *this) {
  int i, new_line;
  long offset = this->offset;

  /* Clear the screen and move to the top left corner */
  fputs(cls, stdout); fputs(home, stdout);

  /* Move to the file offset for the first line */
  if (fseek(fp, offset, SEEK_SET) == -1)
    error("fseek error in paint_screen\n");

  /* Print out each line. Don't do a newline on the last one */
  for (i = 0, new_line = 1; i < rows; i++) {
    if (this == NULL) return;
    if (this->next == NULL) append_offsets(this, rows - i);
    if (i == rows - 1) new_line = 0;
    paint_line(new_line);
    this = this->next;
  }
}

/* Reposition the current view of the file */
struct lineposn *reposition(struct lineposn *this, int count) {
  int i;
  struct lineposn *last;

  if (count == 0) return (this);

  if (count > 0)
    for (i = 0; i < count; i++) {
      /* Get more offsets if we don't have any */
      if (this->next == NULL) append_offsets(this, count - i);
      last = this; this = this->next;

      if (this == NULL) {	/* Too far, go back a line */
	this = last; break;
      }
  } else {
    count = -count;
    for (i = 0; i < count; i++) {
      last = this; this = this->prev;
      if (this == NULL) {	/* Too far, go forward a line */
	this = last; break;
      }
    }
  }

  return (this);
}

int main(int argc, char *argv[]) {
  char *tmpfile = strdup("/tmp/less.XXXXXX");
  struct lineposn *this;
  int looping = 1;
  int fd;
  char ch;
  char *tcapbuf, *cptr;
  char *dptr, *eptr;		/* Not used but needed */

  /* Check the arguments */
  if (argc > 2) {
    fprintf(stderr, "Usage: less [filename]\n"); exit(1);
  }

  /* Open the terminal */
  if ((ttyfd = open(DEVTTY, O_RDONLY)) == -1)
    error("Unable to open " DEVTTY ", exiting\n");

  /* Get the terminal's size. */
  /* Put the terminal into cbreak mode */
  /* and start at line 1 */
  get_termsize();
  set_cbreak();
  this = linehead;

  /* Get the escape sequences for the given TERM */
  tcapbuf = (char *) malloc(1024);
  eptr= dptr = (char *) malloc(256);
  if (dptr == NULL)
    perror("Unable to malloc a termcap buffer\n");

  /* We have a termcap entry */
  if (tgetent(tcapbuf, getenv("TERM"))) {
    /* Copy these escape sequences if they exist */
    cptr= tgetstr("cl", &dptr);
    if (cptr != NULL) cls = strdup(cptr);
    cptr= tgetstr("ho", &dptr);
    if (cptr != NULL) home = strdup(cptr);
    cptr= tgetstr("me", &dptr);
    if (cptr != NULL) noattr = strdup(cptr);
    cptr= tgetstr("md", &dptr);
    if (cptr != NULL) bold = strdup(cptr);
    cptr= tgetstr("us", &dptr);
    if (cptr != NULL) underline = strdup(cptr);
  }
  free(tcapbuf);
  free(eptr);

  /* Allocate buffers to paint each line */
  linebuf = (char *) malloc(cols + 1);
  attrbuf = (char *) malloc(cols + 1);
  if (linebuf == NULL || attrbuf == NULL)
    error("malloc err with linebuf\n");

  /* If we have an argument, try to open that file. */
  /* Otherwise make a temporary file to hold the content */
  if (argc == 2) {
    if ((fp = fopen(argv[1], "r")) == NULL) {
      fprintf(stderr, "Unable to open %s\n", argv[1]); exit(1);
    }
  } else {
    is_stdin = 1;
    fd = mkstemp(tmpfile);
    unlink(tmpfile);		/* Unlink so it goes away on exit */
    fp = fdopen(fd, "w+");

    /* Try to read something from stdin. */
    /* Do a blocking read so there is something to display */
    read_stdin();
  }

  /* Start with a single node at offset 0 */
  /* and get enough line offsets for the first screen */
  this = (struct lineposn *) calloc(1, sizeof(struct lineposn));
  if (this == NULL) error("calloc error in main\n");
  linehead = linetail = this;
  append_offsets(this, rows);

  /* Get a command and deal with it */
  while (looping) {

    /* Try to read from stdin if that's our input. */
    /* Do a non-blocking read this time */
    if (is_stdin) read_stdin();

    /* Draw a page of the file */
    paint_screen(this);

    /* Get the next command from the user */
    if (read(ttyfd, &ch, 1) != 1)
      error("User input failed\n");

    switch (ch) {
    case 'q':			/* Quit the pager */
      looping = 0;
      break;
    case 'f':			/* Move down a screen */
    case ' ':
      this = reposition(this, rows);
      break;
    case 'b':			/* Move up a screen */
      this = reposition(this, -rows);
      break;
    case 'd':			/* Move down half a screen */
      this = reposition(this, rows / 2);
      break;
    case 'u':			/* Move up half a screen */
      this = reposition(this, -(rows / 2));
      break;
    case 'j':			/* Down one line */
    case '\n':
    case '\r':
      this = reposition(this, 1);
      break;
    case 'k':			/* Up one line */
      this = reposition(this, -1);
      break;
    case 'g':			/* Start of the input */
      this = linehead;
      break;
    }
  }
  fputc('\n', stdout); return (0);
}
