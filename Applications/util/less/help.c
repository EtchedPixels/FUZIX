#include  "less.h"

/*
 * Display some help.
 * Help is in two pages.
 */
static void
help0()
{
    cputs("f, SPACE       Forward one screen.\n");
    cputs("b              Backward one screen.\n");
    cputs("e, j, CR    *  Forward N lines, default 1.\n");
    cputs("y, k        *  Backward N lines, default 1.\n");
    cputs("d           *  Forward N lines, default 10 or last N to d or u command.\n");
    cputs("u           *  Backward N lines, default 10 or last N to d or u command.\n");
    cputs("r              Repaint screen.\n");
    cputs("g           *  Go to line N, default 1.\n");
    cputs("G           *  Like g, but default is last line in file.\n");
    cputs("=              Print current file name\n");
    cputs("/pattern    *  Search forward for N-th occurence of pattern.\n");
    cputs("?pattern    *  Search backward for N-th occurence of pattern.\n");
    cputs("n           *  Repeat previous search (for N-th occurence).\n");
    cputs("q              Exit.\n");
    error("More help...");
}

static void
help1()
{
    char message[100];
    extern char all_options[];

    cputs("R              Repaint screen, discarding buffered input.\n");
    cputs("p, %        *  Position to N percent into the file.\n");
    cputs("m<letter>      Mark the current position with <letter>.\n");
    cputs("'<letter>      Return to a previously marked position.\n");
    sprintf(message, 
	 "-X             Toggle a flag (X may be one of \"%s\").\n", 
			    all_options);
    cputs(message);
    cputs("E [file]       Examine a new file.\n");
    cputs("N              Examine the next file (from the command line).\n");
    cputs("P              Examine the previous file (from the command line).\n");
    cputs("V              Print version number.\n");
#if SHELL_ESCAPE
    cputs("!command       Passes the command to a shell to be executed.\n");
#endif
#if EDITOR
    sprintf(message,
	 "v              Edit the current file with $EDITOR (default %s).\n",
			    EDIT_PGM);
    cputs(message);
#endif
    error("");
}

void
help()
{
    register int i;

    for (i = 0;  i < 2;  i++)
    {
	clear();
	cputs("Commands marked with * may be preceeded by a number, N.\n\n");

	switch (i) {
	case 0:		help0();	break;
	case 1:		help1();	break;
	}
    }
}
