
/*  program ADVENT.C    modified by LCC for V 1.43 by:
        altering buffer sizes
  
        Changed call of exec() to static call for Eco-C88 (BW)
        Added function initw() for Eco-C88                (BW)
	Made function init() static for Eco-C88 	  (BW)

May 1990 - Bob Withers
	- Ported code to Microsoft C V 5.10 and 6.00
	- Placed all global variables in header ADVENT.H
	- Removed most runtime variable initialization and replaced
	  by compile time initializers.
	- Added file ADVENTDB.C to completely replace all the adventure
	  data base files.  This keeps all data in memory and eliminates
	  all disk accesses during game play.  All functions in DATABASE.C
	  were modified to access in memory data.
	- Added code to support the BRIEF and VERBOSE verbs.
	- Added code to ignore CTRL-C signal.
	- Modified vocab search to use a binary search.
*/

#define DRIVER

#include "advent.h"
#include <signal.h>
static void init(void);
static void eadvent(void);
int main(int argc, char **argv)
{
	short rflag;		/* user restore request option */
	short dflag;		/* user restore request option */
	brief_sw = game.dbgflg = rflag = 0;
	signal(SIGINT, SIG_IGN);
	while (--argc > 0) {
		if ((argv[argc])[0] == '-' || (argv[argc])[0] == '/') {
			switch (tolower((argv[argc])[1])) {
			case 'r':
				++rflag;
				continue;
			case 'd':
				++game.dbgflg;
				writes("Debug enabled.\n");
				continue;
			case 'b':
				++brief_sw;
				writes("Brief mode enabled.\n");
				continue;
			default:
				writes("unknown flag: ");
				write(1, argv[argc] + 1, 1);
				nl();
				continue;
			}
		}
	}
	dflag = game.dbgflg;
	db_init();
	init();
	if (!rflag) {
		writes("\nDo you want to restore a saved game? (Y/N) ");

		do {
			char buf[2];
			getinp(buf, 2);
			rflag = toupper(buf[0]);
		} while (!(rflag == 'Y' || rflag == 'N'));
		if (rflag == 'N')
			rflag = 0;
	}
	if (rflag)
		restore();
	game.dbgflg = dflag;
	eadvent();
	return (0);
}


/*  Initialization							*/
static void init(void)
{
	game.dloc[DWARFMAX - 1] = game.chloc;
	return;
}


/*  restore saved game handler	*/
void restore(void)
{
	char username[64];
	int restfd;
	writes("What is your saved game name? ");
	getinp(username, 64);
	if (*username == 0) {
		writes("No name entered, restore request ignored.\n");
		return;
	}
	writes("\nOpening save file \"");
	writes(username);
	writes("\".\n");
	if ((restfd = open(username, O_RDONLY)) == -1) {
		writes("Sorry, can't open save file...\n");
		exit(1);
	}

	if (read(restfd, &game, sizeof(game)) != sizeof(game)) {
		writes("Can't read save file...\n");
		exit(1);
	}
	if (close(restfd) == -1)
		writes("warning -- can't close save file...\n");
	nl();
	game.saveflg = 0;
}

static void eadvent(void)
{
	srand(511);
	if (yes(65, 1, 0))
		game.limit = 1000;

	else
		game.limit = 330;
	while (!game.saveflg)
		turn();
	if (game.saveflg)
		saveadv();
	return;
}
