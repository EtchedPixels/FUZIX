/*
 * startrek.c
 *
 * Super Star Trek Classic (v1.1)
 * Retro Star Trek Game 
 * C Port Copyright (C) 1996  <Chris Nystrom>
 *
 * Reworked for Fuzix by Alan Cox (C) 2018
 *	- Removed all floating point
 *	- Fixed multiple bugs in the BASIC to C conversion
 *	- Fixed a couple of bugs in the BASIC that either got in during it's
 *	  conversion between BASICs or from the original trek
 *	- Put it on a diet to get it to run in 32K. No features were harmed
 *	  in the making of this code smaller.
 *
 * TODO:
 *	- Look hard at all the rounding cases
 *	- Review some of the funnies in the BASIC code that appear to be bugs
 *	  either in the conversion or between the original and 'super' trek
 *	  Notably need to fix the use of shield energy directly for warp
 *	- Find a crazy person to draw ascii art bits we can showfile for things
 *	  like messages from crew/docking/klingons etc
 *	- I think it would make a lot of sense to switch to real angles, but
 *	  trek game traditionalists might consider that heresy.
 *
 * 
 * This program is free software; you can redistribute it and/or modify
 * in any way that you wish. _Star Trek_ is a trademark of Paramount
 * I think.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * This is a C port of an old BASIC program: the classic Super Star Trek
 * game. It comes from the book _BASIC Computer Games_ edited by David Ahl
 * of Creative Computing fame. It was published in 1978 by Workman Publishing,
 * 1 West 39 Street, New York, New York, and the ISBN is: 0-89489-052-3.
 * 
 * See http://www.cactus.org/~nystrom/startrek.html for more info.
 *
 * Contact Author of C port at:
 *
 * Chris Nystrom
 * 1013 Prairie Dove Circle
 * Austin, Texas  78758
 *
 * E-Mail: cnystrom@gmail.com, nystrom@cactus.org
 *
 * BASIC -> Conversion Issues
 *
 *     - String Names changed from A$ to sA
 *     - Arrays changed from G(8,8) to g[9][9] so indexes can
 *       stay the same.
 *
 * Here is the original BASIC header:
 *
 * SUPER STARTREK - MAY 16, 1978 - REQUIRES 24K MEMORY
 *
 ***        **** STAR TREK ****        ****
 *** SIMULATION OF A MISSION OF THE STARSHIP ENTERPRISE,
 *** AS SEEN ON THE STAR TREK TV SHOW.
 *** ORIGINAL PROGRAM BY MIKE MAYFIELD, MODIFIED VERSION
 *** PUBLISHED IN DEC'S "101 BASIC GAMES", BY DAVE AHL.
 *** MODIFICATIONS TO THE LATTER (PLUS DEBUGGING) BY BOB
 *** LEEDOM - APRIL & DECEMBER 1974,
 *** WITH A LITTLE HELP FROM HIS FRIENDS . . .
 *** COMMENTS, EPITHETS, AND SUGGESTIONS SOLICITED --
 *** SEND TO:  R. C. LEEDOM
 ***           WESTINGHOUSE DEFENSE & ELECTRONICS SYSTEMS CNTR.
 ***           BOX 746, M.S. 338
 ***           BALTIMORE, MD  21203
 ***
 *** CONVERTED TO MICROSOFT 8 K BASIC 3/16/78 BY JOHN BORDERS
 *** LINE NUMBERS FROM VERSION STREK7 OF 1/12/75 PRESERVED AS
 *** MUCH AS POSSIBLE WHILE USING MULTIPLE STATMENTS PER LINE
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#ifndef FALSE
#define FALSE        0
#endif

#ifndef TRUE
#define TRUE         ! FALSE
#endif

#ifndef TREK_DIR
#define TREK_DIR	"/usr/lib/trek/"
#endif

/* Standard Line Length */

#define MAXLEN     255

/* Standard Terminal Sizes */

#define MAXROW      24
#define MAXCOL      80

/* Standard Page Size */

#define MAXLINES    66

/* Useful typedefs */

typedef int bool;
typedef char line[MAXCOL];
typedef char string[MAXLEN];

/* Function Declarations */

static void intro(void);
static void new_game(void);
static void initialize(void);
static void new_quadrant(void);
static void course_control(void);
static void complete_maneuver(void);
static void exceed_quadrant_limits(void);
static void maneuver_energy(void);
static void short_range_scan(void);
static void long_range_scan(void);
static void phaser_control(void);
static void photon_torpedoes(void);
static void torpedo_hit(void);
static void damage_control(void);
static void shield_control(void);
static void library_computer(void);
static void galactic_record(void);
static void status_report(void);
static void torpedo_data(void);
static void nav_data(void);
static void dirdist_calc(void);
static void galaxy_map(void);
static void end_of_time(void);
static void resign_commision(void);
static void won_game(void);
static void end_of_game(void);
static void klingons_move(void);
static void klingons_shoot(void);
static void repair_damage(void);
static void find_set_empty_place(uint8_t t);
static const char *get_device_name(int n);
static void quadrant_name(void);
static int distance_to(int i);
static int square00(int16_t t);
static int cint100(int16_t d);
static void compute_vector(void);
static void sub1(void);
static void sub2(void);
static void showfile(char *filename);

/* Global Variables */

static int8_t b3;			/* Starbases in Quadrant */
static int b4, b5;			/* Starbase Location in sector */
static int8_t b9;			/* Total Starbases */

			  /* @@@ int c[2][10] = *//* Used for location and movement */
static int8_t c[3][10] =		/* modified to match MS BASIC array indicies */
{
	{0},
	{0, 0, -1, -1, -1, 0, 1, 1, 1, 0},
	{1, 1, 1, 0, -1, -1, -1, 0, 1, 1}
};

static uint8_t d0;			/* Docked flag */
static int d1;				/* Damage Repair Flag */
static int e;				/* Current Energy */
static int e0 = 3000;			/* Starting Energy */
static unsigned int g[9][9];		/* Galaxy. BCD of k b s plus flag */
#define MAP_VISITED 0x1000		/* Set if this sector was mapped */
static int g5;				/* Quadrant name flag */
static int k[4][4];			/* Klingon Data */
static uint8_t k3;			/* Klingons in Quadrant */
static uint8_t k7;			/* Klingons at start */
static uint8_t k9;			/* Total Klingons left */
static int n;				/* Number of sectors to travel */
static uint8_t p;			/* Photon Torpedoes left */
static uint8_t p0 = 10;			/* Photon Torpedo capacity */
static int q1, q2;			/* Quadrant Position of Enterprise */
static int r1, r2;			/* Temporary Location Corrdinates */
static int s;				/* Current shield value */
static uint8_t s3;			/* Stars in quadrant */
static uint16_t t0;			/* Starting Stardate */
static uint16_t t9;			/* End of time */
static int z1, z2;			/* Temporary Sector Coordinates */
static int z4, z5;			/* Temporary quadrant coordinates */

static int16_t a, c1;			/* Used by Library Computer */
static int16_t d[9];			/* Damage Array */
static int16_t d4;			/* Used for computing damage repair time */
static int16_t s1, s2;			/* Current Sector Position of Enterprise, fixed point */
static uint16_t t;			/* Current Stardate */
static int16_t w1;			/* Warp Factor */
static int16_t x, y, x1, x2;		/* Navigational coordinates, fixed point */

static char *sC;			/* Condition */

static uint8_t quad[8][8];
#define Q_SPACE		0
#define Q_STAR		1
#define Q_BASE		2
#define Q_KLINGON	3
#define Q_SHIP		4

static char quadname[12];		/* Quadrant name */

/* We probably need double digit for co-ordinate maths, single for time */
#define TO_FIXED(x)	((x) * 10)
#define FROM_FIXED(x)	((x) / 10)

#define TO_FIXED00(x)	((x) * 100)
#define FROM_FIXED00(x)	((x) / 100)

/*
 *	Returns an integer from 1 to iSpread
 */
static int get_rand(int iSpread)
{
	return ((rand() % iSpread) + 1);
}

/*
 *	Get a random co-ordinate
 */
static uint8_t rand8(void)
{
	return (get_rand(8));
}

/* This is basically a fancier fgets that always eats the line even if it
   only copies part of it */
static void input(char *b, uint8_t l)
{
	int c;

	fflush(stdout);
	while((c = getchar()) != '\n') {
		if (c == EOF)
			exit(1);
		if (l > 1) {
			*b++ = c;
			l--;
		}
	}
	*b = 0;
}

static uint8_t yesno(void)
{
	char b[2];
	input(b,2);
	if (tolower(*b) == 'y')
		return 1;
	return 0;
}

/* Input a value between 0.00 and 9.99 */
static int16_t input_f00(void)
{
	int16_t v;
	char buf[8];
	char *x;
	input(buf, 8);
	x = buf;
	if (!isdigit(*x))
		return -1;
	v = 100 * (*x++ - '0');
	if (*x == 0)
		return v;
	if (*x++ != '.')
		return -1;
	if (!isdigit(*x))
		return -1;
	v += 10 * (*x++ - '0');
	if (!*x)
		return v;
	if (!isdigit(*x))
		return -1;
	v += *x++ - '0';
	return v;
}

/* Integer: unsigned, or returns -1 for blank/error */
static int input_int(void)
{
	char x[8];
	input(x, 8);
	if (!isdigit(*x))
		return -1;
	return atoi(x);
}

static const char *print100(int16_t v)
{
	static char buf[16];
	char *p = buf;
	if (v < 0) {
		v = -v;
		*p++ = '-';
	}
	p += sprintf(p, "%d.%02d", v / 100, v%100);
	return buf;
}

/* Main Program */

int main(int argc, char *argv[])
{
	chdir(TREK_DIR);
	intro();

	new_game();

	/* @@@ exit(0);  *//* causes a warning in C++ */
	return (0);
}

static uint8_t inoperable(uint8_t u)
{
	if (d[u] < 0) {
		printf("%s %s inoperable.\n",
			get_device_name(u),
			u == 5 ? "are":"is");
		return 1;
	}
	return 0;
}

static void intro(void)
{
	showfile("startrek.intro");

	if (yesno())
		showfile("startrek.doc");

	showfile("startrek.logo");

	/* Seed the randomizer with the timer */
	srand((unsigned) time(NULL));

	/* Max of 4000, which works nicely with our 0.1 fixed point giving
	   us a 16bit unsigned range of time */
	t = TO_FIXED((get_rand(20) + 20) * 100);
}

static void new_game(void)
{
	char cmd[4];

	initialize();

	new_quadrant();

	short_range_scan();

	while (1) {
		if (s + e <= 10 && (e < 10 || d[7] < 0)) {
			showfile("startrek.fatal");
			end_of_time();
		}

		fputs("Command? ", stdout);

		input(cmd, 4);
		putchar('\n');

		if (!strncmp(cmd, "nav", 3))
			course_control();
		else if (!strncmp(cmd, "srs", 3))
			short_range_scan();
		else if (!strncmp(cmd, "lrs", 3))
			long_range_scan();
		else if (!strncmp(cmd, "pha", 3))
			phaser_control();
		else if (!strncmp(cmd, "tor", 3))
			photon_torpedoes();
		else if (!strncmp(cmd, "shi", 3))
			shield_control();
		else if (!strncmp(cmd, "dam", 3))
			damage_control();
		else if (!strncmp(cmd, "com", 3))
			library_computer();
		else if (!strncmp(cmd, "xxx", 3))
			resign_commision();
		else {
			/* FIXME: showfile ?*/
			puts("Enter one of the following:\n");
			puts("  nav - To Set Course");
			puts("  srs - Short Range Sensors");
			puts("  lrs - Long Range Sensors");
			puts("  pha - Phasers");
			puts("  tor - Photon Torpedoes");
			puts("  shi - Shield Control");
			puts("  dam - Damage Control");
			puts("  com - Library Computer");
			puts("  xxx - Resign Command\n");
		}
	}
}

static void initialize(void)
{
	int i, j;
	char sX[2] = "";
	char sX0[4] = "is";

	/* InItialize time */

	/* @@@ t0 = t; */
	t0 = FROM_FIXED(t);
	t9 = 25 + get_rand(10);

	/* Initialize Enterprise */

	d0 = 0;
	e = e0;
	p = p0;
	s = 0;

	q1 = rand8();
	q2 = rand8();
	s1 = TO_FIXED00(rand8());
	s2 = TO_FIXED00(rand8());

	for (i = 1; i <= 8; i++)
		d[i] = 0;

	/* Setup What Exists in Galaxy */

	for (i = 1; i <= 8; i++) {
		for (j = 1; j <= 8; j++) {
			k3 = 0;
			r1 = get_rand(100);
			if (r1 > 98)
				k3 = 3;
			else if (r1 > 95)
				k3 = 2;
			else if (r1 > 80)
				k3 = 1;

			k9 = k9 + k3;
			b3 = 0;

			if (get_rand(100) > 96)
				b3 = 1;

			b9 = b9 + b3;

			g[i][j] = (k3 << 8) + (b3 << 4) + rand8();
		}
	}

	/* Give more time for more Klingons */
	if (k9 > t9)
		t9 = k9 + 1;

	/* Add a base if we don't have one */
	if (b9 == 0) {
		if (g[q1][q2] < 0x200) {
			g[q1][q2] += (1 << 8);
			k9++;
		}

		g[q1][q2] += (1 << 4);
		b9++;

		q1 = rand8();
		q2 = rand8();
	}

	k7 = k9;

	if (b9 != 1) {
		strcpy(sX, "s");
		strcpy(sX0, "are");
	}

	printf("Your orders are as follows:\n"
	       " Destroy the %d Klingon warships which have invaded\n"
	       " the galaxy before they can attack Federation Headquarters\n"
	       " on stardate %u. This gives you %d days. There %s\n"
	       " %d starbase%s in the galaxy for resupplying your ship.\n\n"
	       "Hit any key to accept command. ",
	       k9, t0 + t9, t9, sX0, b9, sX);
	getchar();
}

static void new_quadrant(void)
{
	int i;
	uint16_t tmp;

	z4 = q1;
	z5 = q2;
	k3 = 0;
	b3 = 0;
	s3 = 0;
	g5 = 0;

	/* Random factor for damage repair. We compute it on each new
	   quadrant to stop the user just retrying until they get a number
	   they like. The conversion here was wrong and now copies the BASIC
	   code generate 0.00 to 0.49 */
	d4 = get_rand(50) - 1;

	/* Copy to computer */
	g[q1][q2] |= MAP_VISITED;

	if (q1 >= 1 && q1 <= 8 && q2 >= 1 && q2 <= 8) {
		quadrant_name();

		if (TO_FIXED(t0) != t)
			printf("Now entering %s quadrant...\n\n", quadname);
		else {
			puts("\nYour mission begins with your starship located");
			printf("in the galactic quadrant %s.\n\n", quadname);
		}
	}

	/* @@@ k3 = g[q1][q2] * .01; */
	tmp = g[q1][q2];
	k3 = (tmp >> 8) & 0x0F;
	b3 = (tmp >> 4) & 0x0F;
	s3 = tmp & 0x0F;

	if (k3 > 0) {
		printf("Combat Area  Condition Red\n");

		if (s < 200)
			printf("Shields Dangerously Low\n");
	}

	for (i = 1; i <= 3; i++) {
		k[i][1] = 0;
		k[i][2] = 0;
		k[i][3] = 0;
	}

	memset(quad, Q_SPACE, 64);

	/* FIXME: do we need 0.5 shifts ? */
	quad[FROM_FIXED00(s1) - 1][FROM_FIXED00(s2) - 1] = Q_SHIP;
	
	if (k3 > 0) {
		for (i = 1; i <= k3; i++) {
			find_set_empty_place(Q_KLINGON);
			k[i][1] = r1;
			k[i][2] = r2;
			k[i][3] = 100 + get_rand(200);
		}
	}

	if (b3 > 0) {
		find_set_empty_place(Q_BASE);

		b4 = r1;
		b5 = r2;
	}

	for (i = 1; i <= s3; i++)
		find_set_empty_place(Q_STAR);
}

static const char *inc_1 = "reports:\n  Incorrect course data, sir!\n";

static void course_control(void)
{
	register int i;
	/* @@@ int c2, c3, q4, q5; */
	int16_t c1;
	int c2, c3, c4;
	char sX[4] = "8";

	fputs("Course (0-9): ", stdout);

	c1 = input_f00();

	if (c1 == 900)
		c1 = 100;

	if (c1 < 0 || c1 > 900) {
		printf("Lt. Sulu%s", inc_1);
		return;
	}

	if (d[1] < 0)
		strcpy(sX, "0.2");

	printf("Warp Factor (0-%s): ", sX);

	w1 = input_f00();

	if (d[1] < 0 && w1 > 20) {
		printf("Warp Engines are damaged. "
		       "Maximum speed = Warp 0.2.\n\n");
		return;
	}

	if (w1 <= 0)
		return;

	if (w1 > 800) {
		printf("Chief Engineer Scott reports:\n"
		       "  The engines won't take warp %s!\n\n", print100(w1));
		return;
	}

	n = w1 * 8;

	n = cint100(n);	/* @@@ note: this is a real round in the original basic */

	/* FIXME: should be  s + e - n > 0 iff shield control undamaged */
	if (e - n < 0) {
		printf("Engineering reports:\n"
		       "  Insufficient energy available for maneuvering"
		       " at warp %s!\n\n", print100(w1));

		if (s >= n && d[7] >= 0) {
			printf("Deflector Control Room acknowledges:\n"
			       "  %d units of energy presently deployed to shields.\n", s);
		}

		return;
	}

	klingons_move();

	repair_damage();

	/* @@@ z1 = cint(s1); */
	z1 = FROM_FIXED00(s1);
	/* @@@ z2 = cint(s2); */
	z2 = FROM_FIXED00(s2);
	quad[z1-1][z2-1] = Q_SPACE;

#if 0
	/* @@@ c2 = cint(c1); */
	/* @@@ c3 = c2 + 1; */

	/* @@@ x1 = c[0][c2] + (c[0][c3] - c[0][c2]) * (c1 - c2); */
	/* @@@ x2 = c[1][c2] + (c[1][c3] - c[1][c2]) * (c1 - c2); */

	x1 = c[1][(int) c1] + (c[1][(int) c1 + 1] - c[1][(int) c1]) * (c1 - (int) c1);
	x2 = c[2][(int) c1] + (c[2][(int) c1 + 1] - c[2][(int) c1]) * (c1 - (int) c1);
#else

	c2 = FROM_FIXED00(c1);	/* Integer part */
//	printf("C2 = %d\n", c2);
	c3 = c2 + 1;		/* Next integer part */
	c4 = (c1 - TO_FIXED00(c2));	/* Fractional element in fixed point */

	x1 = 100 * c[1][c2] + (c[1][c3] - c[1][c2]) * c4;
	x2 = 100 * c[2][c2] + (c[2][c3] - c[2][c2]) * c4;

//	printf("x1 = %d x2 = %d\n", x1, x2);

#endif

	x = s1;
	y = s2;

	for (i = 1; i <= n; i++) {
//		printf(">%d %4.1f %4.1f %4.1f %4.1f\n",
//			i, s1, s2, x1, x2);
		s1 = s1 + x1;
		s2 = s2 + x2;

//		printf("=%d %4.1f %4.1f %4.1f %4.1f\n",
//			i, s1, s2, x1, x2);

		/* @@@ z1 = cint(s1); */
		z1 = FROM_FIXED00(s1);
		/* @@@ z2 = cint(s2); */
		z2 = FROM_FIXED00(s2);	/* ?? cint100 ?? */

		if (z1 < 1 || z1 >= 9 || z2 < 1 || z2 >= 9) {
			exceed_quadrant_limits();
			complete_maneuver();
			return;
		}

		if (quad[z1-1][z2-1] != Q_SPACE) {	/* Sector not empty */
			s1 = s1 - x1;
			s2 = s2 - x2;
			printf("Warp Engines shut down at sector "
			       "%d, %d due to bad navigation.\n\n", z1, z2);
			i = n + 1;
		}
	}

	complete_maneuver();
}

static void complete_maneuver(void)
{
	int t8;

	/* @@@ z1 = cint(s1); */
	z1 = FROM_FIXED00(s1);
	/* @@@ z2 = cint(s2); */
	z2 = FROM_FIXED00(s2);
	quad[z1-1][z2-1] = Q_SHIP;

	maneuver_energy();

	t8 = TO_FIXED(1);

	/* Ick FIXME - re really want to tidy up time to FIXED00 */
	if (w1 < 100)
		t8 = TO_FIXED(FROM_FIXED00(w1));

	t = t + t8;

	if (FROM_FIXED(t) > t0 + t9)
		end_of_time();

	short_range_scan();
}

static void exceed_quadrant_limits(void)
{
	int x5 = 0;		/* Outside galaxy flag */

	/* @@@ x = (8 * (q1 - 1)) + x + (n * x1); */
	x = (800 * q1) + x + (n * x1);
	/* @@@ y = (8 * (q2 - 1)) + y + (n * x2); */
	y = (800 * q2) + y + (n * x2);

//	printf("X %d Y %d\n", x, y);

	/* @@@ q1 = cint(x / 8.0); */
	q1 = x / 800;	/* Fixed point to int and divide by 8 */
	/* @@@ q2 = cint(y / 8.0); */
	q2 = y / 800;	/* Ditto */

//	printf("Q %d %d\n", q1, q2);

	/* @@@ s1 = x - ((q1 - 1) * 8); */
	s1 = x - (q1 * 800);
	/* @@@ s2 = y - ((q2 - 1) * 8); */
	s2 = y - (q2 * 800);

//	printf("S %d %d\n", s1, s2);

	/* @@@ if (cint(s1) == 0) */
	if (s1 < 100) {
		q1 = q1 - 1;
		s1 = s1 + 800;
	}

	/* @@@ if (cint(s2) == 0) */
	if (s2 < 100) {
		q2 = q2 - 1;
		s2 = s2 + 800;
	}

	/* check if outside galaxy */

	if (q1 < 1) {
		x5 = 1;
		q1 = 1;
		s1 = 100;
	}

	if (q1 > 8) {
		x5 = 1;
		q1 = 8;
		s1 = 800;
	}

	if (q2 < 1) {
		x5 = 1;
		q2 = 1;
		s2 = 100;
	}

	if (q2 > 8) {
		x5 = 1;
		q2 = 8;
		s2 = 800;
	}

	if (x5 == 1) {
		/* Mostly showfile ? FIXME */
		printf("LT. Uhura reports:\n"
		       "  Message from Starfleet Command:\n\n"
		       "  Permission to attempt crossing of galactic perimeter\n"
		       "  is hereby *denied*. Shut down your engines.\n\n"
		       "Chief Engineer Scott reports:\n"
		       "  Warp Engines shut down at sector %d, "
		       "%d of quadrant %d, %d.\n\n",
		       FROM_FIXED00(s1), FROM_FIXED00(s2), q1, q2);
	}
	/* else 
	   new_quadrant(); @@@ this causes bugs when bouncing off galaxy walls.
	   basically, if you bounce very far, your quadrant contents
	   won't match your LRS.  Cool huh? */


	maneuver_energy();

	/* this section has a different order in the original.
	   t = t + 1;

	   if (t > t0 + t9)
	   end_of_time();
	 */

	if (FROM_FIXED(t) > t0 + t9)
		end_of_time();

	/* @@@ what does this do?? It's in the original.
	   if (8 * q1 + q2 = 8 * q4 + q5) 
	   { 
	   complete_maneuver();
	   }
	 */

	t = t + 1;

	new_quadrant();
}

static void maneuver_energy(void)
{
	e = e - n - 10;

	if (e >= 0)
		return;

	puts("Shield Control supplies energy to complete maneuver.\n");

	s = s + e;
	e = 0;

	if (s <= 0)
		s = 0;
}

static const char *srs_1 = "------------------------";

static const char *tilestr[] = {
	"   ",
	" * ",
	">!<",
	"+K+",
	"<*>"
};

static void short_range_scan(void)
{
	register int i, j;

	sC = "GREEN";

	if (e < e0 / 10)
		sC = "YELLOW";

	if (k3 > 0)
		sC = "*RED*";

	/* @@@ need to clear the docked flag here */
	d0 = 0;

	/* @@@ for (i = s1 - 1; i <= s1 + 1; i++) */
	for (i = (int) (FROM_FIXED00(s1) - 1); i <= (int) (FROM_FIXED00(s1) + 1); i++)
		/* @@@ for (j = s2 - 1; j <= s2 + 1; j++) */
		for (j = (int) (FROM_FIXED00(s2) - 1); j <= (int) (FROM_FIXED00(s2) + 1); j++)
			if (i >= 1 && i <= 8 && j >= 1 && j <= 8) {
				/* This is dumb - we store the base co-ords! */
				if (quad[i-1][j-1] == Q_BASE) {
					d0 = 1;
					sC = "DOCKED";
					e = e0;
					p = p0;
					puts("Shields dropped for docking purposes.");
					s = 0;
				}
			}

	if (d[2] < 0) {
		puts("\n*** Short Range Sensors are out ***");
		return;
	}

	puts(srs_1);
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++)
			fputs(tilestr[quad[i][j]], stdout);

		if (i == 0)
			printf("    Stardate            %d\n", FROM_FIXED(t));
		if (i == 1)
			printf("    Condition           %s\n", sC);
		if (i == 2)
			printf("    Quadrant            %d, %d\n", q1, q2);
		if (i == 3)
			/* @@@ printf("    Sector              %d, %d\n", cint(s1), cint(s2)); */
			printf("    Sector              %d, %d\n", FROM_FIXED00(s1), FROM_FIXED00(s2));
		if (i == 4)
			printf("    Photon Torpedoes    %d\n", p);
		if (i == 5)
			printf("    Total Energy        %d\n", e + s);
		if (i == 6)
			printf("    Shields             %d\n", s);
		if (i == 7)
			printf("    Klingons Remaining  %d\n", k9);
	}
	puts(srs_1);
	putchar('\n');

	return;
}

static const char *lrs_1 = "-------------------\n";

static void put1bcd(uint8_t v)
{
	v &= 0x0F;
	putchar('0' + v);
}

static void putbcd(uint16_t x)
{
	put1bcd(x >> 8);
	put1bcd(x >> 4);
	put1bcd(x);
}

static void long_range_scan(void)
{
	register int i, j;

	if (inoperable(3))
		return;

	printf("Long Range Scan for Quadrant %d, %d\n\n", q1, q2);

	for (i = q1 - 1; i <= q1 + 1; i++) {
		printf("%s:", lrs_1);
		for (j = q2 - 1; j <= q2 + 1; j++) {
			putchar(' ');
			if (i > 0 && i <= 8 && j > 0 && j <= 8) {
				g[i][j] |= MAP_VISITED;
				putbcd(g[i][j]);
			} else
				fputs("***", stdout);
			fputs(" :", stdout);
		}
		putchar('\n');
	}

	printf("%s\n", lrs_1);
}

static uint8_t no_klingon(void)
{
	if (k3 <= 0) {
		puts("Science Officer Spock reports:\n"
		     "  'Sensors show no enemy ships in this quadrant'\n");
		return 1;
	}
	return 0;
}


static void phaser_control(void)
{
	register int i;
	int32_t iEnergy;
	uint32_t h1;
	int h;

	if (inoperable(4))
		return;

	if (no_klingon())
		return;

	/* There's Klingon's on the starboard bow... */
	if (d[8] < 0)
		/* @@@ printf("Computer failure happers accuracy.\n"); */
		puts("Computer failure hampers accuracy.");

	printf("Phasers locked on target;\n"
	       "Energy available = %d units\n\n"
	       "Number of units to fire: ", e);

	iEnergy = input_int();

	if (iEnergy <= 0)
		return;

	if (e - iEnergy < 0) {
		puts("Not enough energy available.\n");
		return;
	}

	e = e - iEnergy;

	/* We can fire up to nearly 3000 points of energy so we do this
	   bit in 32bit math */

	if (d[8] < 0)
		/* @@@ iEnergy = iEnergy * rnd(); */
		iEnergy = iEnergy * get_rand(100);
	else
		iEnergy *= 100;

	h1 = iEnergy / k3;

//	printf("h1 = %d\n", h1);

	for (i = 1; i <= 3; i++) {
		if (k[i][3] > 0) {
			/* @@@ h = (h1 / distance_to(0) * (rnd() + 2)); */
			/* We are now 32bit with four digits accuracy */
			h = h1 * (get_rand(100) + 200);
			/* Takes us down to 2 digit accuracy */

//			printf("distance_to = %d\n", distance_to(i));

			h /= distance_to(i);

//			printf("damage base %d\n", h);

			if (h <= 15 * k[i][3]) {	/* was 0.15 */
				printf("Sensors show no damage to enemy at "
				       "%d, %d\n\n", k[i][1], k[i][2]);
			} else {
				h = FROM_FIXED00(h);
				k[i][3] = k[i][3] - h;
				printf("%d unit hit on Klingon at sector "
				       "%d, %d\n",
					h, k[i][1], k[i][2]);
				if (k[i][3] <= 0) {
					puts("*** Klingon Destroyed ***\n");
					k3--;
					k9--;
					z1 = k[i][1];
					z2 = k[i][2];
					quad[z1-1][z2-1] = Q_SPACE;
					k[i][3] = 0;
					/* Minus a Klingon.. */
					g[q1][q2] -= 0x100;
					if (k9 <= 0)
						won_game();
				} else
					/* @@@ printf("\n"); */
					printf("   (Sensors show %d units remaining.)\n\n", k[i][3]);
			}
		}
	}

	klingons_shoot();
}

static void photon_torpedoes(void)
{
	/* @@@ int c2, c3, x3, y3, x5; */
	int x3, y3;
	int16_t c1;
	int c2, c3, c4;

	if (p <= 0) {
		puts("All photon torpedoes expended");
		return;
	}

	if (inoperable(5))
		return;

	fputs("Course (0-9): ", stdout);

	c1 = input_f00();

	if (c1 == 900)
		c1 = 100;

	/* @@@ if (c1 < 0 || c1 > 9.0) */
	if (c1 < 100 || c1 > 900) {
		printf("Ensign Chekov%s", inc_1);
		return;
	}

	e = e - 2;
	p--;

#if 0
	/* @@@ c2 = cint(c1); */
	/* @@@ c3 = c2 + 1; */

	/* @@@ x1 = c[0][c2] + (c[0][c3] - c[0][c2]) * (c1 - c2); */
	/* @@@ x2 = c[1][c2] + (c[1][c3] - c[1][c2]) * (c1 - c2); */

	x1 = c[1][(int) c1] + (c[1][(int) c1 + 1] - c[1][(int) c1]) * (c1 - (int) c1);
	x2 = c[2][(int) c1] + (c[2][(int) c1 + 1] - c[2][(int) c1]) * (c1 - (int) c1);
#else

	c2 = FROM_FIXED00(c1);	/* Integer part */
	c3 = c2 + 1;		/* Next integer part */
	c4 = (c1 - TO_FIXED00(c2));	/* Fractional element in fixed point */

	x1 = 100 * c[1][c2] + (c[1][c3] - c[1][c2]) * c4;
	x2 = 100 * c[2][c2] + (c[2][c3] - c[2][c2]) * c4;

#endif

	x = s1 + x1;
	y = s2 + x2;

	/* If the ship is not rounded this is wrong FIXME */

	x3 = cint100(x);	/* @@@ note: this is a true integer round in the MS BASIC version */
	y3 = cint100(y);	/* @@@ note: this is a true integer round in the MS BASIC version */

	puts("Torpedo Track:");

	while (x3 >= 1 && x3 <= 8 && y3 >= 1 && y3 <= 8) {
		uint8_t p;

		printf("    %d, %d\n", x3, y3);

		p = quad[x3-1][y3-1];
		/* In certain corner cases the first trace we'll step is
		   ourself. If so treat it as space */
		if (p != Q_SPACE && p != Q_SHIP) {
			torpedo_hit();
			klingons_shoot();
			return;
		}

		x = x + x1;
		y = y + x2;

		x3 = cint100(x);    /* @@@ note: this is a true integer round in the MS BASIC version */
		y3 = cint100(y);    /* @@@ note: this is a true integer round in the MS BASIC version */
	}

	puts("Torpedo Missed\n");

	klingons_shoot();
}

static void torpedo_hit(void)
{
	int i, x3, y3;

	x3 = cint100(x);	/* @@@ note: this is a true integer round in the MS BASIC version */
	y3 = cint100(y);	/* @@@ note: this is a true integer round in the MS BASIC version */

	switch(quad[x3-1][y3-1]) {
	case Q_STAR:
		printf("Star at %d, %d absorbed torpedo energy.\n\n", x3, y3);
		return;
	case Q_KLINGON:
		puts("*** Klingon Destroyed ***\n");
		k3--;
		k9--;

		if (k9 <= 0)
			won_game();

		for (i = 0; i <= 3; i++)
			if (x3 == k[i][1] && y3 == k[i][2])
				k[i][3] = 0;
		g[q1][q2] -= 0x100;
		break;
	case Q_BASE:					
		puts("*** Starbase Destroyed ***");
		b3--;
		b9--;

		if (b9 <= 0 && k9 <= FROM_FIXED(t) - t0 - t9) {
			/* showfile ? FIXME */
			puts("That does it, Captain!!"
			     "You are hereby relieved of command\n"
			     "and sentenced to 99 stardates of hard"
			     "labor on Cygnus 12!!\n");
			resign_commision();
		}

		puts("Starfleet Command reviewing your record to consider\n"
		     "court martial!\n");

		d0 = 0;		/* Undock */
		g[q1][q2] -= 0x10;
		break;
	}
	z1 = x3;
	z2 = y3;
	quad[z1-1][z2-1] = Q_SPACE;
}

static void damage_control(void)
{
	int16_t d3 = 0;
	register int i;

	if (d[6] < 0)
		puts("Damage Control report not available.");

	/* Offer repair if docked */
	if (d0) {
		/* d3 is x.xx fixed point */
		d3 = 0;
		for (i = 1; i <= 8; i++)
			if (d[i] < 0)
				d3 = d3 + 10;

		if (d3) {
			d3 = d3 + d4;
			if (d3 >= 100)
				d3 = 90;	/* 0.9 */

			printf("\nTechnicians standing by to effect repairs to your"
			       "ship;\nEstimated time to repair: %s stardates.\n"
			       "Will you authorize the repair order (y/N)? ", print100(d3));

			if (yesno()) {
				for (i = 1; i <= 8; i++)
					if (d[i] < 0)
						d[i] = 0;

				/* Work from two digit to one digit. We might actually
				   have to give in and make t a two digt offset from
				   a saved constant base only used in printing to
				   avoid that round below FIXME */
				t = t + (d3 + 5)/10 + 1;
			}
			return;
		}
	}

	if (d[6] < 0)
		return;

	puts("Device            State of Repair");

	for (r1 = 1; r1 <= 8; r1++)
		printf("%-25s%6s\n", get_device_name(r1), print100(d[r1]));

	printf("\n");
}

static void shield_control(void)
{
	int i;

	if (inoperable(7))
		return;

	printf("Energy available = %d\n\n"
	       "Input number of units to shields: ", e + s);

	i = input_int();

	if (i < 0 || s == i) {
unchanged:
		puts("<Shields Unchanged>\n");
		return;
	}

	if (i >= e + s) {
		puts("Shield Control Reports:\n"
		     "  'This is not the Federation Treasury.'");
		goto unchanged;
	}

	e = e + s - i;
	s = i;

	printf("Deflector Control Room report:\n"
	       "  'Shields now at %d units per your command.'\n\n", s);
}

static void library_computer(void)
{

	if (inoperable(8))
		return;

	fputs("Computer active and awating command: ", stdout);

	switch(input_int()) {
		/* -1 means 'typed nothing or junk */
		case -1:
			break;
		case 0:
			galactic_record();
			break;
		case 1:
			status_report();
			break;
		case 2:
			torpedo_data();
			break;
		case 3:
			nav_data();
			break;
		case 4:
			dirdist_calc();
			break;
		case 5:
			galaxy_map();
			break;
		default:
			/* FIXME: showfile */
			puts("Functions available from Library-Computer:\n\n"
			     "   0 = Cumulative Galactic Record\n"
			     "   1 = Status Report\n"
			     "   2 = Photon Torpedo Data\n"
			     "   3 = Starbase Nav Data\n"
			     "   4 = Direction/Distance Calculator\n"
			     "   5 = Galaxy 'Region Name' Map\n");
	}
}

static const char *gr_1 = "   ----- ----- ----- ----- ----- ----- ----- -----\n";

static void galactic_record(void)
{
	int i, j;

	printf("\n     Computer Record of Galaxy for Quadrant %d,%d\n\n", q1, q2);
	puts("     1     2     3     4     5     6     7     8");

	for (i = 1; i <= 8; i++) {
		printf("%s%d", gr_1, i);

		for (j = 1; j <= 8; j++) {
			printf("   ");

			if (g[i][j] & MAP_VISITED)
				putbcd(g[i][j]);
			else
				printf("***");
		}
		putchar('\n');
	}

	printf("%s\n", gr_1);
}

static const char *str_s = "s";

static void status_report(void)
{
	const char *plural = str_s + 1;
	uint16_t left = TO_FIXED(t0 + t9) - t;

	puts("   Status Report:\n");

	if (k9 > 1)
		plural = str_s;

	/* Assumes fixed point is single digit fixed */
	printf("Klingon%s Left: %d\n"
	       "Mission must be completed in %d.%d stardates\n",
		plural, k9,
		FROM_FIXED(left), left%10);

	if (b9 < 1) {
		puts("Your stupidity has left you on your own in the galaxy\n"
		     " -- you have no starbases left!\n");
	} else {
		plural = str_s;
		if (b9 < 2)
			plural++;

		printf("The Federation is maintaining %d starbase%s in the galaxy\n\n", b9, plural);
	}
}

static void torpedo_data(void)
{
	int i;
	const char *plural = str_s + 1;

	/* Need to also check sensors here ?? */
	if (no_klingon())
		return;

	if (k3 > 1)
		plural--;

	printf("From Enterprise to Klingon battlecriuser%s:\n\n", plural);

	for (i = 1; i <= 3; i++) {
		if (k[i][3] > 0) {
			w1 = TO_FIXED00(k[i][1]);
			x = TO_FIXED00(k[i][2]);
			c1 = s1;
			a = s2;

			compute_vector();
		}
	}
}

static void nav_data(void)
{
	if (b3 <= 0) {
		puts("Mr. Spock reports,\n"
		     "  'Sensors show no starbases in this quadrant.'\n");
		return;
	}

	w1 = TO_FIXED00(b4);
	x = TO_FIXED00(b5);
	c1 = s1;
	a = s2;

	compute_vector();
}

/* Q: do we want to support fractional co-ords ? */
static void dirdist_calc(void)
{
	printf("Direction/Distance Calculator\n"
	       "You are at quadrant %d,%d sector %d,%d\n\n"
	       "Please enter initial X coordinate: ",
	       q1, q2,
	       /* @@@ cint(s1), cint(s2)); */
	       FROM_FIXED00(s1), FROM_FIXED00(s2));

	c1 = TO_FIXED00(input_int());
	if (c1 < 0 || c1 > 900 )
		return;

	fputs("Please enter initial Y coordinate: ", stdout);
	a = TO_FIXED00(input_int());
	if (a < 0 || a > 900)
		return;

	fputs("Please enter final X coordinate: ", stdout);
	w1 = TO_FIXED00(input_int());
	if (w1 < 0 || w1 > 900)
		return;

	fputs("Please enter final Y coordinate: ", stdout);
	x = TO_FIXED00(input_int());
	if (x < 0 || x > 900)
		return;
	compute_vector();
}

static const char *gm_1 = "  ----- ----- ----- ----- ----- ----- ----- -----\n";

static void galaxy_map(void)
{
	int i, j, j0;

	g5 = 1;

	printf("\n                   The Galaxy\n\n");
	printf("    1     2     3     4     5     6     7     8\n");

	for (i = 1; i <= 8; i++) {
		printf("%s%d ", gm_1, i);

		z4 = i;
		z5 = 1;
		quadrant_name();

		j0 = (int) (11 - (strlen(quadname) / 2));

		for (j = 0; j < j0; j++)
			putchar(' ');

		fputs(quadname, stdout);

		for (j = 0; j < j0; j++)
			putchar(' ');

		if (!(strlen(quadname) % 2))
			putchar(' ');

		z5 = 5;
		quadrant_name();

		j0 = (int) (12 - (strlen(quadname) / 2));

		for (j = 0; j < j0; j++)
			putchar(' ');

		puts(quadname);
	}

	puts(gm_1);

}

static void compute_vector(void)
{
	/* All this is happening in fixed point */
	x = x - a;
	a = c1 - w1;

	if (x < 0) {
/*8350*/	if (a > 0) {
			c1 = 300;
/* 8420 */		sub2();
			return;
		} else if (x != 0){
/* 8360 */		c1 = 500;
/* 8290 */		sub1();
			return;
/* 8410 */	} else {
			c1 = 700;
/* 8420 */		sub2();
			return;
		}
/*8250*/} else if (a < 0) {
/*8410*/	c1 = 700;
/*8420*/	sub2();
		return;
/* 8260 */} else if (x > 0) {
/* 8280 */	c1 = 100;
/* 8290 */		sub1();
		return;
	} else if (a == 0) {
		c1 = 500;
		sub1();
	} else {
		c1 = 100;
		sub1();
	}
}

static const char *dir_1 = "  DIRECTION = ";
static const char *dist_1 = "  DISTANCE = %s\n\n";

static void sub1(void)
{
/* 8290 */uint32_t xl = abs(x);
	uint32_t al = abs(a);

	fputs(dir_1, stdout);
	/* Multiply the top half by 100 as well so that we keep it in fixed00
	   format. Our larget value is int 9 (900) so we must do this 32bit */
/* 8290 */if (al <= xl)
/*8330 */	printf("%s", print100(c1 + ((al * 100) / xl)));
	else
/*8310 */	printf("%s", print100(c1 + ((((al * 2) - xl) * 100) / al)));

	printf(dist_1, print100((xl > al) ? xl : al));
}

static void sub2(void)
{
/* 8420 */	uint32_t xl = abs(x);
	uint32_t al = abs(a);

	fputs(dir_1, stdout);
	/* Multiply the top half by 100 to keep in fixed0 */
	if (al >= xl)
		printf("%s", print100(c1 + ((xl * 100) / al)));
	else
		/* @@@ printf("  DIRECTION = %4.2f\n\n", c1 + (((x * 2) - a) / x)); */
		printf("%s", print100(c1 + ((((xl * 2) - al) * 100)  / xl)));

	/* @@@ printf("  DISTANCE = %4.2f\n", (x > a) ? x : a); */
	printf(dist_1, print100((x > a) ? x : a));
}

static void ship_destroyed(void)
{
	puts("The Enterprise has been destroyed. "
	     "The Federation will be conquered.\n");

	end_of_time();
}

static void end_of_time(void)
{
	printf("It is stardate %d.\n\n",  FROM_FIXED(t));

	resign_commision();
}

static void resign_commision(void)
{
	printf("There were %d Klingon Battlecruisers left at the"
	       " end of your mission.\n\n", k9);

	end_of_game();
}

static void won_game(void)
{
	puts("Congratulations, Captain!  The last Klingon Battle Cruiser\n"
	     "menacing the Federation has been destoyed.\n");

	/* FIXME: finish integerising this */
	if (FROM_FIXED(t) - t0 > 0)
		printf("Your efficiency rating is %s\n",
			print100(square00(TO_FIXED00(k7)/(FROM_FIXED(t) - t0))));
		// 1000 * pow(k7 / (float)(FROM_FIXED(t) - t0), 2));
	end_of_game();
}

static void end_of_game(void)
{
	char x[4];
	if (b9 > 0) {
		/* FIXME: showfile ? */
		fputs("The Federation is in need of a new starship commander"
		     " for a similar mission.\n"
		     "If there is a volunteer, let him step forward and"
		     " enter 'aye': ", stdout);

		input(x,4);
		if (!strncmp(x, "aye", 3))
			new_game();
	}
	exit(0);
}

static void klingons_move(void)
{
	int i;

	for (i = 1; i <= 3; i++) {
		if (k[i][3] > 0) {
			z1 = k[i][1];
			z2 = k[i][2];
			quad[z1-1][z2-1] = Q_SPACE;

			find_set_empty_place(Q_KLINGON);

			k[i][1] = z1;
			k[i][2] = z2;
		}
	}

	klingons_shoot();
}

static const char *dcr_1 = "Damage Control report:";

static void klingons_shoot(void)
{
	uint32_t h;
	uint8_t i;

	if (k3 <= 0)
		return;

	if (d0 != 0) {
		puts("Starbase shields protect the Enterprise\n");
		return;
	}

	for (i = 1; i <= 3; i++) {
		if (k[i][3] > 0) {
			h = k[i][3] * (200 + get_rand(100));
			h *= 100;	/* Ready for division in fixed */
			h /= distance_to(i);
			/* Takes us back into FIXED00 */
			s = s - FROM_FIXED00(h);
			/* @@@ k[i][3] = k[i][3] / (3 + rnd()); */

			k[i][3] = (k[i][3] * 100) / (300 + get_rand(100));

			printf("%d unit hit on Enterprise from sector "
			       "%d, %d\n", FROM_FIXED00(h), k[i][1], k[i][2]);

			if (s <= 0) {
				putchar('\n');
				ship_destroyed();
			}

			printf("    <Shields down to %d units>\n\n", s);

			if (h >= 20) {
				/* The check in basic is float and is h/s >.02. We
				   have to use 32bit values here to avoid an overflow
				   FIXME: use a better algorithm perhaps ? */
				uint32_t ratio = ((uint32_t)h)/s;
				if (get_rand(10) <= 6 || ratio > 2) {
					r1 = rand8();
					/* The original basic code computed h/s in
					   float form the C conversion broke this. We correct it in the fixed
					   point change */
					d[r1] = d[r1] - ratio - get_rand(50);

					/* FIXME: can we use dcr_1 here ?? */
					printf("Damage Control reports\n"
					       "   '%s' damaged by hit\n\n", get_device_name(r1));
				}
			}
		}
	}
}

static void repair_damage(void)
{
	int i;
	int16_t d6;		/* Repair Factor */

	d6 = w1;
	if (w1 >= 100)
		d6 = TO_FIXED00(1);

	for (i = 1; i <= 8; i++) {
		if (d[i] < 0) {
			d[i] = d[i] + d6;
			if (d[i] > -10 && d[i] < 0)	/* -0.1 */
				d[i] = -10;
			else if (d[i] >= 0) {
				if (d1 != 1) {
					d1 = 1;
					puts(dcr_1);
				}
				printf("    %s repair completed\n\n",
					get_device_name(i));
				d[i] = 0;
			}
		}
	}

	if (get_rand(10) <= 2) {
		r1 = rand8();

		if (get_rand(10) < 6) {
			/* Working in 1/100ths */
			d[r1] = d[r1] - (get_rand(500) + 100);
			//TO_FIXED00((rnd() * 5.0 + 1.0));
			puts(dcr_1);
			printf("    %s damaged\n\n", get_device_name(r1));
		} else {
			/* Working in 1/100ths */
			d[r1] = d[r1] + get_rand(300) + 100;
			// TO_FIXED00(rnd() * 3.0 + 1.0);
			puts(dcr_1);
			printf("    %s state of repair improved\n\n",
					get_device_name(r1));
		}
	}
}

/* Misc Functions and Subroutines */

static void find_set_empty_place(uint8_t t)
{
	do {
		r1 = rand8();
		r2 = rand8();
	} while (quad[r1-1][r2-1] != Q_SPACE );
	quad[r1-1][r2-1] = t;
	z1 = r1;
	z2 = r2;
}

static const char *device_name[] = {
	"", "Warp engines", "Short range sensors", "Long range sensors",
	"Phaser control", "Photon tubes", "Damage control", "Shield control",
	"Library computer"
};

static const char *get_device_name(int n)
{
	if (n < 0 || n > 8)
		n = 0;
	return device_name[n];
}

static const char *quad_name[] = { "",
	"Antares", "Rigel", "Procyon", "Vega", "Canopus", "Altair",
	"Sagittarius", "Pollux", "Sirius", "Deneb", "Capella",
	"Betelgeuse", "Aldebaran", "Regulus", "Arcturus", "Spica"
};

static void quadrant_name(void)
{

	static char *sect_name[] = { "", " I", " II", " III", " IV" };

	if (z4 < 1 || z4 > 8 || z5 < 1 || z5 > 8)
		strcpy(quadname, "Unknown");

	if (z5 <= 4)
		strcpy(quadname, quad_name[z4]);
	else
		strcpy(quadname, quad_name[z4 + 8]);

	if (g5 != 1) {
		if (z5 > 4)
			z5 = z5 - 4;
		strcat(quadname, sect_name[z5]);
	}

	return;
}

/* An unsigned sqrt is all we need.

   What we are actually doing here is a smart version of calculating n^2
   repeatedly until we find the right one */
static int16_t isqrt(int16_t i)
{
	uint16_t b = 0x4000, q = 0, r = i, t;
//	printf("isqrt %d=", i);
	while (b) {
		t = q + b;
		q >>= 1;
		if (r >= t) {
			r -= t;
			q += b;
		}
		b >>= 2;
	}
//	printf("%d\n", q);
	return q;
}

static int square00(int16_t t)
{
//	printf("square00 %d = ", t);
	if (t > 181) {
		t /= 10;
		t *= t;
	} else {
		t *= t;
		t /= 100;
	}
//	printf("%d\n", t);
	return t;
}

/* Return the distance to an object in x.xx fixed point */
static int distance_to(int i)
{
	int j;

	/* We do the squares in fixed point maths */
	j = square00(TO_FIXED00(k[i][1]) - s1);
	j += square00(TO_FIXED00(k[i][2]) - s2);

	/* Find the integer square root */
	j = isqrt(j);
	/* Correct back into 0.00 fixed point */
	j *= 10;
	/* @@@ j = sqrt(pow((k[i][1] - s1), 2) + pow((k[i][2] - s2), 2)); */
//	j = (int) sqrt(pow((k[i][1] - s1), 2) + pow((k[i][2] - s2), 2));

	return j;
}


/* Round off floating point numbers instead of truncating */

static int cint100(int16_t d)
{
	return (d + 50) / 100;
}

static void showfile(char *filename)
{
	FILE *fp;
	line lBuffer;
	int iRow = 0;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		perror(filename);
		return;
	}
	while (fgets(lBuffer, sizeof(lBuffer), fp) != NULL) {
		fputs(lBuffer, stdout);
		if (iRow++ > MAXROW - 3) {
			getchar();
			iRow = 0;
		}
	}
	fclose(fp);
}

