/*
 * High level routines dealing with the output to the screen.
 */
#include <string.h>
#include "less.h"

extern int sigs;
extern int sc_width, sc_height;
extern int ul_width, ue_width;
extern int so_width, se_width;
extern int tabstop;
extern int twiddle;
extern char *line;
extern char *first_cmd;

/*
 * Display the line which is in the line buffer.
 */
void
put_line()
{
    register char *p;
    register int c;
    register int column;
    extern int auto_wrap, ignaw;

    if (sigs) {
	/*
	 * Don't output if a signal is pending.
	 */
	return;
    }

    if (line == NULL) {
	line = (twiddle) ? "~" : "";
    }

    column = 0;
    for (p = line;  *p != '\0';  p++) {
	switch (c = *p) {

	case UL_CHAR:
	    ul_enter();
	    column += ul_width;
	    break;

	case UE_CHAR:
	    ul_exit();
	    column += ue_width;
	    break;

	case '\t':
	    do {
		cputc(' ');
		column++;
	    } while ((column % tabstop) != 0);
	    break;

	case '\b':
	    putbs();
	    column--;
	    break;

	default:
	    if (c & 0200) {
		cputc('^');
		cputc(c & 0177);
		column += 2;
	    } else {
		cputc(c);
		column++;
	    }
	}
    }
    if (column < sc_width || !auto_wrap || ignaw) {
	cputc('\n');
    }
}

/*
 * Is a given character a "control" character?
 * {{ ASCII DEPENDENT }}
 */
int
control_char(c)
    int c;
{
    return (c < ' ' || c == '\177');
}

/*
 * Return the printable character used to identify a control character
 * (printed after a carat; e.g. '\3' => "^C").
 * {{ ASCII DEPENDENT }}
 */
int
carat_char(c)
    int c;
{
    return ((c == '\177') ? '?' : (c | 0100));
}


static char obuf[1024];
static char *ob = obuf;

/*
 * Flush buffered output.
 */
void
flush()
{
    write(1, obuf, ob-obuf);
    ob = obuf;
}

/*
 * Discard buffered output.
 */
void
dropout()
{
    ob = obuf;
}

/*
 * Output a character.
 */
void
cputc(c)
    int c;
{
    if (ob >= &obuf[sizeof(obuf)]) {
	flush();
    }
    *ob++ = c;
}

/*
 * Output a string.
 */
void
cputs(s)
    register char *s;
{
    while (*s != '\0') {
	cputc(*s++);
    }
}

/*
 * Output a message in the lower left corner of the screen
 * and wait for carriage return.
 */
static char return_to_continue[] = "  (press RETURN)";

void
error(s)
    char *s;
{
    register int c;
    static char buf[2];

    lower_left();
    clear_eol();
    so_enter();
    cputs(s);
    cputs(return_to_continue);
    so_exit();

#if ONLY_RETURN
    while ((c = getc()) != '\n' && c != '\r') {
	bell();
    }
#else
    c = getc();
    if (c != '\n' && c != '\r' && c != ' ') {
	buf[0] = c;
	first_cmd = buf;
    }
#endif

    if (strlen(s) > sc_width) {
	repaint();
    }
}

int
error_width()
{
    /*
     * Don't use the last position, because some terminals
     * will scroll if you write in the last char of the last line.
     */
    return (sc_width - 
	    (sizeof(return_to_continue) + so_width + se_width + 1));
}
