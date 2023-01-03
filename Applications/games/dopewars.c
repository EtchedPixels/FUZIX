/*
 *	A dopewars rewrite.
 *
 *	John Dell's Drug Wars was remade as a DOS game by Happy Hacker
 *	Foundation and then grew all kinds of BBS door and other variants
 *
 *	This rework aims to be close to the spirit of the Happy Hacker version
 *	and Matthew Lee's PalmOS version.
 *
 *	It implements
 *	- Drug trading
 *	- Events in keeping with the BBS/Palm version
 *	- Guns and coat upgrades
 *	- Multiple locations
 *	- Bank
 *	- Loan shark
 *	- 31 day timer
 *	- Actually make it random (fixed rand sequence is just handy for debug)
 *	- Cops
 *	- Hospital
 *
 *	Things To Finish
 *	- Save/load ?
 *
 *	Where there are deviations it generally favours the PalmOS version
 *
 *	- Language is neutral. Many BBS versions used fairly crass by modern
 *	  standards mock ghetto language
 *	- No network player and modern extensions
 *	- Currently locations are all the same (except that only one spot
 *	  has the bank and loan shark). May tweak this to follow the versions
 *	  that have higher/lower risk-return places
 *	- No annoying woman on the subway
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

typedef long money;

struct good {
	const char *name;
	unsigned base;
	unsigned plus;
	unsigned price;
	unsigned owned;
};

struct pricemod {
	unsigned chance;
	const char *text;
	unsigned id;		/* Good affected */
	int mod;
	int found;
};


#define GID_ACID	0
#define GID_COCAINE	1
#define GID_LUDES	2
#define GID_PCP		3
#define GID_HEROIN	4
#define GID_WEED	5
#define GID_SHROOMS	6
#define GID_SPEED	7

struct good good[] = {
	{ "Acid", 1000, 3500, },
	{ "Cocaine", 15000, 15000, },
	{ "Ludes", 10, 50, },
	{ "PCP", 1000, 2500, },
	{ "Heroin", 5000, 9000, },
	{ "Weed", 300, 600, },
	{ "Mushrooms", 600, 760, },
	{ "Speed", 70, 180, },
	{ NULL }
};

struct pricemod pricemod[] = {
	{ 13, "The cops just did a big Weed bust!  Prices are sky-high!",
	 GID_WEED, 4, 0 },
	{ 20, "The cops just did a big PCP bust!  Prices are sky-high!",
	 GID_PCP, 4, 0 },
	{ 25, "The cops just did a big Heroin bust!  Prices are sky-high!",
	 GID_HEROIN, 4, 0 },
	{ 13, "The cops just did a big Ludes bust!  Prices are sky-high!",
	 GID_LUDES, 4, 0 },
	{ 35,
	 "The cops just did a big Cocaine bust!  Prices are sky-high!",
	 GID_COCAINE, 4, 0 },
	{ 15, "The cops just did a big Speed bust!  Prices are sky-high!",
	 GID_SPEED, 4, 0 },
	{ 25, "Addicts are buying Heroin at outrageous prices!",
	 GID_HEROIN, 8, 0 },
	{ 20, "Addicts are buying Speed at outrageous prices!", GID_SPEED,
	 8, 0 },
	{ 20, "Addicts are buying PCP at outrageous prices!", GID_PCP, 8,
	 0 },
	{ 17, "Addicts are buying Shrooms at outrageous prices!",
	 GID_SHROOMS, 8, 0 },
	{ 35, "Addicts are buying Cocaine at outrageous prices!",
	 GID_COCAINE, 8, 0 },
	{ 17, "The market has been flooded with cheap home-made Acid!",
	 GID_ACID, -8, 0 },
	{ 10,
	 "A Columbian freighter dusted the Coast Guard!  Weed prices have bottomed out!",
	 GID_WEED, -4, 0 },
	{ 11,
	 "A gang raided a local pharmacy and are selling cheap Ludes!",
	 GID_LUDES, -8, 0 },
	{ 55, "You found some Cocaine on a dead dude in the subway!",
	 GID_COCAINE, 0, 3 },
	{ 45, "You found some Acid on a dead dude in the subway!",
	 GID_ACID, 0, 6 },
	{ 35, "You found some PCP on a dead dude in the subway!", GID_PCP,
	 0, 4 },
	{ 0, NULL }
};

const char *locations[] = {
	"Bronx",
	"Ghetto",
	"Central Park",
	"Manhattan",
	"Coney Island",
	"Brooklyn",
	"Hospital"
};

const char *locstr = "123456";	/* Can't select Hospital */

const char *goodstr = "aclphwmp";

#define NO_ROOM		"You don't have room in your trenchcoat."
#define NOT_ENOUGH_CASH	"You don't have that much cash"
#define NOT_ENOUGH_BANK	"You don't have that much in the bank"
#define BUY_DONE	"OK"
#define NOT_ENOUGH	"You don't have that much to sell."
#define SELL_DONE	"OK"
#define BAD_ITEM	"I don't know what that is."
#define WHICH		"Which drug (/ to return): "
#define NOBUYERS 	"No buyers."
#define NOSELLERS	"No sellers."
#define NO_LEND_MORE	"The loan shark won't lend you any more."
#define NO_LEND_TODAY	"The loan shark already loaned you money today."
#define NOT_OWED	"You don't owe the loan shark that much."
#define NOT_LEND_THIS	"The loan shark won't lend you that much."
#define COP_DOWN	"You shoot and injure a cop, man down."
#define MISSED_COP	"You shoot and miss."
#define NO_GUN		"You have no gun."
#define COPS_KILL	"Officer Hardass guns you down."
#define COPS_WOUND	"The cops wound you and bring you to heel."
#define COPS_MISS	"Bullets zing past your ear."
#define COPS_TAKE_ALL	"Officer Hardass relieves you of all your drugs and money."
#define COPS_TAKE_HALF	"Officer Hardass relieves you of all your durgs and half your money."
#define COP_NO_GUN	"Officer Hardass takes your gun away."
#define ESCAPED		"You escape into a back alley."

#define SHARK_LEVERAGE	400	/* 400 % Classic, up to 3000 in some versions */

unsigned location;
unsigned gun;
unsigned space;
unsigned coatsize;
unsigned days = 1;
unsigned last_borrow;
unsigned dead = 0;
unsigned cops = 3;
money cash;
money debt;
money bank;

/*
 *	Helpers
 */

unsigned chance(unsigned n)
{
	if ((rand() >> 3) % 100 < n)
		return 1;
	return 0;
}

int random_range(int low, int high)
{
	int v = (rand() >> 2) % high;
	v += low;
	return v;
}

char getch(void)
{
	char buf[32];
	if (fgets(buf, 32, stdin) == NULL)
		exit(1);
	return *buf;
}

unsigned yesno(char *p)
{
	char c;
	do {
		printf("%s", p);
		c = getch();
		if (c == 'y')
			return 1;
	} while(c != 'n');
	return 0;
}

char *price(money n)
{
	static char buf[32];
	if (n < 100 && n > -100)
		snprintf(buf, 32, "%ldc", n);
	else
		snprintf(buf, 32, "$%ld.%02ld", n / 100, n % 100);
	return buf;
}

unsigned quantity(unsigned *n, const char *op)
{
	char buf[32];
	printf("How much do you want to %s ? ", op);
	if (fgets(buf, 32, stdin) == NULL)
		exit(1);
	if (*buf == '\n')
		return 0;
	*n = atoi(buf);
	return 1;
}

unsigned quantity_cash(money *n, const char *op)
{
	char buf[32];
	char *bufp = buf;
	money sum = 0;
	printf("How much do you want to %s ? ", op);
	if (fgets(buf, 32, stdin) == NULL)
		exit(1);
	while(*bufp && isspace(*bufp))
		bufp++;
	if (*bufp == 0)
		return 0;
	if (*bufp == '$')
		bufp++;
	while(isdigit(*bufp)) {
		sum *= 10;
		sum += *bufp - '0';
		bufp++;
	}
	if (*bufp != 'c') {
		unsigned cents = 0;
		sum *= 100;
		if (*bufp == '.') {
			bufp++;
			if (isdigit(*bufp)) {
				cents = *bufp++ - '0';
				if (isdigit(*bufp)) {
					cents *= 10;
					cents += *bufp - '0';
				}
			}
		}
		sum += cents;
	}	
	*n = sum;
	return 1;
}

void error(const char *p)
{
	puts(p);
}

void info(const char *p)
{
	puts(p);
}

/*
 *	Generic trade functions
 */

void price_goods(int leaveout)
{
	struct good *g = good;
	while (g->name) {
		g->price = random_range(g->base, g->plus);
		g++;
	}
	while (leaveout--)
		good[(rand() >> 2) % 7].price = 0;
}

void price_modifier(struct pricemod *m)
{
	printf("%s\n", m->text);
	/* Shortcut - unavailable items are 0 so will stay 0 */
	if (m->mod < 0)
		good[m->id].price /= -m->mod;
	if (m->mod > 0)
		good[m->id].price *= m->mod;
	if (m->found) {
		if (space <= m->found) {
			printf("Unfortunately you can't carry it all.\n");
			good[m->id].owned += space;
			space = 0;
		} else {
			good[m->id].owned += m->found;
			space -= m->found;
		}
	}
}

void do_modifiers(void)
{
	struct pricemod *m = pricemod;
	while (m->text) {
		if (chance(m->chance))
			price_modifier(m);
		m++;
	}
}

void show_inventory(unsigned all)
{
	struct good *g = good;
	const char *p = goodstr;

	while (g->name) {
		if ((all && g->owned) || g->price)
			printf("%c %-25s %-3d %s\n", *p, g->name, g->owned,
			       g->price ? price(g->price): "");
		p++;
		g++;
	}
	printf("\n\n");
}

struct good *select_goods(void)
{
	char c;
	char *p;

	do {
		show_inventory(0);
		printf("[%s] ", price(cash));
		fputs(WHICH, stdout);
		c = getch();
		p = strchr(goodstr, c);

		if (c == '/' || c == '\n')
			return NULL;
		if (p == NULL)
			error(BAD_ITEM);
	} while (p == NULL);

	return good + (p - goodstr);
}

void buy(void)
{
	struct good *g;
	unsigned n;
	while ((g = select_goods()) != NULL) {
		if (g->price == 0) {
			error(NOBUYERS);
			continue;
		}
		if (!quantity(&n, "buy"))
			return;
		if (n > space) {
			error(NO_ROOM);
			continue;
		}
		if (n * g->price > cash) {
			error(NOT_ENOUGH_CASH);
			continue;
		}
		cash -= g->price * n;
		space -= n;
		g->owned += n;
		info(BUY_DONE);
	}
}

void sell(void)
{
	struct good *g;
	unsigned n;
	while ((g = select_goods()) != NULL) {
		if (g->price == 0) {
			error(NOBUYERS);
			continue;
		}
		if (!quantity(&n, "sell"))
			return;
		if (g->owned < n) {
			error(NOT_ENOUGH);
			continue;
		}
		cash += g->price * n;
		space += n;
		g->owned -= n;
		info(SELL_DONE);
	}
}

/* There are various versions of this including ones you can bribe or flee */

void police(void)
{
	char c;
	unsigned shotat = 0;
	unsigned caught = 0;

	while(cops && !caught) {
		printf("Officer Hardass ");
		if (cops == 2)
			printf("and his deputy ");
		else if (cops == 3)
			printf("and his two deputies ");
		printf("are on your tail.\n");
		printf("r)un for it, or f)ight > ");
		c = getch();
		if (c == 'f') {
			if (!gun) {
				error(NO_GUN);
				continue;
			}
			shotat = 1;
			if (chance(25)) {
				cops--;
				info(COP_DOWN);
				if (cops == 0)
					return;
			} else {
				info(MISSED_COP);
				/* Missed */
			}
		}
		if (c == 'r') {
			if (chance(33)) {
				if (chance(20))
					caught = 1;
				/* Still chasing */
			} else {
				info(ESCAPED);
				return;
			}
		}
		/* If you've shot at them then they shoot back */
		if (shotat && !caught) {
			if (chance(20)) {
				if (chance(20)) {
					info(COPS_KILL);
					dead = 1;
				}
				info(COPS_WOUND);
				caught = 1;
			} else
				info(COPS_MISS);
		}
	}
	/* Fight over .. */
	if (caught) {
		struct good *g = good;
		if (shotat)
			info(COPS_TAKE_ALL);
		else
			info(COPS_TAKE_HALF);
		if (gun) { 
			info(COP_NO_GUN);
			gun = 0;
		}
		while(g->name) {
			space += g->owned;
			g->owned = 0;
			g++;
		}
		if (shotat) {
			cash = 0;
			location = 6;
		} else
			cash /= 2;
	}
}

void coat(void)
{
	printf("You are offered a trenchcoat wiht bigger pockets for $200.\n");
	if (cash < 20000) {
		printf("You can't afford it.\n");
		return;
	}
	if (!yesno("Buy it ? "))
		return;
	cash -= 20000;
	coatsize += 40;
	space += 40;
}

void gunsale(void)
{
	if (gun == 1)
		return;

	printf("You are offered a gun for $400");
	/* Q: 400 dollars or 4.00 ? */
	if (cash < 40000L) {
		printf(" but can't afford it.\n");
		return;
	}
	if (!yesno(". Buy it ? "))
		return;
	gun = 1;
	cash -= 40000;
}

money borrow_limit(void)
{
	money maxb = cash / 100 * SHARK_LEVERAGE;
	if (maxb < 5500)
		maxb = 5500;
	if (maxb > 20000000L)
		maxb = 20000000L;
	return maxb;
}

void loanshark(void)
{
	money n;
	char c;
	money maxb = borrow_limit();

	while(1) {
		if (debt)
			printf("You currently owe %s.\n", price(debt));
		if (maxb)
			printf("You can borrow up to %s.\n", price(maxb));

		printf("[%s] b)orrow, p)ay back, / to return: ", price(cash));
		c = getch();
		if (c == '/' || c == '\n')
			return;
		if (c == 'p') {
			if (!quantity_cash(&n, "pay back"))
				return;
			if (n > cash)
				error(NOT_ENOUGH_CASH);
			else if (n > debt)
				error(NOT_OWED);
			else {
				/* Borrowing creates money. paying it back
				   destroys money. Isn't economics fun */
				cash -= n;
				debt -= n;
			}
			continue;
		}
		if (c == 'b') {
			if (days == last_borrow) {
				error(NO_LEND_TODAY);
				continue;
			}
			if (maxb == 0) {
				error(NO_LEND_MORE);
				continue;
			}
			if (!quantity_cash(&n, "borrow"))
				return;
			if (n > maxb)
				error(NOT_LEND_THIS);
			else {
				/* Create money */
				cash += n;
				debt += n;
				last_borrow = days;
			}
			continue;
		}
	}
}

void banker(void)
{
	money n;
	char c;

	while(1) {
		printf("You have %s on deposit.\n", price(bank));
		printf("[%s] d)eposit, w)ithdraw, / to return: ", price(cash));
		c = getch();
		if (c == '/' || c == '\n')
			return;
		if (c == 'd') {
			if (!quantity_cash(&n, "deposit"))
				return;
			if (n > cash)
				error(NOT_ENOUGH_CASH);
			else {
				bank += n;
				cash -= n;
			}
			continue;
		}
		if (c == 'w') {
			if (!quantity_cash(&n, "widthdraw"))
				return;
			if (n > bank)
				error(NOT_ENOUGH_BANK);
			else {
				bank -= n;
				cash += n;
			}
			continue;
			
		}
	}
}

void day_cycle(void)
{
	debt += debt >> 3;
	bank += bank >> 4;
	days++;
	if (chance(13)
	    && (space != coatsize || gun))
		police();
	if (dead)
		return;
	if (chance(10))
		coat();
	if (!gun && chance(10))
		gunsale();
	price_goods(3);
	do_modifiers();
}

void move_place(void)
{
	unsigned i;
	char c;
	char *p;
	for (i = 0; i < 6; i++)
		printf("%c %s\n", locstr[i], locations[i]);
	printf("Take the subway where ? ");
	while (1) {
		c = getch();
		p = strchr(locstr, c);
		if (p) {
			location = p - locstr;
			day_cycle();
			if (dead)
				return;
		}
		if (c == '\n' || c == '/')
			return;
	}
}

money value_owned(void)
{
	struct good *g = good;
	money n = cash + bank - debt;
	while (g->name) {
		n += g->owned * g->price;
		g++;
	}
	return n;
}

/*
 *	The rest is the game specific mechanics
 */

void init_game(void)
{
	location = 0;
	cash = 2000;
	space = 100;
	coatsize = 100;
	bank = 0;
	debt = 5500;
}

void status(void)
{
	printf("\n\nDay %d, In %s.\n", days, locations[location]);
	if (location != 6)
		show_inventory(1);
}

int game_run(void)
{
	char c;

	while (1) {
		if (dead)
			return 0;
		status();
		if (location == 6) {
			if (days == 31)
				return 0;
			move_place();
			continue;
		}
		if (location == 0)
			printf("l)oan shark b(a)nk ");
		printf("b)uy s)ell i)nventory m)ove d)one Q)uit > ");
		c = getch();
		switch (c) {
		case 'b':
			buy();
			break;
		case 's':
			sell();
			break;
		case 'i':
			show_inventory(1);
			break;
		case 'l':
			if (location == 0)
				loanshark();
			break;
		case 'a':
			if (location == 0)
				banker();
			break;
		case 'm':
			if (days == 31)
				return 0;
			move_place();
			/* TODO move on last day lets you then buy sell but ends after */
			break;
		case 'd':
			if (days == 31)
				return 0;
			day_cycle();
			return 1;
		case 'Q':
			return 0;
		}
	}
}

int main(int argc, char *argv[])
{
	money score;
	srand(getpid() ^ time(NULL) ^ getuid());
	init_game();
	price_goods(3);
	while (game_run());
	score = value_owned();
	if (dead)
		printf("You ended the game in the city morgue.\n");
	else {
		printf("\nYou ended the game with %s\n", price(score));
		if (score < 0)
			printf("The loan shark's thugs broke your legs.\n");
		else if (score >= 100000000)
			printf("You retired a millionaire in the Carribbean.\n");
		else if (score >= 200000)
			printf("Congratulations! You didn't do half bad.\n");
		else
			printf("You didn't make much money. Better luck next time.\n");
	} 
	return 0;
}
