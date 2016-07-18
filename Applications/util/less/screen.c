/*
 * Routines which deal with the characteristics of the terminal.
 * Uses termcap to be as terminal-independent as possible.
 *
 * {{ Someday this should be rewritten to use curses. }}
 */
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include "less.h"

/*
 * Strings passed to tputs() to do various terminal functions.
 */
static char
    *sc_pad,		/* Pad string */
    *sc_home,		/* Cursor home */
    *sc_addline,	/* Add line, scroll down following lines */
    *sc_lower_left,	/* Cursor to last line, first column */
    *sc_move,		/* General cursor positioning */
    *sc_clear,		/* Clear screen */
    *sc_eol_clear,	/* Clear to end of line */
    *sc_s_in,		/* Enter standout (highlighted) mode */
    *sc_s_out,		/* Exit standout mode */
    *sc_u_in,		/* Enter underline mode */
    *sc_u_out,		/* Exit underline mode */
    *sc_visual_bell,	/* Visual bell (flash screen) sequence */
    *sc_backspace,	/* Backspace cursor */
    *sc_init,		/* Startup terminal initialization */
    *sc_deinit;		/* Exit terminal de-intialization */
static int dumb;
static int hard;

int auto_wrap;		/* Terminal does \r\n when write past margin */
int ignaw;		/* Terminal ignores \n immediately after wrap */
int erase_char, kill_char; /* The user's erase and line-kill chars */
int sc_width, sc_height;	/* Height & width of screen */
int ul_width, ue_width;	/* Printing width of underline sequences */
int so_width, se_width;	/* Printing width of standout sequences */

/*
 * These two variables are sometimes defined in,
 * and needed by, the termcap library.
 * It may be necessary on some systems to declare them extern here.
 */
/*extern*/ short ospeed;	/* Terminal output baud rate */
/*extern*/ char PC;		/* Pad character */

extern int quiet;		/* If VERY_QUIET, use visual bell for bell */
extern int know_dumb;		/* Don't complain about a dumb terminal */
extern int back_scroll;
char *tgetstr();
char *tgoto();

/*
 * Change terminal to "raw mode", or restore to "normal" mode.
 * "Raw mode" means 
 *	1. An outstanding read will complete on receipt of a single keystroke.
 *	2. Input is not echoed.  
 *	3. On output, \n is mapped to \r\n.
 *	4. \t is NOT be expanded into spaces.
 *	5. Signal-causing characters such as ctrl-C (interrupt),
 *	   etc. are NOT disabled.
 * It doesn't matter whether an input \n is mapped to \r, or vice versa.
 */
void
raw_mode(on)
    int on;
{
    struct termios s;
    static struct termios save_term;

    if (on) {
	/*
	 * Get terminal modes.
	 */
	tcgetattr(2, &s);

	/*
	 * Save modes and set certain variables dependent on modes.
	 */
	save_term = s;
	ospeed = cfgetospeed(&s);
	erase_char = s.c_cc[VERASE];
	kill_char = s.c_cc[VKILL];

	/*
	 * Set the modes to the way we want them.
	 */
	s.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL);
	s.c_oflag |=  (OPOST|ONLCR|TAB3);
	s.c_oflag &= ~(OCRNL|ONLRET);
	s.c_cc[VMIN] = 1;
	s.c_cc[VTIME] = 0;
    } else {
	/*
	 * Restore saved modes.
	 */
	s = save_term;
    }
    tcsetattr(2, TCSANOW, &s);
}

static int couldnt = 0;

static void
cannot(s)
	char *s;
{
    if (know_dumb) {
	/* 
	 * He knows he has a dumb terminal, so don't tell him. 
	 */
	return;
    }
    printf("WARNING: terminal cannot \"%s\"\n", s);
    couldnt = 1;
}

/*
* Get terminal capabilities via termcap.
*/
void
get_term()
{
    char termbuf[1024];
    char *sp;
    static char sbuf[150];

    char *getenv();

    /*
     * Find out what kind of terminal this is.
     */
    if (tgetent(termbuf, getenv("TERM")) <= 0) {
	dumb = 1;
    }

    /*
     * Get size of the screen.
     */
    if (dumb || (sc_height = tgetnum("li")) < 0 || tgetflag("hc")) {
	/* Oh no, this is a hardcopy terminal. */
	hard = 1;
	sc_height = 24;
    }
    if (dumb || (sc_width = tgetnum("co")) < 0) {
	sc_width = 80;
    }

    auto_wrap = tgetflag("am");
    ignaw = tgetflag("xn");

    /*
     * Assumes termcap variable "sg" is the printing width of
     * the standout sequence, the end standout sequence,
     * the underline sequence, and the end underline sequence.
     */
    if ((ul_width = tgetnum("sg")) < 0) {
	ul_width = 0;
    }
    so_width = se_width = ue_width = ul_width;

    /*
     * Get various string-valued capabilities.
     */
    sp = sbuf;

    sc_pad = (dumb) ? NULL : tgetstr("pc", &sp);
    if (sc_pad != NULL) {
	PC = *sc_pad;
    }

    sc_init = (dumb) ? NULL : tgetstr("ti", &sp);
    if (sc_init == NULL) {
	sc_init = "";
    }

    sc_deinit= (dumb) ? NULL : tgetstr("te", &sp);
    if (sc_deinit == NULL) {
	sc_deinit = "";
    }

    sc_eol_clear = (dumb) ? NULL : tgetstr("ce", &sp);
    if (hard || sc_eol_clear == NULL || *sc_eol_clear == '\0') {
	cannot("clear to end of line");
	sc_eol_clear = "";
    }

    sc_clear = (dumb) ? NULL : tgetstr("cl", &sp);
    if (hard || sc_clear == NULL || *sc_clear == '\0') {
	cannot("clear screen");
	sc_clear = "\n\n";
    }

    sc_move = (dumb) ? NULL : tgetstr("cm", &sp);
    if (hard || sc_move == NULL || *sc_move == '\0') {
	/*
	 * This is not an error here, because we don't 
	 * always need sc_move.
	 * We need it only if we don't have home or lower-left.
	 */
	sc_move = "";
    }

    sc_s_in = (dumb) ? NULL : tgetstr("so", &sp);
    if (hard || sc_s_in == NULL) {
	sc_s_in = "";
    }

    sc_s_out = (dumb) ? NULL : tgetstr("se", &sp);
    if (hard || sc_s_out == NULL) {
	sc_s_out = "";
    }

    sc_u_in = (dumb) ? NULL : tgetstr("us", &sp);
    if (hard || sc_u_in == NULL) {
	sc_u_in = sc_s_in;
    }

    sc_u_out = (dumb) ? NULL : tgetstr("ue", &sp);
    if (hard || sc_u_out == NULL) {
	sc_u_out = sc_s_out;
    }

    sc_visual_bell = (dumb) ? NULL : tgetstr("vb", &sp);
    if (hard || sc_visual_bell == NULL) {
	sc_visual_bell = "";
    }

    sc_home = (dumb) ? NULL : tgetstr("ho", &sp);
    if (hard || sc_home == NULL || *sc_home == '\0') {
	if (*sc_move == '\0') {
	    cannot("home cursor");
	    /*
	     * This last resort for sc_home is supposed to
	     * be an up-arrow suggesting moving to the 
	     * top of the "virtual screen". (The one in
	     * your imagination as you try to use this on
	     * a hard copy terminal.)
	     */
	    sc_home = "|\b^";		
	} else {
	    /* 
	     * No "home" string,
	     * but we can use "move(0,0)".
	     */
	    strcpy(sp, tgoto(sc_move, 0, 0));
	    sc_home = sp;
	    sp += strlen(sp) + 1;
	}
    }

    sc_lower_left = (dumb) ? NULL : tgetstr("ll", &sp);
    if (hard || sc_lower_left == NULL || *sc_lower_left == '\0') {
	if (*sc_move == '\0') {
	    cannot("move cursor to lower left of screen");
	    sc_lower_left = "\r";
	} else {
	    /*
	     * No "lower-left" string, 
	     * but we can use "move(0,last-line)".
	     */
	    strcpy(sp, tgoto(sc_move, 0, sc_height-1));
	    sc_lower_left = sp;
	    sp += strlen(sp) + 1;
	}
    }

    /*
     * To add a line at top of screen and scroll the display down,
     * we use "al" (add line) or "sr" (scroll reverse).
     */
    if (dumb) {
	    sc_addline = NULL;
    } else if ((sc_addline = tgetstr("al", &sp)) == NULL ||
	     *sc_addline == '\0') {
	sc_addline = tgetstr("sr", &sp);
    }

    if (hard || sc_addline == NULL || *sc_addline == '\0') {
	cannot("scroll backwards");
	sc_addline = "";
	/* Force repaint on any backward movement */
	back_scroll = 0;
    }

    if (dumb || tgetflag("bs")) {
	    sc_backspace = "\b";
    } else {
	sc_backspace = tgetstr("bc", &sp);
	if (sc_backspace == NULL || *sc_backspace == '\0')
	    sc_backspace = "\b";
    }

    if (couldnt) {
	/* Give him time to read all the "cannot" messages. */
	error("");
    }
}


/*
* Below are the functions which perform all the 
* terminal-specific screen manipulation.
*/


/*
 * Initialize terminal
 */
void
init()
{
    tputs(sc_init, sc_height, cputc);
}

/*
 * Deinitialize terminal
 */
void
deinit()
{
    tputs(sc_deinit, sc_height, cputc);
}

/*
 * Home cursor (move to upper left corner of screen).
 */
void
home()
{
    tputs(sc_home, 1, cputc);
}

/*
 * Add a blank line (called with cursor at home).
 * Should scroll the display down.
 */
void
add_line()
{
    tputs(sc_addline, sc_height, cputc);
}

/*
 * Move cursor to lower left corner of screen.
 */
void
lower_left()
{
    tputs(sc_lower_left, 1, cputc);
}

/*
 * Ring the terminal bell.
 */
void
bell()
{
    if (quiet == VERY_QUIET) {
	vbell();
    } else {
	cputc('\7');
    }
}

/*
 * Output the "visual bell", if there is one.
 */
void
vbell()
{
    if (*sc_visual_bell == '\0') {
	return;
    }
    tputs(sc_visual_bell, sc_height, cputc);
}

/*
 * Clear the screen.
 */
void
clear()
{
    tputs(sc_clear, sc_height, cputc);
}

/*
 * Clear from the cursor to the end of the cursor's line.
 * {{ This must not move the cursor. }}
*/
void
clear_eol()
{
    tputs(sc_eol_clear, 1, cputc);
}

/*
 * Begin "standout" (bold, underline, or whatever).
 */
void
so_enter()
{
    tputs(sc_s_in, 1, cputc);
}

/*
 * End "standout".
 */
void
so_exit()
{
    tputs(sc_s_out, 1, cputc);
}

/*
 * Begin "underline" (hopefully real underlining, 
 * otherwise whatever the terminal provides).
 */
void
ul_enter()
{
    tputs(sc_u_in, 1, cputc);
}

/*
 * End "underline".
 */
void
ul_exit()
{
    tputs(sc_u_out, 1, cputc);
}

/*
 * Erase the character to the left of the cursor 
 * and move the cursor left.
 */
void
backspace()
{
    /* 
     * Try to erase the previous character by overstriking with a space.
     */
    tputs(sc_backspace, 1, cputc);
    cputc(' ');
    tputs(sc_backspace, 1, cputc);
}

/*
 * Output a plain backspace, without erasing the previous char.
 */
void
putbs()
{
    tputs(sc_backspace, 1, cputc);
}
