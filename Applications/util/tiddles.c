/*
 *	Tiddles: a tiny editor
 *
 *	Work in progress. Just keeping it in git while it is worked on
 */

struct line {
  uint32_t offset;		/* Offset in base file */
  uint32_t newoff;		/* Offset during reconstruct */
  uint8_t *ptr;			/* If in memory */
  uint8_t flags;
#define L_DIRTY		1
#define L_EXTEND	2	/* In the spew file not the original */
  uint8_t len;			/* Disc len */
  uint8_t mlen;			/* Length in memory */
  uint8_t mspc;			/* Allocation size in memory */
};


/* Multi purpose disc buffer */
static char ibuf[512];
static int ibuflen;
static char *ibufptr;
static int ibuffd;

/* Multipurpose second buffer */
static char lbuf[256];

static int in_fd, out_fd, spew_fd;
static off_t spew_pos;

/* Simple interface for buffered file reading */
static int ibuf_getmore(void)
{
  ibuflen = read(ibuffd, ibuf, 512);
  if (ibuflen <= 0)
    return 0;
  ibufptr = ibuf;
  return ibuflen;
}

static int getch(void)
{
  if (ibufptr == ibuf + ibuflen)
    if (ibuf_getmore(ibuffd) == 0)
      return -1;

  return *ibufptr++;
}

static int open_in(int in_fd)
{
  ibuffd = fd;
  return ibuf_getmore();
}

/* Tunable allocation strategy */

/* Bytes of spew space to allocate for a given line. We want to avoid
   excess but also avoid lots of extra spew creation */
static uint8_t spew_size(uint8_t l)
{
  if (l < 128)
    return 128;
  return 256;
}

/* Bytes of memory to allocate for a given line */
static uint8_t spew_size(uint8_t l)
{
  if (l < 48)
    return 64;
  if (l < 80)
    return 96;
  if (l < 128)
    return 160;
  return 256;
}

/*
 *	Add a record to the spew file and update its line information
 *	accordingly.
 */

static int append_spew(struct line *l)
{
  int bytes;
  if (lseek(spew_fd, spew_pos, 0)) != spew_pos)
    return -1;
  bytes = write(spew_fd, l->buf, l->mlen);
  if (bytes != l->mlen)
    return -1;
  l->offset = spew_pos;
  l->flags |= L_EXTEND;
  l->len = l->mlen;
  spew_pos += spew_size(l);
}

/*
 * Update a line in the disc file
 */

static int write_bytes(int ofd, int infd, struct line *l)
{
  uint8_t *bp = l->ptr;
  /* FIXME: needs buffer algorithms */
  
  if (bp != NULL)
    l->len = l->mlen;		/* Disc length becomes mem length */
  else {
    /* Doing a disc to disc transfer */
    bp = lbuf;
    if (infd == -1)
      panic("no in no buf");
    if (read(infd, lbuf, l->len) != len)
      return -1;
  }
  if (write(ofd, bp, l->len) != len)
    return -1;
  return 0;
}

/*
 *	Reconstruct the edits we have made into a single coherent
 *	file on out_fd. At this point spew can be discarded. in and out
 *	must not be the same.
 */

static int reconstruct(void)
{
  struct line *lp = lines;
  uint32_t pos = 0;		/* In output */
  int err;

  while(lp < lines_end) {
    if (lp->flags & L_EXTEND)
      err = write_bytes(out_fd, spew_fd, lp);
    else
      err = write_bytes(out_fd, in_fd, lp);
    if (err)
      return -1;
    lp->newoff = pos;
    pos += len;
    lp++;
  }
  
  lp = lines;
  while (lp < lines_end) {
    lp->offset = lp->newoff;
    lp->flags &= ~(L_DIRTY|L_EXTEND);
  }
  spew_pos = 0L;
  return 0;

}

/*
 * Write a line back to disc. If it fits in its existing space in the
 * spew file then put it back there. If not then add it to the spew file
 * and we will rebuild later. Don't write back to in, in is the input file
 * and sacrosanct until we correctly write out a final new file.
 */

static int write_back(struct line *l)
{
  int spc = spew_size(l->len);
  if (l->ptr == NULL)
    panic("dirty no ptr");
  if (!(l->flags & L_EXTEND) || spc < l->mlen) {
    if (append_spew(l) == -1)
      return -1;
  } else {
    /* Reuse existing spew */
    if (write_bytes(spew_fd, -1, l) == -1)
      return -1;
  }
  l->flags &= ~L_DIRTY;
  return 0;
}
    
/* Flush out the lines we don't need.
   We should be smarter about this - keep a pool measure and
   trim when we hit it ??
 */

static int trim_window(struct line *s, struct line *e)
{
  if (s == NULL)
    return;
  while (s < e) {
    if (s->flags & L_DIRTY) {
      if (writeback(s) == -1)
        return -1;
      free(s->buf);
      s->buf = NULL;
    }
    s++;
  }
}

/* Make sure the visible window of lines is present in memory */
static int load_window(struct line *l, int num)
{
  struct line *le = l + num;
  
  if (trim_buffer(viewstart, l))
    return -1;
  if (trim_buffer(le, viewend))
    return -1;

  while (l < le) {
    if (l->buf == NULL) {
      readin(l);
    l++;
  }
  viewstart = l;
  viewend = le;
  return 0;
}  
    
static int delete_line(struct line *l)
{
  /* We take a hit here but it saves a lot of data structure crap elsehwere.
     We could defer deletes but it makes things like goto harder, unless we
     defer until we next need a line finding etc .. still icky */
  if (l->ptr)
    free(l->ptr);
  memmove(l, l + 1, lines_end - (l + 1));
  lines_end--;
  /* FIXME: count this into spew rebuild hints ? */
  return 0;
}

static int insert_line(struct line *l)
{
  void *p;

  if (lines_end >= lines_realend) {
    /* We don't want to do this every few lines */
    lines_realend += 256;

    /* TODO: Flush everything we can here and retry on fail. If that
       fails we could write the entire thing out and restart the load
       and edit */

    p = malloc((lines_realend - lines) * sizeof(struct line));
    if (p == NULL) {
      lines_realend -= 256;
      /* Consider flushing the file entirely and reloading our window */
      return NULL;
    }
    memcpy(p, lines, (lines_end - lines) * sizeof(struct line));
  }
  lines_end++;
  memmove(l, l+1, lines_end - (l + 1));
  l->flags = L_DIRTY;
  l->offset = 0L;	/* Doesn't matter as will go to spew */
  l->ptr = malloc(64);
  l->len = 0;
  l->mlen = 1;
  *l->ptr = '\n';
  l->mspc = 64;
  if (l->ptr == NULL) {
    delete_line(l);
    return NULL;
  }
  return l;
}

static int line_insert(struct line *l, int off, uint8_t c)
{
  if (!l->ptr)
    panic("insnoptr");
  l->flags |= L_DIRTY;
  /* Maximum line size */
  if (off > 255)
    return -1;
  if (off >= l->mspc) {
    size = line_size(off);
    uint8_t *p = malloc(size);
    if (p == NULL)
      return -1;
    l->mspc = size;
    l->mlen++;
    memmove(p, l->ptr, off);
    p[off] = c;
    memmove(p + off + 1, l->ptr + off, l->mlen - off);
    free(l->ptr);
    l->ptr = p;
    return 0;
  }
  memmove(l->ptr + off + 1, l->ptr + off, l->mlen - off);
  l->ptr[off] = c;
  return 0;
}

static int line_delete(struct line *l, int off)
{
  memove(l->ptr + off, l->ptr + off + 1, l->mlen - off);
  l->mlen --;
  l->flags |= L_DIRTY;
  return 0;
}

static int line_wipe(struct line *l, int off)
{
  if (l->mlen) {
    l->mlen = 0;
    l->flags |= L_DIRTY;
  }
  return 0;
}

static int line_replace(struct line *l, int off, uint8_t c)
{
  uint8_t *p = l->ptr + off;
  if (off >= l->mlen)
    panic("reptl");
  if (*p != c) {
    *p = c;
    l->flags |= L_DIRTY;
  }
}

/* Join two lines */

static int line_join(struct line *a, struct line *b)
{
  int n = 0;
  int nlen = a->mlen + b->mlen;

  if (nlen > a->mspc) {
    uint8_t *p;
    size = line_size(nlen);
    p = malloc(size);
    if (p == NULL)
      return -1;
    a->mspc = size;
  }
  if (a->ptr[a->mlen-1] == '\n')
    a->mlen--;
  memcpy(a->ptr + a->mlen, b->ptr, b->mlen);
  a->mlen += b->mlen;
  return line_delete(b);
}

/* Load the input file: FIXME: what to do about overlong lines */

static int load_file(int infd)
{
  struct stat s;
  int lnum;		/* Guess of size */
  uint8_t *lp = lbuf;
  uint8_t *le = lbuf + sizeof(lbuf);
  int c;
  long pos = 0;
  long lpos;

  if (fstat(infd, &s) == -1 || !S_ISREG(s.st_mode))
    return -1;
  lnum = s.st_size / 32;
  lnum /= 32;
  lnum += 256;
  /* FIXME: overflow check ? */
  lines = malloc(lnum * sizeof(struct line));
  if (lines == NULL)
    return -1;
  memset(lines, 0, lnum * sizeof(struct line));
  lines_realend = lines + lnum;
  lines_end = lines;
  
  if (open_in(infd) == -1)
    return -1;

  lpos = pos;	/* Save offset */
  while((c = getch()) != -1) {
    if (c == '\n') {
      l = insert_line(lines_end);
      if (l == NULL)
        return -1;
      l->pos = lpos;
      l->len = pos - lpos + 1;	/* We keep the \n */
      lpos = pos + 1;
    } else {
      *lp++ = c;
      if (lp == le) {
        /* line too long ?? truncate or wrap ?? FIXME */
        lp--;
      }
    }
    pos++;
  }
  if (pos != lpos) {
    insert_line(lines_end);
    l->pos = lpos;
    l->len = pos - lpos;
  }
  return 0;
}

/* Video layer: we need termcap in the end */

void crlf(void)
{
  write(1, "\n");
}

void home(void)
{
  write(1, "\033H",2);
}

static char moveto_buf[4] = "\033Y  ";

void moveto(int y, int x)
{
  moveto_buf[2] = y + ' ';
  moveto_buf[3] = x + ' ';
  write(1, moveto_buf, 4);
}


void setcursor(void)
{
  moveto(cursory, cursorx);
  cursor_valid = 1;
}
  
/* FIXME: end line without \n ?? */

static void reload_window(void)
{
  load_window(cursorline, num_lines);
}

static void redraw_line(struct line *l)
{
  int n = l->mlen - viewleft;
  n--;
  if (n)
    write(1, l->ptr + viewleft, n);
  /* Actually it might be valid in a few cases.. so maybe check one day */
  cursorvalid = 0;
}

static void redraw_status()
{
}

/* FIXME: need to blank lines below the end of file ! */

static void redraw(void)
{
  struct line *l = viewstart;
  home();
  while(l < viewend)
    redraw_line(l);
    crlf();
    l++;
  }
  redraw_status();
  crlf();
}

static void redraw_down(void)
{
  struct line *l = cursorline;
  moveto(cursory, 0);
  while(l < viewend)
    redraw_line(l);
    crlf();
    l++;
  }
  redraw_status();
  crlf();
}

static void view_adjust(int shift)
{
   if (viewleft + shift < 0)
     shift = -viewleft;
   cursorx -= shift;
   viewleft += shift;
   async_dirty = 1;	/* Catch up display when we get a gap */
   cursorvalid = 0;
}

static void vert_adjust_up(void)
{
  int n = cursorline - lines;
  if (n > bottom_line / 2)
    n = bottom_line / 2;
  cursory += n;
  cursorline -= n;
  async_dirty = 1;
  cursorvalid = 0;
}

static void vert_adjust_down(void)
{
  int n = lines_end - cursorline;
  if (n > bottom_line / 2)
    n = bottom_line / 2;
  cursory -= n;
  cursorline += n;
  async_dirty = 1;
  cursorvalid = 0;
}


/* Special characters are not handled here yet */
static void ins_at_cursor(uint8_t c)
{

  if (c == '\n') {
    if(insert_line(cursorline + 1) == -1) {
      beep();
      return;
    }
    redraw_below();
    return;
  }
  
  /* FIXME: play with this - probably better to look ahead if line right
     is off screen */
  if (cursorx == right_column)
    /* Assume display >= 16 wide! */
    view_adjust(16);

  if (line_insert(cursorline, cursorx, c) == -1) {
    beep();
    return;
  }

  if (cursorx == cursorline->mlen - viewleft - 1 && cursorvalid)
    write(1, &c, 1);
  else if (cursorvalid) {
    /* Bytes left in line */
    int n = cursorline->mlen - viewleft - cursorx - 1;
    int r = right_column - cursorright;
    if (n > r)
      n = r;
    if (n) {
      write(1, cursorline->ptr + viewleft, n);
      cursorvalid = 0;
    }
  }
  else	/* Hard .. */
    redraw_line(cursorline);
  cursorx++;
}

static void del_at_cursor(uint8_t c)
{
  int i;
  if (cursorx == 0 && viewleft == 0) {
    if (cursorline == line)	/* Start of file */
      return;
    cursorline--;
    if (line_join(cursorline, cursorline + 1) == -1) {
      beep();
      return;
    }
    reload_window();
    redraw_down(cursorline);
    return;
  }
  if (cursorx < 16 && viewleft)
    view_adjust(-16);		/* Assumes we always work in 16s! */
  if (line_delete(cursorline, cursorx + viewleft) == -1) {
    beep();
    return;
  }
  if (cursorx == cursorline->mlen - viewleft - 1 && cursorvalid)
    backspace();
    write(1, " ", 1);
    backspace();
  /* Below is common in ins/del - split this bit out */
  } else if (cursorvalid) {
    /* Bytes left in line */
    int n = cursorline->mlen - viewleft - cursorx - 1;
    int r = right_column - cursorright;
    if (n > r)
      n = r;
    if (n) {
      write(1, cursorline->ptr + viewleft, n);
      cursorvalid = 0;
    }
  }
  else	/* Hard .. */
    redraw_line(cursorline);
}

static void cursor_left(void)
{
  if (cursorx < 16 && viewleft)
    view_adjust(-16);
  if (cursorx) {
    cursorx--;
    setcursor();
  }
  else
    cursor_up();
}

static void cursor_right(void)
{
  if (cursorx == right_column)
    view_adjust(16);
  if (cursorx < cursorline->mlen) {
    cursorx++;
    setcursor();
  }
  else
    cursor_down();
}

static void cursor_up(void)
{
  if (cursorline == line)
    return;

  if (cursory < 2 && viewdown)
    vert_adjust_up();
  cursory--;
  cursorline--;
  if (cursorx > cursorline->mlen)
    cursorx = cursorline->mlen;
  setcursor();
}

static void cursor_down(void)
{
  if (cursorline == line_end - 1)
    return;
  if (cursory >= bottom_row)	/* FIXME need to consider if lines below 
                                   screen and scroll earlier */
    vert_adjust_down();
  cursory++;
  cursorline++;
  if (cursorx > cursorline->mlen)
    cursorx = cursorline->mlen;
  setcursor();
}

