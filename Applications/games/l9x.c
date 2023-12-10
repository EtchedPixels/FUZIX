/*
 * (C) Copright 2015 Alan Cox
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Parts of this code are based upon poking through level9, the Level 9
 * interpreter by Glen Summers et al.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <termios.h>

#define VIRTUAL_GAME

/*
 *	Defines
 *
 *	VIRTUAL_GAME	:	Page the game from disc file
 *	STACKSIZE	:	Override stack size default (256)
 *	LISTSIZE	:	Override list size default (1024)
 *	TEXT_VERSION1	:	Interpreter for early stype text strings
 */

#ifndef STACKSIZE
#define STACKSIZE	256	/* Probably needs more for later games */
#endif
#ifndef LISTSIZE
#define LISTSIZE	1024	/* Later games need more */
#endif

#ifdef VIRTUAL_GAME

static uint8_t game[32];
#define game_base ((uint8_t *)NULL)

#else

/* Running from memory directly */
#define getb(x)	*(x)
static uint8_t game[27000];
#define game_base game

#endif

static int gamefile;

static uint16_t gamesize;

static uint8_t *messages;
static uint8_t *worddict;
static uint8_t *dictionary;
static uint8_t *exitmap;
static uint8_t *pc;
static uint8_t *pcbase;

static uint8_t game_over;

static uint8_t opcode;

static struct {
  uint16_t c_hash;
  uint16_t c_pc;
  uint16_t c_sp;
  uint16_t c_stackbase[STACKSIZE];
  uint16_t c_variables[256];
  uint8_t c_lists[LISTSIZE];	/* Probably much bigger for later games */
} context;

#define variables context.c_variables
#define lists context.c_lists
#define stackbase context.c_stackbase

uint16_t *stack = stackbase;

static uint8_t *tables[16];
static uint8_t ttype[16];

static char buffer[80];
static uint8_t wordbuf[3];
static uint8_t wordcount;

static uint16_t seed;	/* Random numbers */

static void error(const char *p);

/*
 *	I/O routines.
 */

static char wbuf[80];
static int wbp = 0;
static int xpos = 0;
static uint8_t cols;

static void display_init(void)
{
  char *c;
#ifdef TIOCGWINSZ
  struct winsize w;
  if (ioctl(0, TIOCGWINSZ, &w) != -1) {
    cols = w.ws_col;
    return;
  }
#elif VTSIZE
  int16_t v = ioctl(0, VTSIZE, 0);
  if (v != -1) {
    cols = v & 0xFF;
    return;
  }
#endif
  c = getenv("COLS");
  cols = c ? atoi(c): 80;
  if (cols == 0)
    cols = 80;
}

static void display_exit(void)
{
}

static void flush_word(void)
{
  write(1, wbuf, wbp);
  xpos += wbp;
  wbp = 0;
}

static void char_out(char c)
{
  if (c == '\n') {
    flush_word();
    if (xpos)
      write(1, "\n", 1);
    xpos = 0;
    return;
  }
  if (c != ' ') {
    if (wbp < 80)
      wbuf[wbp++] = c;
    return;
  }
  if (xpos + wbp >= cols) {
    xpos = 0;
    write(1,"\n", 1);
  }
  flush_word();
  write(1," ", 1);
  xpos++;
}

static void string_out(const char *p)
{
  while(*p)
    char_out(*p++);
}

static void print_num(uint16_t v)
{
#ifdef __linux__
  char buf[9];
  snprintf(buf, 8, "%d", v);	/* FIXME: avoid expensive snprintf */
  string_out(buf);
#else
  string_out(_itoa(v));
#endif
}

static void read_line(void)
{
  int l = read(0, buffer, sizeof(buffer));
  if (l < 0)
    error("read");
  buffer[l] = 0;
  if (l && buffer[l-1] == '\n')
    buffer[l-1] = 0;
  xpos = 0;
}

static void read_filename(void)
{
  string_out("Filename: ");
  read_line();
}

static void print_char(uint8_t c)
{
  if (c == 0x25)
    c = '\n';
  else if (c == 0x5F)
    c = ' ';
  char_out(c);
}

static void error(const char *p)
{
  display_exit();
  write(2, p, strlen(p));
  write(2, "\n", 1);
  exit(1);
}

#ifdef VIRTUAL_GAME

#define NUM_PAGES	64		/* 16K */

static uint8_t last_ah;
static uint8_t *last_base;

#ifdef STATISTICS
static unsigned long slow;
static unsigned long miss;
static unsigned long fast;
#define STAT(x)	((x)++)
#else
#define STAT(x)
#endif

static uint8_t page_cache[NUM_PAGES][256];
static uint8_t page_addr[NUM_PAGES];
static uint8_t page_pri[NUM_PAGES];	/* 0 = unused , 1+ is use count */

static uint8_t page_alloc(void)
{
	uint8_t low = 255;
	uint8_t i, lnum = 0;
	for (i = 0; i < NUM_PAGES; i++) {
		if (page_pri[i] == 0)
			return i;
		if (page_pri[i] < low) {
			lnum = i;
			low = page_pri[i];
		}
	}
	return lnum;
}

static void page_sweep(void)
{
	uint8_t i;
	for (i = 0; i < NUM_PAGES; i++)
		if (page_pri[i] > 1)
			page_pri[i] /= 2;
}

static void page_load(uint8_t slot, uint8_t ah)
{
	page_addr[slot] = ah;
	page_pri[slot] = 0x80;
	/* Caution - last page is not packed so a short read isn't
	   always an error */
	if (lseek(gamefile, (ah << 8), SEEK_SET) < 0 ||
	    read(gamefile, page_cache[slot], 256) < 0) {
		error("pageload");
	}
	/* Quick hack if you want to simulate disk loading on smaller
	   devices */
/*	usleep(10000);*/	/* DEBUG */
}

static uint8_t page_find(uint8_t ah)
{
	uint8_t i;
	for (i = 0; i < NUM_PAGES; i++) {
		if (page_addr[i] == ah) {
			STAT(slow);
			page_pri[i] |= 0x80;
			return i;
		}
	}
	page_sweep();
	i = page_alloc();
	page_load(i, ah);
	STAT(miss);
	return i;
}

static uint8_t getb(uint8_t *p)
{
	uint16_t addr = (uint16_t)p;
	uint8_t ah = addr >> 8;
	uint8_t c;

	if (ah == last_ah) {
		STAT(fast);
		return last_base[addr&0xff];
	}

	/* Find the right buffer */
	c = page_find(ah);
	last_ah = ah;
	last_base = page_cache[c];
	return last_base[addr & 0xff];
}

#endif

#ifdef TEXT_VERSION1
/* Call initially with the messages as the pointer. Each message contains
   a mix of character codes and dictionary numbers giving pieces of text
   to substitute. The dictionary is permitted to self reference giving an
   extremely elegant compressor that beats the Infocom and AdventureSoft /
   Adveture International compressors while being smaller ! */

/* FIXME: for version 2 games they swapped the 1 markers for length bytes
   with an odd hack where a 0 length means 255 + nextbyte (unless 0 if so
   repeat */
static void decompress(uint8_t *p, uint16_t m)
{
  uint8_t d;
  /* Walk the table looking for 1 bytes and counting off our
     input */
  while(m--)
    while(getb(p++) != 1);
  while((d = getb(p++)) > 2) {
    if (d < 0x5E)
      print_char(d + 0x1d);
    else
      decompress(worddict, d - 0x5E);
  }
}
#else

static uint8_t *msglen(uint8_t *p, uint16_t *l)
{
  *l = 0;
  while(!getb(p)) {
    *l += 255;
    p++;
  }
  *l += getb(p++);
  return p;
}
  
static void decompress(uint8_t *p, uint16_t m)
{
  uint8_t d;
  uint16_t l;
  /* Walk the table skipping messages */
  if (m == 0)
    return;
  while(--m) {
    p = msglen(p, &l);
    p += l - 1;
  }
  p = msglen(p, &l);
  /* A 1 byte message means its 0 text chars long */
  while(--l) {
    d = getb(p++);
    if (d < 3)
      return;
    if (d < 0x5E)
      print_char(d + 0x1d);
    else
      decompress(worddict - 1, d - 0x5d);
  }
}
#endif

static void print_message(uint16_t m)
{
  decompress(messages, m);
}

/*
 *	More complex bits the engine has methods for
 */

static uint8_t reverse[] = {
  0x10, 0x14, 0x16, 0x17,
  0x11, 0x18, 0x12, 0x13,
  0x15, 0x1a, 0x19, 0x1c,
  0x1b
};

static void lookup_exit(void)
{
  uint8_t l = variables[getb(pc++)];
  uint8_t d = variables[getb(pc++)];
  uint8_t *p = exitmap;
  uint8_t v;
  uint8_t ls = l;

  /* Scan through the table finding 0x80 end markers */
  l--;		/* No entry 0 */
  while (l--) {
    do {
      v = getb(p);
      p += 2;
    } while (!(v & 0x80));
  }
  /* Now find our exit */
  /* Basically each entry is a word in the form
     [Last.1][BiDir.1][Flags.2][Exit.4][Target.8] */
  do {
    v = getb(p);
    if ((v & 0x0F) == d) {
      variables[getb(pc++)] = ((getb(p++)) >> 4) & 7;	/* Flag bits */
      variables[getb(pc++)] = getb(p++);
      return;
    }
    p+=2;
  } while(!(v & 0x80));
  /* Exits can be bidirectional - we have to now sweep the whole table looking
     for a backlinked exit */
  if (d <= 12) {
    d = reverse[d];
    p = exitmap;
    l = 1;
    do {
      v = getb(p++);
      if (getb(p++) == ls && ((v & 0x1f) == d)) {
        variables[getb(pc++)] = (v >> 4) & 7;
        variables[getb(pc++)] = l;
        return;
      }
      if (v & 0x80)
        l++;
    } while(getb(p));
  }
  variables[getb(pc++)] = 0;
  variables[getb(pc++)] = 0;
}

static uint8_t wordcmp(char *s, uint8_t *p, uint8_t *v)
{
  do {
    if (*s != 0 && toupper(*s++) != (getb(p) & 0x7F))
      return 0;
  } while(!(getb(p++) & 0x80));
  *v = getb(p);
  return 1;
}

static uint8_t matchword(char *s)
{
  uint8_t *p = dictionary;
  uint8_t v;
  
  do {
/*    outword(p); */
    if (wordcmp(s, p, &v) == 1)
      return v;
    /* Find the next word */
    while(getb(p) && !(getb(p) & 0x80))
      p++;
    p++;
    p++;
  } while ((getb(p) & 0x80) == 0);
  /* FIXME: correct code for non match check */
  return 0xFF;
}

static void do_input(void)
{
  uint8_t *w = wordbuf;
  char *p = buffer;
  char *s;

  wordcount = 0;
  read_line();

  while(*p) {
    while (isspace(*p))
      p++;
    /* Now at word start */
    wordcount++;
    /* Check - do we count unknown words */
    s = p;
    while(*p && !isspace(*p))
      p++;
    /* The text between s and p-1 is now the word */
    *p++ = 0;
    if (w < wordbuf + sizeof(wordbuf))
      *w++ = matchword(s);
  }

  /* Finally put the first 3 words and the count into variables */
  w = wordbuf;
  variables[getb(pc++)] = *w++;
  variables[getb(pc++)] = *w++;
  variables[getb(pc++)] = *w++;
  variables[getb(pc++)] = wordcount;
}

/* This is fairly mindless but will do for now */
static uint16_t hash(void)
{
  uint8_t h = 0;
  uint8_t *gp = game;
  while(gp < game + 32)
    h += *gp++;
  return h;
}

static char savefail[] = "Save failed\n";
static char loadfail[] = "Load failed\n";

static void save_game(void)
{
  int fd;
  read_filename();
  if (!*buffer)
    return;
  fd = open(buffer, O_WRONLY|O_TRUNC|O_CREAT, 0600);
  if (fd == -1) {
    string_out(savefail);
    return;
  }
  context.c_pc = pc - pcbase;
  context.c_sp = stack - stackbase;
  context.c_hash = hash();
  if (write(fd, &context, sizeof(context)) != sizeof(context))
    string_out(savefail);
  close(fd);
}

static void load_game(void)
{
  int fd;

  read_filename();
  if (!*buffer)
    return;
  fd = open(buffer, O_RDONLY);
  if (fd == -1) {
    string_out(loadfail);
    return;
  }
  if (read(fd, &context, sizeof(context)) != sizeof(context) ||
      context.c_hash != hash()) {
    string_out(loadfail);
    memset(lists, 0, sizeof(lists));
    memset(variables, 0, sizeof(variables));
    pc = pcbase;
  } else {
    pc = pcbase + context.c_pc;
    stack = stackbase + context.c_sp;
  }
  close(fd);
}

/*
 *	Implement the core Level 9 machine (for version 1 and 2 anyway)
 *
 *	This is an extremely elegant and very compact bytecode with
 *	various helpers for "game" things.
 */

static uint16_t constant(void)
{
  uint16_t r = getb(pc++);
  if (!(opcode & 0x40))
    r |= (getb(pc++)) << 8;
  return r;
}

static uint8_t *address(void)
{
  if (opcode & 0x20) {
    int8_t s = (int8_t)getb(pc++);
    return pc + s - 1;
  }
  pc += 2;
  return pcbase + getb(pc-2) + (getb(pc-1) << 8);
} 

static void skipaddress(void)
{
  if (!(opcode & 0x20))
    pc++;
  pc++;
}

/* List ops access a small fixed number of tables */
static void listop(void)
{
  uint8_t t = (opcode & 0x1F) + 1;
  uint8_t *base = tables[t];
  if (base == NULL)
    error("BADL");
  if (opcode & 0x20)
    base += variables[getb(pc++)];
  else
    base += getb(pc++);
  if ((base >= game_base && base < game_base + gamesize) ||
      (base >= lists && base < lists + sizeof(lists))) {
    if (!(opcode & 0x40)) {
      if (ttype[t])
        variables[getb(pc++)] = *base;
      else
        variables[getb(pc++)] = getb(base);
    } else { 
      if (ttype[t] == 0)
        error("WFLT");
      *base = variables[getb(pc++)];
    }
  } else {
    error("LFLT");
  }
}  

static void execute(void)
{
  uint8_t *base;
  uint8_t tmp;
  uint16_t tmp16;
  

  while(!game_over) {
    opcode = getb(pc++);
    if (opcode & 0x80)
      listop();
    else switch(opcode & 0x1f) {
      case 0:
        pc = address();
        break;
      case 1: {
          uint8_t *newpc = address();
          if (stack == stackbase + sizeof(stackbase))
            error("stack overflow");
          *stack++ = pc - pcbase;
          pc = newpc;
        }
        break;
      case 2:
        if (stack == stackbase)
          error("stack underflow");
        pc = pcbase + *--stack;
        break;
      case 3:
        print_num(variables[getb(pc++)]);
        break;
      case 4:
        print_message(variables[getb(pc++)]);
        break;
      case 5:
        print_message(constant());
        break;
      case 6:
        switch(getb(pc++)) {
          case 1:
            game_over = 1;
            /* FIXME: call driver in later game engines */
            break;
          case 2:
            /* Emulate the random number algorithm in the original */
            seed = (((seed << 8) + 0x0A - seed) << 2) + seed + 1;
            variables[getb(pc++)] = seed & 0xff;
            break;
          case 3:
            save_game();
            break;
          case 4:
            load_game();
            break;
          case 5:
            memset(variables, 0, sizeof(variables));
            break;
          case 6:
            stack = stackbase;
            break;
          default:
/*            fprintf(stderr, "Unknown driver function %d\n", pc[-1]); */
            error("unkndriv");
        }
        break;
      case 7:
        do_input();
        break;
      case 8:
        tmp16 = constant();
        variables[getb(pc++)] = tmp16;
        break;
      case 9:
        variables[getb(pc + 1)] = variables[getb(pc)];
        pc += 2;
        break;
      case 10:
        variables[getb(pc + 1)] += variables[getb(pc)];
        pc += 2;
        break;
      case 11:
        variables[getb(pc + 1)] -= variables[getb(pc)];
        pc += 2;
        break;
      case 14: /* This looks weird, but its basically a jump table */
        base = pcbase + (getb(pc) + (getb(pc + 1) << 8));
        base += 2 * variables[getb(pc + 2)];	/* 16bit entries * */
        pc = pcbase + getb(base) + (getb(base + 1) << 8);
        break;
      case 15:
        lookup_exit();
        break;
      case 16:
        /* These two are defined despite gcc whining. It doesn't matter
           which way around they get evaluated */
        if (variables[getb(pc++)] == variables[getb(pc++)])
          pc = address();
        else
          skipaddress();
        break;
      case 17:
        if (variables[getb(pc++)] != variables[getb(pc++)])
          pc = address();
        else
          skipaddress();
        break;
      case 18:
        tmp = getb(pc++);
        if (variables[tmp] < variables[getb(pc++)])
          pc = address();
        else
          skipaddress();
        break;
      case 19:
        tmp = getb(pc++);
        if (variables[tmp] > variables[getb(pc++)])
          pc = address();
        else
          skipaddress();
        break;
      case 24:
        if (variables[getb(pc++)] == constant())
          pc = address();
        else
          skipaddress();
        break;
      case 25:
        if (variables[getb(pc++)] != constant())
          pc = address();
        else
          skipaddress();
        break;
      case 26:
        if (variables[getb(pc++)] < constant())
          pc = address();
        else
          skipaddress();
        break;
      case 27:
        if (variables[getb(pc++)] > constant())
          pc = address();
        else
          skipaddress();
        break;
      case 21:
        /* clear screen */
        pc++;	/* value indicates screen to clear */
        break;
      case 22:
        /* picture */
        pc++;
        break;
      case 20:
        /* graphics mode */
      case 23:
        /* getnextobject */
      case 28:
        /* print input */      
      default:
/*        fprintf(stderr, "bad op %d\n", opcode); */
        error("badop");
    }
  }
}


int main(int argc, char *argv[])
{
  uint8_t off = 4;
  int i;
  
  if (argc == 1)
    error("l9x [game.dat]\n");

  gamefile = open(argv[1], O_RDONLY);
  if (gamefile == -1) {
    perror(argv[1]);
    exit(1);
  }
  /* FIXME: allocate via sbrk once removed stdio usage */
  if ((gamesize = read(gamefile, game, sizeof(game))) < 32)
    error("l9x: not a valid game\n");
#ifdef VIRTUAL_GAME
  gamesize = 0xff00;
  memset(page_addr, 0xff, sizeof(page_addr));
#else
  close(gamefile);
#endif

  /* Header starts with message and decompression dictionary */
  messages = game_base + (game[0] | (game[1] << 8));
  worddict = game_base + (game[2] | (game[3] << 8));
  /* Then the tables for list ops */
  for (i = 0; i  < 12; i++) {
    uint16_t v = game[off] | (game[off + 1] << 8);
    if (i != 11 && (v & 0x8000)) {
      tables[i] = lists + (v & 0x7FFF);
      ttype[i] = 1;
    } else
      tables[i] = game_base + v;
    off += 2;
  }
  /* Some of which have hard coded uses and always point into game */
  exitmap = tables[0];
  dictionary = tables[1];
  pcbase = pc = tables[11];
  /* 3 and 4 are used for getnextobject and friends on later games,
     9 is used for driver magic and ramsave stuff */
  
  display_init();
  
  seed = time(NULL);

  execute();

#ifdef STATISTICS
  printf("Fast %d Slow %d Miss %d\n", fast, slow, miss);
#endif
}
