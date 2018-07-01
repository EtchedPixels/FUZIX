
/*    ADVENT.H    revised header for BDS c vers 1.43   */

/*                Revised for Eco-C88 V2.72 by Bob Withers
                  Defined all variables for driver routines and
                  altered header to declare them external for
                  all sub-modules.    BW - 09/14/85
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#define ADVDB_PATH		    "/usr/games/lib/advent.db"

#define NUL			    '\0'

#define MAXDIM(a)		    (sizeof(a) / sizeof(a[0]))

#ifndef TRUE
#define TRUE			    1
#define FALSE			    0
#endif

#define MAXLOC			    150
#define MAXOBJ			    100
#define WORDSIZE		    20

#define DWARFMAX		    7
#define MAXDIE			    3
#define MAXTRS			    79

#define READ_BIN		    "rb"
#define WRITE_BIN		    "wb"


#define VOCAB_OBJECT		    1000
#define VOCAB_VERB		    2000
#define VOCAB_MSG		    3000

/*
        Object definitions
*/
#define KEYS			    1
#define LAMP			    2
#define GRATE			    3
#define CAGE			    4
#define ROD			    5
#define ROD2			    6
#define STEPS			    7
#define BIRD			    8
#define DOOR			    9
#define PILLOW			    10
#define SNAKE			    11
#define FISSURE 		    12
#define TABLET			    13
#define CLAM			    14
#define OYSTER			    15
#define MAGAZINE		    16
#define DWARF			    17
#define KNIFE			    18
#define FOOD			    19
#define BOTTLE			    20
#define WATER			    21
#define OIL			    22
#define MIRROR			    23
#define PLANT			    24
#define PLANT2			    25
#define STALACTITE		    26
#define FIGURE			    27
#define AXE			    28
#define DRAWING 		    29
#define PIRATE			    30
#define DRAGON			    31
#define CHASM			    32
#define TROLL			    33
#define TROLL2			    34
#define BEAR			    35
#define MESSAGE 		    36
#define VOLCANO 		    37
#define VEND			    38
#define BATTERIES		    39
#define CARPET			    40
#define NUGGET			    50
#define DIAMONDS		    51
#define SILVER			    52
#define JEWELS			    53
#define COINS			    54
#define CHEST			    55
#define EGGS			    56
#define TRIDENT 		    57
#define VASE			    58
#define EMERALD 		    59
#define PYRAMID 		    60
#define PEARL			    61
#define RUG			    62
#define SPICES			    63
#define CHAIN			    64

/*
        Verb definitions
*/
#define NULLX			    21
#define BACK			    8
#define LOOK			    57
#define CAVE			    67
#define ENTRANCE		    64
#define DEPRESSION		    63

/*
        Action verb definitions
*/
#define TAKE			    1
#define DROP			    2
#define SAY			    3
#define OPEN			    4
#define NOTHING 		    5
#define LOCK			    6
#define ON			    7
#define OFF			    8
#define WAVE			    9
#define CALM			    10
#define WALK			    11
#define KILL			    12
#define POUR			    13
#define EAT			    14
#define DRINK			    15
#define RUB			    16
#define THROW			    17
#define QUIT			    18
#define FIND			    19
#define INVENTORY		    20
#define FEED			    21
#define FILL			    22
#define BLAST			    23
#define SCORE			    24
#define FOO			    25
#define BRIEF			    26
#define READ			    27
#define BREAK			    28
#define WAKE			    29
#define SUSPEND 		    30
#define HOURS			    31
#define LOG			    32
#define SAVE			    33
#define RESTORE 		    34
#define VERBOSE 		    35

/*
        Bits of array cond
        indicating location status
*/
#define LIGHT			    1
#define WATOIL			    2
#define LIQUID			    4
#define NOPIRAT 		    8
#define HINTC			    16
#define HINTB			    32
#define HINTS			    64
#define HINTM			    128
#define HINT			    240

/*
        Adventure global variables
*/

struct S_VocabTab {
	char *pWord;
	short sWord;
};
typedef struct S_VocabTab VOCABTAB;

struct trav {
	short tdest;
	short tverb;
	short tcond;
};
typedef struct trav TRAV;

struct travtab {
	TRAV *pTrav;		// trav array for location
	short sTrav;		// # entries for location
};
typedef struct travtab TRAVTAB;

struct gameheader {
	uint16_t msg[211];
	uint16_t lshort[141];
	uint16_t odesc[65];
	uint16_t loclong[148];
};

#ifdef DRIVER
#define CLASS
#define INIT(x) 		= x
#else
#define CLASS			extern
#define INIT(x)
#endif

CLASS short brief_sw;
/*
        Database variables
*/
CLASS TRAV *pTravel;		/* travel array & count for */
CLASS short sTravCnt;		/* the current location     */
CLASS short actmsg[32]		/* action messages */
#ifdef DRIVER
    = {
	0, 24, 29, 0, 33, 0, 33, 38, 38, 42,	/*  0 -  9 */
	14, 43, 110, 29, 110, 73, 75, 29, 13, 59,	/* 10 - 19 */
	59, 174, 109, 67, 13, 147, 155, 195, 146, 110,	/* 20 - 29 */
	13, 13			/* 30 - 31 */
};
#else
;
#endif

/*
        English variables
*/
CLASS short verb;
CLASS short object;
CLASS short motion;
CLASS char word1[WORDSIZE];
CLASS char word2[WORDSIZE];

/*
        Play variables
*/
CLASS short turns INIT(0);
CLASS short loc INIT(1);
CLASS short oldloc INIT(1);
CLASS short oldloc2 INIT(1);
CLASS short newloc INIT(3);
CLASS short cond[MAXLOC]	/* location status    */
#ifdef DRIVER
    = {
	0, 5, 1, 5, 5, 1, 1, 5, 17, 1,	/*   0 -   9 */
	1, 0, 0, 32, 0, 0, 2, 0, 0, 64,	/*  10 -  19 */
	2, 2, 2, 0, 6, 0, 2, 0, 0, 0,	/*  20 -  29 */
	0, 2, 2, 0, 0, 0, 0, 0, 4, 0,	/*  30 -  39 */
	2, 0, 128, 128, 128, 128, 136, 136, 136, 128,	/*  40 -  49 */
	128, 128, 128, 136, 128, 136, 0, 8, 0, 2,	/*  50 -  59 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/*  60 -  69 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 2,	/*  70 -  79 */
	128, 128, 136, 0, 0, 8, 136, 128, 0, 2,	/*  80 -  89 */
	2, 0, 0, 0, 0, 4, 0, 0, 0, 0,	/*  90 -  99 */
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 100 - 109 */
	0, 0, 0, 4, 0, 1, 1, 0, 0, 0,	/* 110 - 119 */
	0, 0, 8, 8, 8, 8, 8, 8, 8, 8,	/* 120 - 129 */
	8, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 130 - 139 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 140 - 149 */
};
#else
;
#endif

CLASS short place[MAXOBJ]	/* object location    */
#ifdef DRIVER
    = {
	0, 3, 3, 8, 10, 11, 0, 14, 13, 94,	/*  0 -  9 */
	96, 19, 17, 101, 103, 0, 106, 0, 0, 3,	/* 10 - 19 */
	3, 0, 0, 109, 25, 23, 111, 35, 0, 97,	/* 20 - 29 */
	0, 119, 117, 117, 0, 130, 0, 126, 140, 0,	/* 30 - 39 */
	96, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 40 - 49 */
	18, 27, 28, 29, 30, 0, 92, 95, 97, 100,	/* 50 - 59 */
	101, 0, 119, 127, 130, 0, 0, 0, 0, 0,	/* 60 - 69 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 70 - 79 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 80 - 89 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 90 - 99 */
};
#else
;
#endif

CLASS short fixed[MAXOBJ]	/* second object loc  */
#ifdef DRIVER
    = {
	0, 0, 0, 9, 0, 0, 0, 15, 0, -1,	/*  0 -  9 */
	0, -1, 27, -1, 0, 0, 0, -1, 0, 0,	/* 10 - 19 */
	0, 0, 0, -1, -1, 67, -1, 110, 0, -1,	/* 20 - 29 */
	-1, 121, 122, 122, 0, -1, -1, -1, -1, 0,	/* 30 - 39 */
	-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 40 - 49 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 50 - 59 */
	0, 0, 121, 0, -1, 0, 0, 0, 0, 0,	/* 60 - 69 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 70 - 79 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 80 - 89 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 90 - 99 */
};
#else
;
#endif

CLASS short visited[MAXLOC];	/* >0 if has been here */
CLASS short prop[MAXOBJ]	/* status of object   */
#ifdef DRIVER
    = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/*  0 -  9 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 10 - 19 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 20 - 29 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 30 - 39 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 40 - 49 */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 50 - 59 */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 60 - 69 */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 70 - 79 */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 80 - 89 */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 90 - 99 */
};
#else
;
#endif

CLASS short tally INIT(15);	/* item counts        */
CLASS short tally2 INIT(0);
CLASS short limit INIT(100);	/* time limit         */
CLASS short lmwarn INIT(0);	/* lamp warning flag  */
CLASS uint8_t wzdark INIT(FALSE);
CLASS uint8_t closing INIT(FALSE);
CLASS uint8_t closed INIT(FALSE);
CLASS short holding INIT(0);	/* count of held items */
CLASS short detail INIT(0);	/* LOOK count         */
CLASS short knfloc INIT(0);	/* knife location     */
CLASS short clock1 INIT(30);	/* timing variables   */
CLASS short clock2 INIT(50);
CLASS short panic INIT(0);
CLASS short dloc[DWARFMAX]	/* dwarf locations    */
#ifdef DRIVER
    = {
	0, 19, 27, 33, 44, 64, 0	/*  0 - 6  */
};
#else
;
#endif

CLASS short dflag INIT(0);	/* dwarf flag         */
CLASS short dseen[DWARFMAX];	/* dwarf seen flag    */
CLASS short odloc[DWARFMAX];	/* dwarf old locs     */
CLASS short daltloc INIT(18);	/* alt appearance     */
CLASS short dkill INIT(0);	/* dwarves killed     */
CLASS short chloc INIT(114);	/* chest locations    */
CLASS short chloc2 INIT(140);
CLASS short bonus INIT(0);	/* to pass to end     */
CLASS short numdie INIT(0);	/* number of deaths   */
CLASS short object1;		/* to help intrans.   */
CLASS uint8_t gaveup INIT(FALSE);
CLASS short foobar INIT(0);	/* fie fie foe foo... */
CLASS uint8_t saveflg INIT(FALSE);	/* game being saved?  */
CLASS short dbgflg;		/* game in restart?   */


CLASS char lastglob;		/* to get space req.  */

/*  endglobal  */

/*  function prototypes 						    */

/*  advent.c  */
int main(int argc, char **argv);
void restore(void);

/*  database.c	*/
void db_init(void);
void gettrav(short loc);
short yes(short msg1, short msg2, short msg3);
void rspeak(short msg);
void pspeak(short item, short state);
void desclg(short loc);
void descsh(short loc);
short vocab(char *word, short val);
uint8_t dark(void);
uint8_t here(short item);
uint8_t toting(short item);
uint8_t forced(short atloc);
uint8_t pct(short x);
uint8_t at(short item);
void dstroy(short obj);
void move(short obj, short where);
void juggle(short loc);
void carry(short obj, short where);
void drop(short obj, short where);
short put(short obj, short where, short pval);
short dcheck(void);
short liq(void);
short liqloc(short loc);
short liq2(short pbottle);
void bug(short n);

/*  verb.c  */
void trverb(void);
void vtake(void);
void vdrop(void);
void vopen(void);
void vsay(void);
void von(void);
void voff(void);
void vwave(void);
void vkill(void);
void vpour(void);
void veat(void);
void vdrink(void);
void vthrow(void);
void vfind(void);
void vfill(void);
void vfeed(void);
void vread(void);
void vblast(void);
void vbreak(void);
void vwake(void);
void actspk(short verb);
void needobj(void);

/*  english.c  */
uint8_t english(void);
uint8_t analyze(char *word, short *type, short *value);
void getin(void);
void getword(char **buff, char *word);
void skipspc(char **buff);

/*  turn.c  */
void turn(void);
void describe(void);
void descitem(void);
void domove(void);
void goback(void);
void dotrav(void);
void badmove(void);
void spcmove(short rdest);
void dwarfend(void);
void normend(void);
void score(void);
void death(void);
void doobj(void);
void trobj(void);
char *probj(short object);
void dwarves(void);
void dopirate(void);
uint8_t stimer(void);

/*  itverb.c  */
void itverb(void);
void ivtake(void);
void ivopen(void);
void ivkill(void);
void iveat(void);
void ivdrink(void);
void ivquit(void);
void ivfill(void);
void ivfoo(void);
void ivread(void);
void inventory(void);
void addobj(short obj);

/*  saveadv.c  */
void saveadv(void);

/* lib.c */
void nl(void);
void writes(const char *p);
void getinp(char *p, int l);
void writei(uint16_t v);
