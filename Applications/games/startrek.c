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
 *     - Arrays changed from G(8,8) to map[9][9] so indexes can
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
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#ifndef TREK_DIR
#define TREK_DIR	"/usr/lib/trek/"
#endif

/* Standard Terminal Sizes */

#define MAXROW      24
#define MAXCOL      80

struct klingon {
	uint8_t y;
	uint8_t x;
	int16_t energy;
};

/* Function Declarations */

static void intro(void);
static void new_game(void);
static void initialize(void);
static void new_quadrant(void);
static void course_control(void);
static void complete_maneuver(uint16_t, uint16_t);
static void maneuver_energy(uint16_t);
static void short_range_scan(void);
static void long_range_scan(void);
static void phaser_control(void);
static void photon_torpedoes(void);
static void torpedo_hit(uint8_t y, uint8_t x);
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
static void repair_damage(uint16_t warp);
static void find_set_empty_place(uint8_t t, uint8_t *z1, uint8_t *z2);
static const char *get_device_name(int n);
static void quadrant_name(uint8_t small, uint8_t y, uint8_t x);
static int distance_to(struct klingon *k);
static int square00(int16_t t);
static int cint100(int16_t d);
static void compute_vector(int16_t, int16_t, int16_t, int16_t);
static void showfile(char *filename);

/* Global Variables */

static int8_t starbases;		/* Starbases in Quadrant */
static uint8_t base_y, base_x;		/* Starbase Location in sector */
static int8_t starbases_left;		/* Total Starbases */

static int8_t c[3][10] =	/* Movement indices 1-9 (9 is wrap of 1) */
{
	{0},
	{0, 0, -1, -1, -1, 0, 1, 1, 1, 0},
	{1, 1, 1, 0, -1, -1, -1, 0, 1, 1}
};

static uint8_t docked;			/* Docked flag */
static int energy;			/* Current Energy */
static const int energy0 = 3000;	/* Starting Energy */
static unsigned int map[9][9];		/* Galaxy. BCD of k b s plus flag */
#define MAP_VISITED 0x1000		/* Set if this sector was mapped */
static struct klingon kdata[3];		/* Klingon Data */
static uint8_t klingons;		/* Klingons in Quadrant */
static uint8_t total_klingons;		/* Klingons at start */
static uint8_t klingons_left;		/* Total Klingons left */
static uint8_t torps;			/* Photon Torpedoes left */
static const uint8_t torps0 = 10;	/* Photon Torpedo capacity */
static int quad_y, quad_x;		/* Quadrant Position of Enterprise */
static int shield;			/* Current shield value */
static uint8_t stars;			/* Stars in quadrant */
static uint16_t time_start;		/* Starting Stardate */
static uint16_t time_up;		/* End of time */
static int16_t damage[9];		/* Damage Array */
static int16_t d4;			/* Used for computing damage repair time */
static int16_t ship_y, ship_x;		/* Current Sector Position of Enterprise, fixed point */
static uint16_t stardate;		/* Current Stardate */

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
 *	Returns an integer from 1 to spread
 */
static int get_rand(int spread)
{
	uint16_t r = rand();
	/* RAND_MAX is at least 15 bits, our largest request is for 500. The
	   classic unix rand() is very poor on the low bits so swap the ends
	   over */
	r = (r >> 8) | (r << 8);
	return ((r % spread) + 1);
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

	return (0);
}

static uint8_t inoperable(uint8_t u)
{
	if (damage[u] < 0) {
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
	stardate = TO_FIXED((get_rand(20) + 20) * 100);
}

static void new_game(void)
{
	char cmd[4];

	initialize();

	new_quadrant();

	short_range_scan();

	while (1) {
		if (shield + energy <= 10 && (energy < 10 || damage[7] < 0)) {
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
	static char plural_2[2] = "";
	static char plural[4] = "is";
	uint8_t yp, xp;

	/* Initialize time */

	time_start = FROM_FIXED(stardate);
	time_up = 25 + get_rand(10);

	/* Initialize Enterprise */

	docked = 0;
	energy = energy0;
	torps = torps0;
	shield = 0;

	quad_y = rand8();
	quad_x = rand8();
	ship_y = TO_FIXED00(rand8());
	ship_x = TO_FIXED00(rand8());

	for (i = 1; i <= 8; i++)
		damage[i] = 0;

	/* Setup What Exists in Galaxy */

	for (i = 1; i <= 8; i++) {
		for (j = 1; j <= 8; j++) {
			uint8_t r = get_rand(100);
			klingons = 0;
			if (r > 98)
				klingons = 3;
			else if (r > 95)
				klingons = 2;
			else if (r > 80)
				klingons = 1;

			klingons_left = klingons_left + klingons;
			starbases = 0;

			if (get_rand(100) > 96)
				starbases = 1;

			starbases_left = starbases_left + starbases;

			map[i][j] = (klingons << 8) + (starbases << 4) + rand8();
		}
	}

	/* Give more time for more Klingons */
	if (klingons_left > time_up)
		time_up = klingons_left + 1;

	/* Add a base if we don't have one */
	if (starbases_left == 0) {
		yp = rand8();
		xp = rand8();
		if (map[yp][xp] < 0x200) {
			map[yp][xp] += (1 << 8);
			klingons_left++;
		}

		map[yp][xp] += (1 << 4);
		starbases_left++;
	}

	total_klingons = klingons_left;

	if (starbases_left != 1) {
		strcpy(plural_2, "s");
		strcpy(plural, "are");
	}

	printf("Your orders are as follows:\n"
	       " Destroy the %d Klingon warships which have invaded\n"
	       " the galaxy before they can attack Federation Headquarters\n"
	       " on stardate %u. This gives you %d days. There %s\n"
	       " %d starbase%s in the galaxy for resupplying your ship.\n\n"
	       "Hit any key to accept command. ",
	       klingons_left, time_start + time_up, time_up, plural, starbases_left, plural_2);
	fflush(stdout);
	getchar();
}

static void place_ship(void)
{
	quad[FROM_FIXED00(ship_y) - 1][FROM_FIXED00(ship_x) - 1] = Q_SHIP;
}

static void new_quadrant(void)
{
	int i;
	uint16_t tmp;
	struct klingon *k = kdata;

	klingons = 0;
	starbases = 0;
	stars = 0;

	/* Random factor for damage repair. We compute it on each new
	   quadrant to stop the user just retrying until they get a number
	   they like. The conversion here was wrong and now copies the BASIC
	   code generate 0.00 to 0.49 */
	d4 = get_rand(50) - 1;

	/* Copy to computer */
	map[quad_y][quad_x] |= MAP_VISITED;

	if (quad_y >= 1 && quad_y <= 8 && quad_x >= 1 && quad_x <= 8) {
		quadrant_name(0, quad_y, quad_x);

		if (TO_FIXED(time_start) != stardate)
			printf("Now entering %s quadrant...\n\n", quadname);
		else {
			puts("\nYour mission begins with your starship located");
			printf("in the galactic quadrant %s.\n\n", quadname);
		}
	}

	tmp = map[quad_y][quad_x];
	klingons = (tmp >> 8) & 0x0F;
	starbases = (tmp >> 4) & 0x0F;
	stars = tmp & 0x0F;

	if (klingons > 0) {
		printf("Combat Area  Condition Red\n");

		if (shield < 200)
			printf("Shields Dangerously Low\n");
	}

	for (i = 1; i <= 3; i++) {
		k->y = 0;
		k->x = 0;
		k->energy = 0;
		k++;
	}

	memset(quad, Q_SPACE, 64);

	place_ship();
	
	if (klingons > 0) {
		k = kdata;
		for (i = 0; i < klingons; i++) {
			find_set_empty_place(Q_KLINGON, &k->y, &k->x);
			k->energy = 100 + get_rand(200);
			k++;
		}
	}

	if (starbases > 0)
		find_set_empty_place(Q_BASE, &base_y, &base_x);

	for (i = 1; i <= stars; i++)
		find_set_empty_place(Q_STAR, NULL, NULL);
}

static const char *inc_1 = "reports:\n  Incorrect course data, sir!\n";

static void course_control(void)
{
	register int i;
	int16_t c1;
	int16_t warp;
	uint16_t n;
	int c2, c3, c4;
	int16_t z1, z2;
	int16_t x1, x2;
	int16_t x, y;
	static char warpmax[4] = "8";

	fputs("Course (0-9): ", stdout);

	c1 = input_f00();

	if (c1 == 900)
		c1 = 100;

	if (c1 < 0 || c1 > 900) {
		printf("Lt. Sulu%s", inc_1);
		return;
	}

	if (damage[1] < 0)
		strcpy(warpmax, "0.2");

	printf("Warp Factor (0-%s): ", warpmax);

	warp = input_f00();

	if (damage[1] < 0 && warp > 20) {
		printf("Warp Engines are damaged. "
		       "Maximum speed = Warp 0.2.\n\n");
		return;
	}

	if (warp <= 0)
		return;

	if (warp > 800) {
		printf("Chief Engineer Scott reports:\n"
		       "  The engines won't take warp %s!\n\n", print100(warp));
		return;
	}

	n = warp * 8;

	n = cint100(n);	

	/* FIXME: should be  s + e - n > 0 iff shield control undamaged */
	if (energy - n < 0) {
		printf("Engineering reports:\n"
		       "  Insufficient energy available for maneuvering"
		       " at warp %s!\n\n", print100(warp));

		if (shield >= n && damage[7] >= 0) {
			printf("Deflector Control Room acknowledges:\n"
			       "  %d units of energy presently deployed to shields.\n", shield);
		}

		return;
	}

	klingons_move();

	repair_damage(warp);

	z1 = FROM_FIXED00(ship_y);
	z2 = FROM_FIXED00(ship_x);
	quad[z1-1][z2-1] = Q_SPACE;


	c2 = FROM_FIXED00(c1);	/* Integer part */
	c3 = c2 + 1;		/* Next integer part */
	c4 = (c1 - TO_FIXED00(c2));	/* Fractional element in fixed point */

	x1 = 100 * c[1][c2] + (c[1][c3] - c[1][c2]) * c4;
	x2 = 100 * c[2][c2] + (c[2][c3] - c[2][c2]) * c4;

	x = ship_y;
	y = ship_x;

	for (i = 1; i <= n; i++) {
//		printf(">%d %d %d %d %d\n",
//			i, ship_y, ship_x, x1, x2);
		ship_y = ship_y + x1;
		ship_x = ship_x + x2;

//		printf("=%d %d %d %d %d\n",
//			i, ship_y, ship_x, x1, x2);

		z1 = FROM_FIXED00(ship_y);
		z2 = FROM_FIXED00(ship_x);	/* ?? cint100 ?? */

		/* Changed quadrant */
		if (z1 < 1 || z1 >= 9 || z2 < 1 || z2 >= 9) {
			uint8_t outside = 0;		/* Outside galaxy flag */
			uint8_t quad_y_old = quad_y;
			uint8_t quad_x_old = quad_x;

			x = (800 * quad_y) + x + (n * x1);
			y = (800 * quad_x) + y + (n * x2);

			//	printf("X %d Y %d\n", x, y);

			quad_y = x / 800;	/* Fixed point to int and divide by 8 */
			quad_x = y / 800;	/* Ditto */

			//	printf("Q %d %d\n", quad_y, quad_x);

			ship_y = x - (quad_y * 800);
			ship_x = y - (quad_x * 800);

			//	printf("S %d %d\n", ship_y, ship_x);

			if (ship_y < 100) {
				quad_y = quad_y - 1;
				ship_y = ship_y + 800;
			}

			if (ship_x < 100) {
				quad_x = quad_x - 1;
				ship_x = ship_x + 800;
			}

			/* check if outside galaxy */

			if (quad_y < 1) {
				outside = 1;
				quad_y = 1;
				ship_y = 100;
			}

			if (quad_y > 8) {
				outside = 1;
				quad_y = 8;
				ship_y = 800;
			}

			if (quad_x < 1) {
				outside = 1;
				quad_x = 1;
				ship_x = 100;
			}

			if (quad_x > 8) {
				outside = 1;
				quad_x = 8;
				ship_x = 800;
			}

			if (outside == 1) {
				/* Mostly showfile ? FIXME */
				printf("LT. Uhura reports:\n"
		       "  Message from Starfleet Command:\n\n"
		       "  Permission to attempt crossing of galactic perimeter\n"
		       "  is hereby *denied*. Shut down your engines.\n\n"
		       "Chief Engineer Scott reports:\n"
		       "  Warp Engines shut down at sector %d, "
		       "%d of quadrant %d, %d.\n\n",
				       FROM_FIXED00(ship_y),
				       FROM_FIXED00(ship_x), quad_y, quad_x);
			}
			maneuver_energy(n);

			/* this section has a different order in the original.
			   t = t + 1;

			   if (t > time_start + time_up)
			   end_of_time();
			 */

			if (FROM_FIXED(stardate) > time_start + time_up)
				end_of_time();

			if (quad_y != quad_y_old || quad_x != quad_x_old) {
				stardate = stardate + TO_FIXED(1);
				new_quadrant();
			}
			complete_maneuver(warp, n);
			return;
		}

		if (quad[z1-1][z2-1] != Q_SPACE) {	/* Sector not empty */
			ship_y = ship_y - x1;
			ship_x = ship_x - x2;
			printf("Warp Engines shut down at sector "
			       "%d, %d due to bad navigation.\n\n", z1, z2);
			i = n + 1;
		}
	}
	complete_maneuver(warp, n);
}

static void complete_maneuver(uint16_t warp, uint16_t n)
{
	uint16_t time_used;

	place_ship();
	maneuver_energy(n);

	time_used = TO_FIXED(1);

	/* Ick FIXME - re really want to tidy up time to FIXED00 */
	if (warp < 100)
		time_used = TO_FIXED(FROM_FIXED00(warp));

	stardate += time_used;

	if (FROM_FIXED(stardate) > time_start + time_up)
		end_of_time();

	short_range_scan();
}


static void maneuver_energy(uint16_t n)
{
	energy -= n + 10;

	if (energy >= 0)
		return;

	/* FIXME:
	   This never occurs with the nav code as is - ancient trek versions
	   included shield power in movement allowance if shield control
	   was undamaged */
	puts("Shield Control supplies energy to complete maneuver.\n");

	shield = shield + energy;
	energy = 0;

	if (shield <= 0)
		shield = 0;
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
	char *sC = "GREEN";

	if (energy < energy0 / 10)
		sC = "YELLOW";

	if (klingons > 0)
		sC = "*RED*";

	docked = 0;

	for (i = (int) (FROM_FIXED00(ship_y) - 1); i <= (int) (FROM_FIXED00(ship_y) + 1); i++)
		for (j = (int) (FROM_FIXED00(ship_x) - 1); j <= (int) (FROM_FIXED00(ship_x) + 1); j++)
			if (i >= 1 && i <= 8 && j >= 1 && j <= 8) {
				if (quad[i-1][j-1] == Q_BASE) {
					docked = 1;
					sC = "DOCKED";
					energy = energy0;
					torps = torps0;
					puts("Shields dropped for docking purposes.");
					shield = 0;
				}
			}

	if (damage[2] < 0) {
		puts("\n*** Short Range Sensors are out ***");
		return;
	}

	puts(srs_1);
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++)
			fputs(tilestr[quad[i][j]], stdout);

		if (i == 0)
			printf("    Stardate            %d\n", FROM_FIXED(stardate));
		if (i == 1)
			printf("    Condition           %s\n", sC);
		if (i == 2)
			printf("    Quadrant            %d, %d\n", quad_y, quad_x);
		if (i == 3)
			printf("    Sector              %d, %d\n", FROM_FIXED00(ship_y), FROM_FIXED00(ship_x));
		if (i == 4)
			printf("    Photon Torpedoes    %d\n", torps);
		if (i == 5)
			printf("    Total Energy        %d\n", energy + shield);
		if (i == 6)
			printf("    Shields             %d\n", shield);
		if (i == 7)
			printf("    Klingons Remaining  %d\n", klingons_left);
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

	printf("Long Range Scan for Quadrant %d, %d\n\n", quad_y, quad_x);

	for (i = quad_y - 1; i <= quad_y + 1; i++) {
		printf("%s:", lrs_1);
		for (j = quad_x - 1; j <= quad_x + 1; j++) {
			putchar(' ');
			if (i > 0 && i <= 8 && j > 0 && j <= 8) {
				map[i][j] |= MAP_VISITED;
				putbcd(map[i][j]);
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
	if (klingons <= 0) {
		puts("Science Officer Spock reports:\n"
		     "  'Sensors show no enemy ships in this quadrant'\n");
		return 1;
	}
	return 0;
}

static void wipe_klingon(struct klingon *k)
{
	quad[k->y-1][k->x-1] = Q_SPACE;
}

static void phaser_control(void)
{
	register int i;
	int32_t phaser_energy;
	uint32_t h1;
	int h;
	struct klingon *k = kdata;

	if (inoperable(4))
		return;

	if (no_klingon())
		return;

	/* There's Klingons on the starboard bow... */
	if (damage[8] < 0)
		puts("Computer failure hampers accuracy.");

	printf("Phasers locked on target;\n"
	       "Energy available = %d units\n\n"
	       "Number of units to fire: ", energy);

	phaser_energy = input_int();

	if (phaser_energy <= 0)
		return;

	if (energy - phaser_energy < 0) {
		puts("Not enough energy available.\n");
		return;
	}

	energy -=  phaser_energy;

	/* We can fire up to nearly 3000 points of energy so we do this
	   bit in 32bit math */

	if (damage[8] < 0)
		phaser_energy *= get_rand(100);
	else
		phaser_energy *= 100;

	h1 = phaser_energy / klingons;

	for (i = 0; i <= 2; i++) {
		if (k->energy > 0) {
			/* We are now 32bit with four digits after the point */
			h1 = h1 * (get_rand(100) + 200);

			/* Takes us down to 2 digit accuracy */
			h1 /= distance_to(k);

			if (h1 <= 15 * k->energy) {	/* was 0.15 */
				printf("Sensors show no damage to enemy at "
				       "%d, %d\n\n", k->y, k->x);
			} else {
				h = FROM_FIXED00(h1);
				k->energy -= h;
				printf("%d unit hit on Klingon at sector "
				       "%d, %d\n",
					h, k->y, k->x);
				if (k->energy <= 0) {
					puts("*** Klingon Destroyed ***\n");
					klingons--;
					klingons_left--;
					wipe_klingon(k);
					k->energy = 0;
					/* Minus a Klingon.. */
					map[quad_y][quad_x] -= 0x100;
					if (klingons_left <= 0)
						won_game();
				} else
					printf("   (Sensors show %d units remaining.)\n\n", k->energy);
			}
		}
		k++;
	}

	klingons_shoot();
}

static void photon_torpedoes(void)
{
	int x3, y3;
	int16_t c1;
	int c2, c3, c4;
	int16_t x, y, x1, x2;

	if (torps <= 0) {
		puts("All photon torpedoes expended");
		return;
	}

	if (inoperable(5))
		return;

	fputs("Course (0-9): ", stdout);

	c1 = input_f00();

	if (c1 == 900)
		c1 = 100;

	if (c1 < 100 || c1 >= 900) {
		printf("Ensign Chekov%s", inc_1);
		return;
	}

	/* FIXME: energy underflow check ? */
	energy = energy - 2;
	torps--;

	c2 = FROM_FIXED00(c1);	/* Integer part */
	c3 = c2 + 1;		/* Next integer part */
	c4 = (c1 - TO_FIXED00(c2));	/* Fractional element in fixed point */

	x1 = 100 * c[1][c2] + (c[1][c3] - c[1][c2]) * c4;
	x2 = 100 * c[2][c2] + (c[2][c3] - c[2][c2]) * c4;

	/* The basic code is very confused about what is X and what is Y */
	x = ship_y + x1;
	y = ship_x + x2;

	x3 = FROM_FIXED00(x);
	y3 = FROM_FIXED00(y);

	puts("Torpedo Track:");

	while (x3 >= 1 && x3 <= 8 && y3 >= 1 && y3 <= 8) {
		uint8_t p;

		printf("    %d, %d\n", x3, y3);

		p = quad[x3-1][y3-1];
		/* In certain corner cases the first trace we'll step is
		   ourself. If so treat it as space */
		if (p != Q_SPACE && p != Q_SHIP) {
			torpedo_hit(x3, y3);
			klingons_shoot();
			return;
		}

		x = x + x1;
		y = y + x2;

		x3 = FROM_FIXED00(x);
		y3 = FROM_FIXED00(y);
	}

	puts("Torpedo Missed\n");

	klingons_shoot();
}

static void torpedo_hit(uint8_t yp, uint8_t xp)
{
	int i;
	struct klingon *k;

	switch(quad[yp-1][xp-1]) {
	case Q_STAR:
		printf("Star at %d, %d absorbed torpedo energy.\n\n", yp, xp);
		return;
	case Q_KLINGON:
		puts("*** Klingon Destroyed ***\n");
		klingons--;
		klingons_left--;

		if (klingons_left <= 0)
			won_game();

		k = kdata;
		for (i = 0; i <= 2; i++) {
			if (yp == k->y && xp == k->x)
				k->energy = 0;
			k++;
		}
		map[quad_y][quad_x] -= 0x100;
		break;
	case Q_BASE:					
		puts("*** Starbase Destroyed ***");
		starbases--;
		starbases_left--;

		if (starbases_left <= 0 && klingons_left <= FROM_FIXED(stardate) - time_start - time_up) {
			/* showfile ? FIXME */
			puts("That does it, Captain!!"
			     "You are hereby relieved of command\n"
			     "and sentenced to 99 stardates of hard"
			     "labor on Cygnus 12!!\n");
			resign_commision();
		}

		puts("Starfleet Command reviewing your record to consider\n"
		     "court martial!\n");

		docked = 0;		/* Undock */
		map[quad_y][quad_x] -= 0x10;
		break;
	}
	quad[yp-1][xp-1] = Q_SPACE;
}

static void damage_control(void)
{
	int16_t repair_cost = 0;
	register int i;

	if (damage[6] < 0)
		puts("Damage Control report not available.");

	/* Offer repair if docked */
	if (docked) {
		/* repair_cost is x.xx fixed point */
		repair_cost = 0;
		for (i = 1; i <= 8; i++)
			if (damage[i] < 0)
				repair_cost = repair_cost + 10;

		if (repair_cost) {
			repair_cost = repair_cost + d4;
			if (repair_cost >= 100)
				repair_cost = 90;	/* 0.9 */

			printf("\nTechnicians standing by to effect repairs to your"
			       "ship;\nEstimated time to repair: %s stardates.\n"
			       "Will you authorize the repair order (y/N)? ", print100(repair_cost));

			if (yesno()) {
				for (i = 1; i <= 8; i++)
					if (damage[i] < 0)
						damage[i] = 0;

				/* Work from two digit to one digit. We might actually
				   have to give in and make t a two digt offset from
				   a saved constant base only used in printing to
				   avoid that round below FIXME */
				stardate += (repair_cost + 5)/10 + 1;
			}
			return;
		}
	}

	if (damage[6] < 0)
		return;

	puts("Device            State of Repair");

	for (i = 1; i <= 8; i++)
		printf("%-25s%6s\n", get_device_name(i), print100(damage[i]));

	printf("\n");
}

static void shield_control(void)
{
	int i;

	if (inoperable(7))
		return;

	printf("Energy available = %d\n\n"
	       "Input number of units to shields: ", energy + shield);

	i = input_int();

	if (i < 0 || shield == i) {
unchanged:
		puts("<Shields Unchanged>\n");
		return;
	}

	if (i >= energy + shield) {
		puts("Shield Control Reports:\n"
		     "  'This is not the Federation Treasury.'");
		goto unchanged;
	}

	energy = energy + shield - i;
	shield = i;

	printf("Deflector Control Room report:\n"
	       "  'Shields now at %d units per your command.'\n\n", shield);
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

	printf("\n     Computer Record of Galaxy for Quadrant %d,%d\n\n", quad_y, quad_x);
	puts("     1     2     3     4     5     6     7     8");

	for (i = 1; i <= 8; i++) {
		printf("%s%d", gr_1, i);

		for (j = 1; j <= 8; j++) {
			printf("   ");

			if (map[i][j] & MAP_VISITED)
				putbcd(map[i][j]);
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
	uint16_t left = TO_FIXED(time_start + time_up) - stardate;

	puts("   Status Report:\n");

	if (klingons_left > 1)
		plural = str_s;

	/* Assumes fixed point is single digit fixed */
	printf("Klingon%s Left: %d\n"
	       "Mission must be completed in %d.%d stardates\n",
		plural, klingons_left,
		FROM_FIXED(left), left%10);

	if (starbases_left < 1) {
		puts("Your stupidity has left you on your own in the galaxy\n"
		     " -- you have no starbases left!\n");
	} else {
		plural = str_s;
		if (starbases_left < 2)
			plural++;

		printf("The Federation is maintaining %d starbase%s in the galaxy\n\n", starbases_left, plural);
	}
}

static void torpedo_data(void)
{
	int i;
	const char *plural = str_s + 1;
	struct klingon *k;

	/* Need to also check sensors here ?? */
	if (no_klingon())
		return;

	if (klingons > 1)
		plural--;

	printf("From Enterprise to Klingon battlecriuser%s:\n\n", plural);

	k = kdata;
	for (i = 0; i <= 2; i++) {
		if (k->energy > 0) {
			compute_vector(TO_FIXED00(k->y),
				       TO_FIXED00(k->x),
				       ship_y, ship_x);
		}
		k++;
	}
}

static void nav_data(void)
{
	if (starbases <= 0) {
		puts("Mr. Spock reports,\n"
		     "  'Sensors show no starbases in this quadrant.'\n");
		return;
	}
	compute_vector(TO_FIXED00(base_y), TO_FIXED00(base_x), ship_y, ship_x);
}

/* Q: do we want to support fractional co-ords ? */
static void dirdist_calc(void)
{
	int16_t c1, a, w1, x;
	printf("Direction/Distance Calculator\n"
	       "You are at quadrant %d,%d sector %d,%d\n\n"
	       "Please enter initial X coordinate: ",
	       quad_y, quad_x,
	       FROM_FIXED00(ship_y), FROM_FIXED00(ship_x));

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
	compute_vector(w1, x, c1, a);
}

static const char *gm_1 = "  ----- ----- ----- ----- ----- ----- ----- -----\n";

static void galaxy_map(void)
{
	int i, j, j0;

	printf("\n                   The Galaxy\n\n");
	printf("    1     2     3     4     5     6     7     8\n");

	for (i = 1; i <= 8; i++) {
		printf("%s%d ", gm_1, i);

		quadrant_name(1, i, 1);

		j0 = (int) (11 - (strlen(quadname) / 2));

		for (j = 0; j < j0; j++)
			putchar(' ');

		fputs(quadname, stdout);

		for (j = 0; j < j0; j++)
			putchar(' ');

		if (!(strlen(quadname) % 2))
			putchar(' ');

		quadrant_name(1, i, 5);

		j0 = (int) (12 - (strlen(quadname) / 2));

		for (j = 0; j < j0; j++)
			putchar(' ');

		puts(quadname);
	}

	puts(gm_1);

}

static const char *dist_1 = "  DISTANCE = %s\n\n";

static void compute_vector(int16_t w1, int16_t x, int16_t c1, int16_t a)
{
	uint32_t xl, al;

	fputs("  DIRECTION = ", stdout);
	/* All this is happening in fixed point */
	x = x - a;
	a = c1 - w1;

	xl = abs(x);
	al = abs(a);

	if (x < 0) {
		if (a > 0) {
			c1 = 300;
estimate2:
		/* Multiply the top half by 100 to keep in fixed0 */
			if (al >= xl)
				printf("%s", print100(c1 + ((xl * 100) / al)));
			else
				printf("%s", print100(c1 + ((((xl * 2) - al) * 100)  / xl)));

			printf(dist_1, print100((x > a) ? x : a));
			return;
		} else if (x != 0){
			c1 = 500;
			goto estimate1;
			return;
		} else {
			c1 = 700;
			goto estimate2;
		}
	} else if (a < 0) {
		c1 = 700;
		goto estimate2;
	} else if (x > 0) {
		c1 = 100;
		goto estimate1;
	} else if (a == 0) {
		c1 = 500;
		goto estimate1;
	} else {
		c1 = 100;
estimate1:
		/* Multiply the top half by 100 as well so that we keep it in fixed00
		   format. Our larget value is int 9 (900) so we must do this 32bit */
		if (al <= xl)
			printf("%s", print100(c1 + ((al * 100) / xl)));
		else
			printf("%s", print100(c1 + ((((al * 2) - xl) * 100) / al)));
		printf(dist_1, print100((xl > al) ? xl : al));
	}
}
static void ship_destroyed(void)
{
	puts("The Enterprise has been destroyed. "
	     "The Federation will be conquered.\n");

	end_of_time();
}

static void end_of_time(void)
{
	printf("It is stardate %d.\n\n",  FROM_FIXED(stardate));

	resign_commision();
}

static void resign_commision(void)
{
	printf("There were %d Klingon Battlecruisers left at the"
	       " end of your mission.\n\n", klingons_left);

	end_of_game();
}

static void won_game(void)
{
	puts("Congratulations, Captain!  The last Klingon Battle Cruiser\n"
	     "menacing the Federation has been destoyed.\n");

	if (FROM_FIXED(stardate) - time_start > 0)
		printf("Your efficiency rating is %s\n",
			print100(square00(TO_FIXED00(total_klingons)/(FROM_FIXED(stardate) - time_start))));
		// 1000 * pow(total_klingons / (float)(FROM_FIXED(t) - time_start), 2));
	end_of_game();
}

static void end_of_game(void)
{
	char x[4];
	if (starbases_left > 0) {
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
	struct klingon *k = kdata;

	for (i = 0; i <= 2; i++) {
		if (k->energy > 0) {
			wipe_klingon(k);

			find_set_empty_place(Q_KLINGON, &k->y, &k->x);
		}
		k++;
	}

	klingons_shoot();
}

static const char *dcr_1 = "Damage Control report:";

static void klingons_shoot(void)
{
	uint32_t h;
	uint8_t i;
	struct klingon *k = kdata;

	if (klingons <= 0)
		return;

	if (docked) {
		puts("Starbase shields protect the Enterprise\n");
		return;
	}

	for (i = 0; i <= 2; i++) {
		if (k->energy > 0) {
			h = k->energy * (200UL + get_rand(100));
			h *= 100;	/* Ready for division in fixed */
			h /= distance_to(k);
			/* Takes us back into FIXED00 */
			shield = shield - FROM_FIXED00(h);

			k->energy = (k->energy * 100) / (300 + get_rand(100));

			printf("%d unit hit on Enterprise from sector "
			       "%d, %d\n", (unsigned)FROM_FIXED00(h), k->y, k->x);

			if (shield <= 0) {
				putchar('\n');
				ship_destroyed();
			}

			printf("    <Shields down to %d units>\n\n", shield);

			if (h >= 20) {
				/* The check in basic is float and is h/s >.02. We
				   have to use 32bit values here to avoid an overflow
				   FIXME: use a better algorithm perhaps ? */
				uint32_t ratio = ((uint32_t)h)/shield;
				if (get_rand(10) <= 6 && ratio > 2) {
					uint8_t r = rand8();
					/* The original basic code computed h/s in
					   float form the C conversion broke this. We correct it in the fixed
					   point change */
					damage[r] -= ratio + get_rand(50);

					/* FIXME: can we use dcr_1 here ?? */
					printf("Damage Control reports\n"
					       "   '%s' damaged by hit\n\n", get_device_name(r));
				}
			}
		}
		k++;
	}
}

static void repair_damage(uint16_t warp)
{
	int i;
	int d1;
	uint16_t repair_factor;		/* Repair Factor */

	repair_factor = warp;
	if (warp >= 100)
		repair_factor = TO_FIXED00(1);

	for (i = 1; i <= 8; i++) {
		if (damage[i] < 0) {
			damage[i] = damage[i] + repair_factor;
			if (damage[i] > -10 && damage[i] < 0)	/* -0.1 */
				damage[i] = -10;
			else if (damage[i] >= 0) {
				if (d1 != 1) {
					d1 = 1;
					puts(dcr_1);
				}
				printf("    %s repair completed\n\n",
					get_device_name(i));
				damage[i] = 0;
			}
		}
	}

	if (get_rand(10) <= 2) {
		uint8_t r = rand8();

		if (get_rand(10) < 6) {
			/* Working in 1/100ths */
			damage[r] -= (get_rand(500) + 100);
			puts(dcr_1);
			printf("    %s damaged\n\n", get_device_name(r));
		} else {
			/* Working in 1/100ths */
			damage[r] += get_rand(300) + 100;
			puts(dcr_1);
			printf("    %s state of repair improved\n\n",
					get_device_name(r));
		}
	}
}

/* Misc Functions and Subroutines
   Returns co-ordinates r1/r2 and for now z1/z2 */

static void find_set_empty_place(uint8_t t, uint8_t *z1, uint8_t *z2)
{
	uint8_t r1, r2;
	do {
		r1 = rand8();
		r2 = rand8();
	} while (quad[r1-1][r2-1] != Q_SPACE );
	quad[r1-1][r2-1] = t;
	if (z1)
		*z1 = r1;
	if (z2)
		*z2 = r2;
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

static void quadrant_name(uint8_t small, uint8_t y, uint8_t x)
{

	static char *sect_name[] = { "", " I", " II", " III", " IV" };

	if (y < 1 || y > 8 || x < 1 || x > 8)
		strcpy(quadname, "Unknown");

	if (x <= 4)
		strcpy(quadname, quad_name[y]);
	else
		strcpy(quadname, quad_name[y + 8]);

	if (small != 1) {
		if (x > 4)
			x = x - 4;
		strcat(quadname, sect_name[x]);
	}

	return;
}

/* An unsigned sqrt is all we need.

   What we are actually doing here is a smart version of calculating n^2
   repeatedly until we find the right one */
static int16_t isqrt(int16_t i)
{
	uint16_t b = 0x4000, q = 0, r = i, t;
	while (b) {
		t = q + b;
		q >>= 1;
		if (r >= t) {
			r -= t;
			q += b;
		}
		b >>= 2;
	}
	return q;
}

static int square00(int16_t t)
{
	if (abs(t) > 181) {
		t /= 10;
		t *= t;
	} else {
		t *= t;
		t /= 100;
	}
	return t;
}

/* Return the distance to an object in x.xx fixed point */
static int distance_to(struct klingon *k)
{
	uint16_t j;

	/* We do the squares in fixed point maths */
	j = square00(TO_FIXED00(k->y) - ship_y);
	j += square00(TO_FIXED00(k->x) - ship_x);

	/* Find the integer square root */
	j = isqrt(j);
	/* Correct back into 0.00 fixed point */
	j *= 10;

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
	char buffer[MAXCOL];
	int row = 0;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		perror(filename);
		return;
	}
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		fputs(buffer, stdout);
		if (row++ > MAXROW - 3) {
			getchar();
			row = 0;
		}
	}
	fclose(fp);
}

