
/*  programs in DATABASE.C    no changes for V 1.43			*/

#include "advent.h"
static void DisplayText(uint16_t * tab);
static char db_buf[513];
static struct trav db_trav[16];	/* Trav for last location fetched */
static uint8_t db_ntrav;
static int db_lastloc;
static int db_fd;
static struct gameheader game_db;

static void dbstring(uint16_t * off)
{
	uint16_t s = off[1] - *off;
	if (lseek(db_fd, *off, SEEK_SET) == -1) {
		perror("lseek");
		exit(1);
	}
	if (s >= sizeof(db_buf)) {
		write(2, "stringbug\n", 11);
		exit(1);
	}
	if (read(db_fd, db_buf, s) != s) {
		perror("read");
		exit(1);
	}
	db_buf[s] = 0;
}

void db_init(void)
{
	db_fd = open(ADVDB_PATH, O_RDONLY);
	if (db_fd == -1) {
		perror("advent.db");
		exit(1);
	}
	if (read(db_fd, &game_db, sizeof(game_db)) != sizeof(game_db)) {
		perror("read");
		exit(1);
	}
}

void travcache(short loc)
{
	char *p;
	if (loc != db_lastloc) {
		dbstring(game_db.lshort + loc - 1);
		db_lastloc = loc;
		p = db_buf + strlen(db_buf) + 1;
		db_ntrav = *p++;
		memcpy(db_trav, p, sizeof(db_trav));
	}
}

/*  Routine to fill travel array for a given location.	*/
void gettrav(short loc)
{
	travcache(loc);
	pTravel = db_trav;
	sTravCnt = db_ntrav;
	return;
}


/*  Routine to request a yes or no answer to a question.		    */
short yes(short msg1, short msg2, short msg3)
{
	char answer[80];
	if (msg1)
		rspeak(msg1);
	writes(">");
	getinp(answer, 79);
	if (tolower(answer[0]) == 'n') {
		if (msg3)
			rspeak(msg3);
		return (0);
	}
	if (msg2)
		rspeak(msg2);
	return (1);
}


/*  Print a random message from database 6				    */
void rspeak(short msg)
{
	dbstring(game_db.msg + msg - 1);

#ifdef DEBUG
	if (game.dbgflg)
		fprintf(stderr, "** rspeak(%d) ** ", msg);

#endif				/*  */
	DisplayText(game_db.msg);
	return;
}


/*  Routine to print the message for an item in a given state.		    */
void pspeak(short item, short state)
{
	register char *p;
	char *s;

#ifdef DEBUG
	if (game.dbgflg)
		fprintf(stderr, "** pspeak(%d,%d) ** ", item, state);

#endif				/*  */
	dbstring(game_db.odesc + item - 1);
	p = db_buf;
	while (state > -1) {
		while (*p && '/' != *p)
			++p;
		if (NUL == *p)
			return;
		++p;
		--state;
	}

	s = p;
	while (TRUE) {
		if (NUL == *p || '/' == *p)
			break;
		++p;
	}
	write(1, s, p - s);
	nl();
	return;
}


/*  Print the long description of a location				    */
void desclg(short loc)
{
	dbstring(game_db.loclong + loc - 1);

#ifdef DEBUG
	if (game.dbgflg)
		fprintf(stderr, "** desclg(%d) ** ", loc);

#endif				/*  */
	DisplayText(game_db.loclong);
	nl();
}


/*  Print the short description of a location				    */
void descsh(short loc)
{
	travcache(loc);
#ifdef DEBUG
	if (game.dbgflg)
		fprintf(stderr, "** descsh(%d) ** ", loc);

#endif				/*  */
	DisplayText(game_db.lshort);
	nl();
}

static void DisplayText(uint16_t * tab)
{
	uint8_t m[6];
	int i;
	int n = 0;
	char *p;
	p = db_buf;

	/* Be careful - we can't re-use db_buf midstream */
	if (*p == '@') {
		p++;
		while (*p) {
			m[n++] = atoi(p);
			while (*p && ',' != *p)
				++p;
			if (',' == *p)
				++p;
		}
		for (i = 0; i < n; i++) {
			dbstring(&tab[m[i] - 1]);
			writes(db_buf);
			nl();
		}
	} else {
		writes(db_buf);
		nl();
	}
}


/*  routine to look up a vocabulary word.
        word is the word to look up.
        val  is the minimum acceptable value,
                if != 0 return %1000
*/
short vocab(char *word, short val)
{
	register short i;
	register short left, right;
	auto short sCmp;
	extern short sVocabCount;
	extern VOCABTAB VocabTab[];
	left = 0;
	right = sVocabCount - 1;
	while (right >= left) {
		i = (left + right) / 2;
		sCmp = *word - *VocabTab[i].pWord;
		if (0 == sCmp)
			sCmp = strcmp(word, VocabTab[i].pWord);
		if (0 == sCmp)
			return (val ? VocabTab[i].sWord % 1000 : VocabTab[i].sWord);
		if (sCmp < 0)
			right = i - 1;

		else
			left = i + 1;
	}
	return (-1);
}


/*
        Utility Routines
*/

/*
        Routine to test for darkness
*/
uint8_t dark(void)
{
	return (!(game.cond[game.loc] & LIGHT) && (!game.prop[LAMP] || !here(LAMP)));
}


/*
        Routine to tell if an item is present.
*/
uint8_t here(short item)
{
	return (game.place[item] == game.loc || toting(item));
}


/*
        Routine to tell if an item is being carried.
*/
uint8_t toting(short item)
{
	return (game.place[item] == -1);
}


/*
        Routine to tell if a location causes
        a forced move.
*/
uint8_t forced(short atloc)
{
	return (game.cond[atloc] == 2);
}


/*
        Routine true x% of the time.
*/
uint8_t pct(short x)
{
	return (rand() % 100 < x);
}


/*
        Routine to tell if player is on
        either side of a two sided object.
*/
uint8_t at(short item)
{
	return (game.place[item] == game.loc || game.fixed[item] == game.loc);
}


/*
        Routine to destroy an object
*/
void dstroy(short obj)
{
	move(obj, 0);
	return;
}


/*
        Routine to move an object
*/
void move(short obj, short where)
{
	auto short from;
	from = (obj < MAXOBJ) ? game.place[obj] : game.fixed[obj];
	if (from > 0 && from <= 300)
		carry(obj, from);
	drop(obj, where);
	return;
}


/*
        Juggle an object
        currently a no-op
*/
void juggle(short loc)
{
	loc = loc;		/* eliminate compiler warning */
	return;
}


/*
        Routine to carry an object
*/
void carry(short obj, short where)
{
	where = where;		/* eliminate compiler warning */
	if (obj < MAXOBJ) {
		if (game.place[obj] == -1)
			return;
		game.place[obj] = -1;
		++game.holding;
	}
	return;
}


/*
        Routine to drop an object
*/
void drop(short obj, short where)
{
	if (obj < MAXOBJ) {
		if (game.place[obj] == -1)
			--game.holding;
		game.place[obj] = where;
	}

	else
		game.fixed[obj - MAXOBJ] = where;
	return;
}


/*
        routine to move an object and return a
        value used to set the negated prop values
        for the repository.
*/
short put(short obj, short where, short pval)
{
	move(obj, where);
	return ((-1) - pval);
}


/*
        Routine to check for presence
        of dwarves..
*/
short dcheck(void)
{
	register short i;
	for (i = 1; i < (DWARFMAX - 1); ++i) {
		if (game.dloc[i] == game.loc)
			return (i);
	}
	return (0);
}


/*
        Determine liquid in the bottle
*/
short liq(void)
{
	auto short i, j;
	i = game.prop[BOTTLE];
	j = (-1) - i;
	return (liq2(i > j ? i : j));
}


/*
        Determine liquid at a location
*/
short liqloc(short loc)
{
	if (game.cond[loc] & LIQUID)
		return (liq2(game.cond[loc] & WATOIL));
	return (liq2(1));
}


/*
        Convert  0 to WATER
                 1 to nothing
                 2 to OIL
*/
short liq2(short pbottle)
{
	return ((1 - pbottle) * WATER + (pbottle >> 1) * (WATER + OIL));
}


/*
        Fatal error routine
*/
void bug(short n)
{
	writes("Fatal error ");
	writei(n);
	nl();
	exit(1);
}
