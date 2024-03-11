/*
 *  Lightweight markdown based man page system for Fuzix
 *
 *  Copyright(c) 2016 Alan Cox
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *	TODO
 *	- Use termcap not hard coded mode changes
 *
 *	Limits currently
 *	- no sublist support
 *	- '''' code markup is only permitted in block form
 *
 *	FIXME
 *	- Remove stdio usage
 *
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <sys/ioctl.h>

#define MAX_TABLE_COLS	16

#define TITLE "Fuzix Programmer's Manual"

#define WIDE	0
#define NARROW	1
#define COMPACT	2

static uint8_t para_indent[] = {7, 2, 1};
static uint8_t list_indent[] = {2, 1, 1};
static uint8_t nlist_indent[] = {4, 2, 1};
static uint8_t code_indent[] = {4, 2, 1};

static uint8_t mode = WIDE;	/* Mode, used for format adjustment */
static uint8_t literal;		/* Literal block copying */
static uint8_t in_olist;	/* In an ordered list */
static uint8_t in_ulist; 	/* In an unordered list */
static uint8_t in_nlist;	/* In a named list */
static uint8_t in_table;	/* In a table */
static uint8_t line_prio;	/* # level */
static uint8_t format[4];	/* Format flags */
static uint8_t bold_forced;	/* Force text bold */
static uint8_t indent;
static uint8_t listindent;	/* List indenting */
static uint16_t olist_count;
static uint8_t suppressnl;	/* Next newline shouldn't happen */

static uint8_t tcols;		/* Table columns */
static uint8_t trow;		/* Table row count */
static uint8_t theader;		/* Table header */
static uint8_t twidth[MAX_TABLE_COLS];
static uint8_t ttype[MAX_TABLE_COLS];
#define CENTRE	1
#define RIGHT	2

static uint8_t widthcount;	/* Used when measuring for tables */
static uint8_t counting;

/* Position on screen and of right of screen */
static uint8_t xright;
static uint8_t xpos;

static char wordbuf[64];
static char *wordptr = wordbuf;
static uint8_t wordsize;

struct string {
    struct string *next;
    char *s;
};

static struct string *tabhead, *tabnext;

#define max(a,b)	((a) > (b) ? (a) : (b))

static void normal_syntax(char *p);
static void wordflush(void);

/*
 *	Output logic: a bit clunky right now, and needs to line buffer
 *	the output data.
 */

/* Move to a given column - if possible */
static void move_column(uint8_t x)
{
    while(xpos < x) {
        write(1, " ", 1);
        xpos++;
    }
}        

/* Output data to the tty */
static void output_bytes(char *p, int len)
{
    if (xpos == 0)
        move_column(indent);
    else if (xpos)
        write(1, " ", 1);
    write(1, p, len);
    xpos += len + 1;
}

/* Copy data to the tty without change */
static void copy_literal(char *p)
{
    if (xpos == 0)
        move_column(indent);
    write(1, p, strlen(p));
    xpos += strlen(p);
}

/* Write one byte to the tty */
static void output_byte(char c)
{
    if (xpos == 0)
        move_column(indent);
    write(1, &c, 1);
    xpos++;
}

/* Always newline */
static void force_newline(void)
{
    xpos = 0;
    write(1, "\n", 1);
}

/* Newline unless we just did one */
static void newline(void)
{
    wordflush();
    if (xpos && !suppressnl)
        force_newline();
    suppressnl = 0;
}

/* FIXME: Replace these with termcap handlers */
static void bold_on(void)
{
    write(1, "\033[1m", 4);
}

static void bold_off(void)
{
    if (!bold_forced)
        write(1, "\033[0m", 4);
}

/* Bold is special - it can also be forced on. Handle this */
static void force_bold(int c)
{
    bold_forced = c;
    if (bold_forced)
        bold_on();
    else if (format[0] == 0)
        bold_off();
}

static void strike_on(void)
{
}

static void strike_off(void)
{
}

static void under_on(void)
{
    write(1, "\033[4m", 4);
}

static void under_off(void)
{
    write(1, "\033[0m", 4);
}

static int width(char *p)
{
    counting = 1;
    widthcount = 0;
    normal_syntax(p);
    counting = 0;
    wordptr = wordbuf;
    return widthcount + 1;
}

static void oom(void)
{
    write(2, "marksman: out of memory.\n", 25);
    exit(1);
}

static char *strdup_err(const char *x)
{
    char *p = strdup(x);
    if (p == NULL)
        oom();
    return p;
}

static char *get_text(void)
{
    struct string *s = tabhead;
    char *t;

    if (s == NULL)
        return NULL;

    t = s->s;
    tabhead = tabhead->next;
    free(s);
    return t;
}

static void add_text(char *p)
{
    struct string *s = malloc(sizeof(struct string));
    if (s == NULL)
        oom();
    s->s = p;
    s->next = NULL;
    if (tabhead == NULL) {
        tabhead = s;
        tabnext = s;
    } else {
        tabnext->next = s;
        tabnext = s;
    }
}

static void wordbyte(char c)
{
    if (wordptr != wordbuf + 64)
        *wordptr++ = c;
}

/*
 *	Flush a word to the output stream while doing the correct inline
 *	format conversions and handling justification
 */
static void wordflush(void)
{
    char *t = wordbuf;

    if (wordsize == 0)
        return;

    if (counting) {
        widthcount += wordsize + 1;
        wordsize = 0;
        return;
    }
    if (xpos + wordsize >= xright)
        force_newline();
    while (t != wordptr) {
        switch((uint8_t)*t) {
        case 0xFD:
            output_byte(*++t);
            break;
        case 0xFE:
            switch(*++t) {
            case 0:
                bold_off();
                break;
            case 1:
                strike_off();
                break;
            case 2:
                under_off();
            }
            break;
        case 0xFF:
            switch(*++t) {
            case 0:
                bold_on();
                break;
            case 1:
                strike_on();
                break;
            case 2:
                under_on();
            }
            break;
        default:
            output_byte(*t);
        }
        t++;
    }
    if (xpos < xright)
        output_byte(' ');
    wordptr = wordbuf;
    wordsize = 0;
}

/* The symbols that introduce inline markdown */
static char tokens[] = "*~_`";

/* Check if our opening markdown has a tail. A lot of markdown tools assume
   if not the text is literal. This is however expensive so we should drop
   that */
static int token_follows(char *p, char *q, int size)
{
    while(*p) {
        if (memcmp(p++, q, size) == 0)
            return 1;
    }
    return 0;
}

/* Extract and display the alternate text for URL and images */
static char *alttext(char *p, char *e)
{
    normal_syntax(p);
    p = strchr(e + 1, *e);
    if (p)
        p++;
    return p;
}

/* Process a line of markdown text with possible inline formatting changes */    
static void normal_syntax(char *p)
{
    char *t;
    int size;

    while(*p && isspace(*p))
        p++;

    while(*p) {
        /* Picture */
        if (memcmp(p, "![", 2) == 0) {
            char *e = strchr(p + 2, ']');
            if (e && (e[1] == '[' || e[1] == ')')) {
                *e = 0;
                p = alttext(p + 2 , e);
                continue;
            }
        }
        /* URL */
        if (*p == '[') {
            char *e = strchr(p + 1, ']');
            if (e) {
                /* [foo][bar] or [foo](bar) */
                if (e[1] == '[' || e[1] == ')') {
                    *e = 0;
                    p = alttext(p + 1, e);
                    continue;
                }
                /* This is a definition. If we ever add support for real URLs
                   and links we'd collect these here and make a list of them */
                /* [foo]: */
                if (e[1] == ':')
                    return;
                /* [foo] */
                /* This is using the text directly so skip this lot */
                alttext(p + 1, e);
                p = e + 1;
                continue;
            }
            /* Not valid URL */
        }

        /* Symbols and markdown inline */
        if (*p != '\\') {
            t = strchr(tokens, *p);
            size = 1;
        } else {
            t = NULL;
            if (p[1])
                p++;
            else {
                /* End of line is \ - supress a newline */
                suppressnl = 1;
                break;
            }
        }

        /* We've found markdown */
        if (t) {
            int n = t - tokens;
            if (*p == p[1])
                size = 2;
            if (format[n] == 1 || token_follows(p + size + 1, p, size))
                format[n] ^= 1;
            /* Encode the bits inline */
            wordbyte('\xFE' + format[n]);
            wordbyte(n);
            p += size;
        }
        else {
            /* Copy bytes into the buffer stream */
            if ((uint8_t)*p >= 0xFD)
                wordbyte((char)0xFD);
            if (isspace(*p)) {
                wordflush();
                p++;
                while(*p && isspace(*p))
                    p++;
            } else {
                wordbyte(*p);
                wordsize++;
                p++;
            }
        }
    }
    if (wordsize)
        wordflush();
}

/* If we are on a new line add one, if not then add two */
static void paragraph(void)
{
    newline();
    force_newline();
}

/* Header lines */
static void header_line(char *p)
{
    int n = 0;
    int oldi = indent;

    newline();
    force_newline();
    indent = 0;

    while(*p == '#' && n < 6) {
        p++;
        n++;
    }
    line_prio = n;
    if (line_prio)
        force_bold(1);
    normal_syntax(p);
    /* Must put this back after we newline */
    newline();
    indent = oldi;
    line_prio = 0;
    force_bold(0);
}

/* Source code */
static void code_line(char *p)
{
    if (literal == 0) {
        newline();
        literal = 1;
        indent += code_indent[mode];
    } else {
        literal = 0;
        newline();
        indent -= code_indent[mode];
    }
}

static void blockquote(char *p)
{
    /* Including the > */
    normal_syntax(p);
}

static void horizontal_rule(void)
{
    uint8_t oldi = indent;
    int i;
    paragraph();
    indent = 0;
    for (i = 0; i < xright; i++)
        output_byte('-');
    newline();       
    indent = oldi;
}

/*
 *	We have all of a table saved away as an ordered list of strings
 *	along with width and type data. Do a dumb reformat on it.
 */
static void table_complete(void)
{
    char *p;
    int t = 0;
    uint16_t pos;
    int row = 0;
    /* Decide on column widths, then print them and free them */
    in_table = 0;
    /* Our current implementation is dumb, we just align them. We also don't
       support centre/right align yet */
    /* Any line that is all dashes implies the line above is a title */
    pos = indent;
    while((p = get_text()) != NULL) {
        if (row != theader + 1) {
            if (row == theader)
                force_bold(1);
            if (ttype[t] & CENTRE) {
                move_column(pos + (twidth[t] - width(p))/2);
            } else if (ttype[t] & RIGHT) {
                move_column(pos + twidth[t] - width(p));
            }
            normal_syntax(p);
            force_bold(0);
            wordflush();
            pos += twidth[t];
            move_column(pos);
        }
        /* Move along to the next table row */
        if (t < tcols - 1) {
            t++;
        } else {
            /* Next line of table */
            pos = indent;
            newline();
            row++;
            t = 0;
        }
    }   
}

/*
 *	Look for --- lines in a table indicating the header, along with
 *	: markers for the type
 */
int header_type(char *p)
{
    int f = 0;
    int n = 0;

    while(*p && isspace(*p))
        p++;

    if (*p == ':') {
        f |= CENTRE;
        p++;
    }
    while(*p == '-') {
        p++;
        n++;
    }
    if (*p == ':') {
        f |= RIGHT;
        p++;
    }

    while(*p && isspace(*p))
        p++;

    /* Centre|Right is valid Centre only is not */
    if (f == CENTRE || n < 3 || *p)
        return -1; 
    return f;
}

/*
 *	Process a line of table data
 */
static void process_table(char *p)
{
    char *e;
    int t = 0;
    int hdr = 1;
    int ht;

    p++;
    /* Not | ended, not a table */
    e = p + strlen(p) - 1;
    if (*p == 0 || *e != '|') {
        if (in_table)
            table_complete();
        normal_syntax(p);
        return;
    }

    /* Set the max widths to 0 */
    if (!in_table) {
        newline();
        memset(&twidth, 0, sizeof(twidth));
        trow = 0;	/* Row we are on */
        theader = 255;	/* Headers row */
    }
    /* Walk each bar separated entry */
    while(*p && t < MAX_TABLE_COLS) {
        e = strchr(p, '|');
        if (e == NULL)
            break;
        *e = 0;
        if (theader == 255) {
            ht = header_type(p);
            if (ht == -1) {
                hdr = 0;
                ht = 0;
            }
            ttype[t] = ht;
        }
        add_text(strdup_err(p));
        twidth[t] = max(width(p), twidth[t]);
        p = e + 1;
        t++;
    }
    if (!in_table) {
        in_table = 1;
        tcols = t;
    } else {
        if (t > tcols) {
            fprintf(stderr, "table format error %s\n", p);
            exit(1);
        }
        while (t < tcols) {
            add_text("");
            t++;
        }
    }
    if (hdr && trow && theader == 0xFF)
        theader = trow - 1;
    trow++;
}

/*
 *	List types
 */
static void unordered_list(char *p)
{
    in_ulist = 1;
    listindent = list_indent[mode];
    newline();
    output_byte(*p);
    indent += listindent;
    normal_syntax(p + 2);
    indent -= listindent;
}

static void ulist_complete(void)
{
    in_ulist = 0;
    newline();
}

static void ordered_list(char *p)
{
    if (!in_olist) {
        in_olist = 1;
        olist_count = 1;
    }
    listindent = list_indent[mode]; /* FIXME: depends on number */
    newline();
    output_byte(*p);	/* FIXME: output number! */
    indent += listindent;
    normal_syntax(p);
    indent -= listindent;
}

static void olist_complete(void)
{
    in_olist = 0;
    newline();
}

static void named_list(char *p)
{
    if (!in_nlist) {
        in_nlist = 1;
    }
    newline();
    normal_syntax(p + 1);
    newline();
    listindent = nlist_indent[mode];
}

static void nlist_complete(void)
{
    in_nlist = 0;
    newline();
}

/*
 *	Process the markdown line by line looking for block markdown
 *	as well as handling any existing state and spotting ends of lists
 *	and tables.
 */
static void parse_line(char *p)
{
    /* Table lines */
    if (in_table && *p == '|') {
        process_table(p);
        return;
    }
    /* We have finished a table, so we can now render it */
    if (in_table) {
        table_complete();
    }
    /* Are we in the middle of a literal code block */
    if (literal && memcmp(p, "```",3)) {
        copy_literal(p);
        newline();
        return;
    }
    if (memcmp(p, "  ", 2)) {
        if (in_ulist && !(isspace(p[1]) &&
            (*p == '+' || *p == '*' || *p == '-')))
            ulist_complete();
        }
        /* Should allow multi-digit */
        if (in_olist && (!isspace(p[1]) || !isdigit(p[0]))) {
            olist_complete();
        if (in_nlist)
            nlist_complete();
    }
    
    /* Check line start to see what to do */
    switch(*p) {
    case 0:	/* blank line - paragraph */
        paragraph();
        return;
    case '#':	/* # sequence for header priority */
        header_line(p);
        return;
    case ' ':	/* four spaces: literal copy */
        if (memcmp(p, "    ", 4) == 0) {
            copy_literal(p + 4);
            newline();		/* Because the source had one we ate */
            return;
        }
        if (memcmp(p, "  ", 2) == 0) {
            /* List continuation */
            if (in_ulist || in_olist || in_nlist) {
                indent += listindent;
                normal_syntax(p + 2);
                indent -= listindent;
                return;
            }
            /* Just starting '  ' -- implies newline ?? */
            normal_syntax(p);
        }
        break;
    case '`':	/* three backticks - code (literal copy block enter/exit) */
        if (memcmp(p, "```", 3) == 0) {
            code_line(p);
            return;
        }
        break;
    case '>':	/* quoted text */
        if (memcmp(p, "> ", 2) == 0) {
            blockquote(p);
            return;
        }
        break;
    case '_':	/* horizontal rule - three variants */
        if (memcmp(p, "___", 3) == 0) {
            horizontal_rule();
            return;
        }
        break;
    case '-':
        if (memcmp(p, "---", 3) == 0) {
            horizontal_rule();
            return;
        }
    case '+':
        /* Could be an undordered list */
        if (isspace(p[1])) {
            unordered_list(p);
            return;
        }
        break;
    case '*':
        if (memcmp(p, "***", 3) == 0) {
            horizontal_rule();
            return;
        }
        /* Could be an undordered list */
        if (isspace(p[1])) {
            unordered_list(p);
            return;
        }
        break;
    case '|':
        process_table(p);
        return;
    case ':':
        named_list(p);
        return;
    default:
        /*FIXME: allow multi-digit */
        if (p[1] == ' ' && isdigit(*p)) {
            ordered_list(p);
            return;
        }
    }
    normal_syntax(p);
}

/*
 *	Test main for formatting up a page from stdin
 */
int main(int argc, char *argv[])
{
    char buf[512];
    struct winsize ws;
    char *p;
    size_t l;
    
    if (ioctl(1, TIOCGWINSZ, &ws) == 0)
        xright = ws.ws_col;
    else
        xright = 80;

    if (xright < 40)
        mode = COMPACT;
    else if (xright < 80)
        mode = NARROW;

    write(1, "\033[H\033[J", 6); 

    if (fgets(buf, sizeof(buf), stdin) == NULL)
        exit(0);
    p = strchr(buf, '\n');
    if (p)
            *p = 0;
    l = strlen(buf);
    output_bytes(buf, l);
    if (mode == WIDE) {
        move_column((xright - strlen(TITLE))/2);
        output_bytes(TITLE, strlen(TITLE));
    }
    if (mode != COMPACT) {
        move_column(xright - l);
        output_bytes(buf, l);
    }

    xpos = 0;
    indent = para_indent[mode];

    while(fgets(buf, sizeof(buf), stdin)) {
        p = strchr(buf, '\n');
        if (p)
            *p = 0;
        parse_line(buf);
    }
    parse_line("");
    return 0;
}
