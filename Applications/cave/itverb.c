
/*  ITVERB.C  no mod for V 1.43  */

#include "advent.h"

/*
        Routines to process intransitive verbs
*/
void itverb(void)
{
	switch (verb) {
	case DROP:
	case SAY:
	case WAVE:
	case CALM:
	case RUB:
	case THROW:
	case FIND:
	case FEED:
	case BREAK:
	case WAKE:
		needobj();
		break;
	case TAKE:
		ivtake();
		break;
	case OPEN:
	case LOCK:
		ivopen();
		break;
	case NOTHING:
		rspeak(54);
		break;
	case ON:
	case OFF:
	case POUR:
		trverb();
		break;
	case WALK:
		actspk(verb);
		break;
	case KILL:
		ivkill();
		break;

#if 0
	case EAT:
		iveat();
		break;

#endif				/*  */
	case DRINK:
		ivdrink();
		break;
	case QUIT:
		ivquit();
		break;

#if 0
	case FILL:
		ivfill();
		break;

#endif				/*  */
	case BLAST:
		vblast();
		break;
	case SCORE:
		score();
		break;
	case FOO:
		ivfoo();
		break;
	case SUSPEND:
		game.saveflg = 1;
		break;
	case INVENTORY:
		inventory();
		break;
	case SAVE:
		saveadv();
		describe();
		descitem();
		break;
	case RESTORE:
		restore();
		describe();
		descitem();
		break;
	case BRIEF:
	case VERBOSE:
		brief_sw = (BRIEF == verb);
		rspeak(54);
		break;
	default:
		writes("This intransitive not implemented yet\n");
	}
	return;
}


/*
        CARRY, TAKE etc.
*/
void ivtake(void)
{
	auto short anobj, item;
	anobj = 0;
	for (item = 1; item < MAXOBJ; ++item) {
		if (game.place[item] == game.loc) {
			if (anobj != 0) {
				needobj();
				return;
			}
			anobj = item;
		}
	}
	if (anobj == 0 || (dcheck() && game.dflag >= 2)) {
		needobj();
		return;
	}
	object = anobj;
	vtake();
	return;
}


/*
        OPEN, LOCK, UNLOCK
*/
void ivopen(void)
{
	if (here(CLAM))
		object = CLAM;
	if (here(OYSTER))
		object = OYSTER;
	if (at(DOOR))
		object = DOOR;
	if (at(GRATE))
		object = GRATE;
	if (here(CHAIN)) {
		if (object != 0) {
			needobj();
			return;
		}
		object = CHAIN;
	}
	if (object == 0) {
		rspeak(28);
		return;
	}
	vopen();
	return;
}


/*
        ATTACK, KILL etc
*/
void ivkill(void)
{
	game.object1 = 0;
	if (dcheck() && game.dflag >= 2)
		object = DWARF;
	if (here(SNAKE))
		addobj(SNAKE);
	if (at(DRAGON) && game.prop[DRAGON] == 0)
		addobj(DRAGON);
	if (at(TROLL))
		addobj(TROLL);
	if (here(BEAR) && game.prop[BEAR] == 0)
		addobj(BEAR);
	if (game.object1 != 0) {
		needobj();
		return;
	}
	if (object != 0) {
		vkill();
		return;
	}
	if (here(BIRD) && verb != THROW)
		object = BIRD;
	if (here(CLAM) || here(OYSTER))
		addobj(CLAM);
	if (game.object1 != 0) {
		needobj();
		return;
	}
	vkill();
	return;
}


/*
        EAT
*/
void iveat(void)
{
	if (!here(FOOD))
		needobj();

	else {
		object = FOOD;
		veat();
	}
	return;
}


/*
        DRINK
*/
void ivdrink(void)
{
	if (liqloc(game.loc) != WATER && (liq() != WATER || !here(BOTTLE)))
		needobj();

	else {
		object = WATER;
		vdrink();
	}
	return;
}


/*
        QUIT
*/
void ivquit(void)
{
	if ((game.gaveup = yes(22, 54, 54)) != 0)
		normend();
	return;
}


/*
        FILL
*/
void ivfill(void)
{
	if (!here(BOTTLE))
		needobj();

	else {
		object = BOTTLE;
		vfill();
	}
	return;
}


/*
        Handle fee fie foe foo...
*/
void ivfoo(void)
{
	auto short k, msg;
	k = vocab(word1, 3000);
	msg = 42;
	if (game.foobar != 1 - k) {
		if (game.foobar != 0)
			msg = 151;
		rspeak(msg);
		return;
	}
	game.foobar = k;
	if (k != 4)
		return;
	game.foobar = 0;
	if (game.place[EGGS] == 92 || (toting(EGGS) && game.loc == 92)) {
		rspeak(msg);
		return;
	}
	if (game.place[EGGS] == 0 && game.place[TROLL] == 0 && game.prop[TROLL] == 0)
		game.prop[TROLL] = 1;
	if (here(EGGS))
		k = 1;

	else {
		if (game.loc == 92)
			k = 0;

		else
			k = 2;
	}
	move(EGGS, 92);
	pspeak(EGGS, k);
	return;
}


/*
        read etc...
*/
void ivread(void)
{
	if (here(MAGAZINE))
		object = MAGAZINE;
	if (here(TABLET))
		object = object * 100 + TABLET;
	if (here(MESSAGE))
		object = object * 100 + MESSAGE;
	if (object > 100 || object == 0 || dark()) {
		needobj();
		return;
	}
	vread();
	return;
}


/*
        INVENTORY
*/
void inventory(void)
{
	auto short msg;
	auto short i;
	msg = 98;
	for (i = 1; i <= MAXOBJ; ++i) {
		if (i == BEAR || !toting(i))
			continue;
		if (msg)
			rspeak(99);
		msg = 0;
		pspeak(i, -1);
	}
	if (toting(BEAR))
		msg = 141;
	if (msg)
		rspeak(msg);
	return;
}


/*
        ensure uniqueness as objects are searched
        out for an intransitive verb
*/
void addobj(short obj)
{
	if (game.object1 != 0)
		return;
	if (object != 0) {
		game.object1 = -1;
		return;
	}
	object = obj;
	return;
}
