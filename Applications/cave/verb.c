
    /* VERB.C  no mods for V 1.43 */

#include "advent.h"

/*
        Routine to process a transitive verb
*/
void trverb(void)
{
	switch (verb) {
	case CALM:
	case WALK:
	case QUIT:
	case SCORE:
	case FOO:
	case BRIEF:
	case SUSPEND:
	case HOURS:
	case LOG:
		actspk(verb);
		break;
	case TAKE:
		vtake();
		break;
	case DROP:
		vdrop();
		break;
	case OPEN:
	case LOCK:
		vopen();
		break;
	case SAY:
		vsay();
		break;
	case NOTHING:
		rspeak(54);
		break;
	case ON:
		von();
		break;
	case OFF:
		voff();
		break;
	case WAVE:
		vwave();
		break;
	case KILL:
		vkill();
		break;
	case POUR:
		vpour();
		break;
	case EAT:
		veat();
		break;
	case DRINK:
		vdrink();
		break;
	case RUB:
		if (object != LAMP)
			rspeak(76);

		else
			actspk(RUB);
		break;
	case THROW:
		vthrow();
		break;
	case FEED:
		vfeed();
		break;
	case FIND:
	case INVENTORY:
		vfind();
		break;
	case FILL:
		vfill();
		break;
	case READ:
		vread();
		break;
	case BLAST:
		vblast();
		break;
	case BREAK:
		vbreak();
		break;
	case WAKE:
		vwake();
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
	default:
		writes("This verb is not implemented yet.\n");
	}
	return;
}


/*
        CARRY TAKE etc.
*/
void vtake(void)
{
	auto short msg;
	auto short i;
	if (toting(object)) {
		actspk(verb);
		return;
	}

	/* special case objects and fixed objects */
	msg = 25;
	if (object == PLANT && prop[PLANT] <= 0)
		msg = 115;
	if (object == BEAR && prop[BEAR] == 1)
		msg = 169;
	if (object == CHAIN && prop[BEAR] != 0)
		msg = 170;
	if (fixed[object]) {
		rspeak(msg);
		return;
	}

	/* special case for liquids */
	if (object == WATER || object == OIL) {
		if (!here(BOTTLE) || liq() != object) {
			object = BOTTLE;
			if (toting(BOTTLE) && prop[BOTTLE] == 1) {
				vfill();
				return;
			}
			if (prop[BOTTLE] != 1)
				msg = 105;
			if (!toting(BOTTLE))
				msg = 104;
			rspeak(msg);
			return;
		}
		object = BOTTLE;
	}
	if (holding >= 7) {
		rspeak(92);
		return;
	}

	/* special case for bird. */
	if (object == BIRD && prop[BIRD] == 0) {
		if (toting(ROD)) {
			rspeak(26);
			return;
		}
		if (!toting(CAGE)) {
			rspeak(27);
			return;
		}
		prop[BIRD] = 1;
	}
	if ((object == BIRD || object == CAGE) && prop[BIRD] != 0)
		carry((BIRD + CAGE) - object, game.loc);
	carry(object, game.loc);

	/* handle liquid in bottle */
	i = liq();
	if (object == BOTTLE && i != 0)
		place[i] = -1;
	rspeak(54);
	return;
}


/*
        DROP etc.
*/
void vdrop(void)
{
	auto short i;

	/* check for dynamite */
	if (toting(ROD2) && object == ROD && !toting(ROD))
		object = ROD2;
	if (!toting(object)) {
		actspk(verb);
		return;
	}

	/* snake and bird */
	if (object == BIRD && here(SNAKE)) {
		rspeak(30);
		if (closed)
			dwarfend();
		dstroy(SNAKE);
		prop[SNAKE] = -1;
	}

	else {			/* coins and vending machine */

		if (object == COINS && here(VEND)) {
			dstroy(COINS);
			drop(BATTERIES, game.loc);
			pspeak(BATTERIES, 0);
			return;
		}

		else {		/* bird and dragon (ouch!!) */

			if (object == BIRD && at(DRAGON) && prop[DRAGON] == 0) {
				rspeak(154);
				dstroy(BIRD);
				prop[BIRD] = 0;
				if (place[SNAKE] != 0)
					++tally2;
				return;
			}
		}
	}

	/* Bear and troll */
	if (object == BEAR && at(TROLL)) {
		rspeak(163);
		move(TROLL, 0);
		move((TROLL + MAXOBJ), 0);
		move(TROLL2, 117);
		move((TROLL2 + MAXOBJ), 122);
		juggle(CHASM);
		prop[TROLL] = 2;
	}

	else {			/* vase */

		if (object == VASE) {
			if (game.loc == 96)
				rspeak(54);

			else {
				prop[VASE] = at(PILLOW) ? 0 : 2;
				pspeak(VASE, prop[VASE] + 1);
				if (prop[VASE] != 0)
					fixed[VASE] = -1;
			}
		}
	}

	/* handle liquid and bottle */
	i = liq();
	if (i == object)
		object = BOTTLE;
	if (object == BOTTLE && i != 0)
		place[i] = 0;

	/* handle bird and cage */
	if (object == CAGE && prop[BIRD] != 0)
		drop(BIRD, game.loc);
	if (object == BIRD)
		prop[BIRD] = 0;
	drop(object, game.loc);
	return;
}


/*
        LOCK, UNLOCK, OPEN, CLOSE etc.
*/
void vopen(void)
{
	auto short msg, oyclam;
	switch (object) {
	case CLAM:
	case OYSTER:
		oyclam = (object == OYSTER ? 1 : 0);
		if (verb == LOCK)
			msg = 61;

		else {
			if (!toting(TRIDENT))
				msg = 122 + oyclam;

			else {
				if (toting(object))
					msg = 120 + oyclam;

				else {
					msg = 124 + oyclam;
					dstroy(CLAM);
					drop(OYSTER, game.loc);
					drop(PEARL, 105);
				}
			}
		}
		break;
	case DOOR:
		msg = (prop[DOOR] == 1 ? 54 : 111);
		break;
	case CAGE:
		msg = 32;
		break;
	case KEYS:
		msg = 55;
		break;
	case CHAIN:
		if (!here(KEYS))
			msg = 31;

		else {
			if (verb == LOCK) {
				if (prop[CHAIN] != 0)
					msg = 34;

				else if (game.loc != 130)
					msg = 173;

				else {
					prop[CHAIN] = 2;
					if (toting(CHAIN))
						drop(CHAIN, game.loc);
					fixed[CHAIN] = -1;
					msg = 172;
				}
			}

			else {
				if (prop[BEAR] == 0)
					msg = 41;

				else {
					if (prop[CHAIN] == 0)
						msg = 37;

					else {
						prop[CHAIN] = 0;
						fixed[CHAIN] = 0;
						if (prop[BEAR] != 3)
							prop[BEAR] = 2;
						fixed[BEAR] = 2 - prop[BEAR];
						msg = 171;
					}
				}
			}
		}
		break;
	case GRATE:
		if (!here(KEYS))
			msg = 31;

		else {
			if (closing) {
				if (!panic) {
					clock2 = 15;
					++panic;
				}
				msg = 130;
			}

			else {
				msg = 34 + prop[GRATE];
				prop[GRATE] = (verb == LOCK ? 0 : 1);
				msg += 2 * prop[GRATE];
			}
		}
		break;
	default:
		msg = 33;
	}
	rspeak(msg);
	return;
}


/*
        SAY etc.
*/
void vsay(void)
{
	auto short wtype, wval;
	analyze(word1, &wtype, &wval);
	writes("Okay.\n");
	writes(wval == SAY ? word2 : word1);
	nl();
	return;
}


/*
        ON etc.
*/
void von(void)
{
	if (!here(LAMP))
		actspk(verb);

	else {
		if (limit < 0)
			rspeak(184);

		else {
			prop[LAMP] = 1;
			rspeak(39);
			if (wzdark) {
				wzdark = 0;
				describe();
				descitem();
			}
		}
	}
	return;
}


/*
        OFF etc.
*/
void voff(void)
{
	if (!here(LAMP))
		actspk(verb);

	else {
		prop[LAMP] = 0;
		rspeak(40);
	}
	return;
}


/*
        WAVE etc.
*/
void vwave(void)
{
	if (!toting(object) && (object != ROD || !toting(ROD2)))
		rspeak(29);

	else {
		if (object != ROD || !at(FISSURE) || !toting(object) || closing)
			actspk(verb);

		else {
			prop[FISSURE] = 1 - prop[FISSURE];
			pspeak(FISSURE, 2 - prop[FISSURE]);
		}
	}
}


/*
        ATTACK, KILL etc.
*/
void vkill(void)
{
	auto short msg;
	auto short i;
	switch (object) {
	case BIRD:
		if (closed)
			msg = 137;

		else {
			dstroy(BIRD);
			prop[BIRD] = 0;
			if (place[SNAKE] == 19)
				++tally2;
			msg = 45;
		}
		break;
	case 0:
		msg = 44;
		break;
	case CLAM:
	case OYSTER:
		msg = 150;
		break;
	case SNAKE:
		msg = 46;
		break;
	case DWARF:
		if (closed)
			dwarfend();
		msg = 49;
		break;
	case TROLL:
		msg = 157;
		break;
	case BEAR:
		msg = 165 + (prop[BEAR] + 1) / 2;
		break;
	case DRAGON:
		if (prop[DRAGON] != 0) {
			msg = 167;
			break;
		}
		if (!yes(49, 0, 0))
			break;
		pspeak(DRAGON, 1);
		prop[DRAGON] = 2;
		prop[RUG] = 0;
		move((DRAGON + MAXOBJ), -1);
		move((RUG + MAXOBJ), 0);
		move(DRAGON, 120);
		move(RUG, 120);
		for (i = 1; i < MAXOBJ; ++i)
			if (place[i] == 119 || place[i] == 121)
				move(i, 120);
		game.newloc = 120;
		return;
	default:
		actspk(verb);
		return;
	}
	rspeak(msg);
}


/*
        POUR
*/
void vpour(void)
{
	if (object == BOTTLE || object == 0)
		object = liq();
	if (object == 0) {
		needobj();
		return;
	}
	if (!toting(object)) {
		actspk(verb);
		return;
	}
	if (object != OIL && object != WATER) {
		rspeak(78);
		return;
	}
	prop[BOTTLE] = 1;
	place[object] = 0;
	if (at(PLANT)) {
		if (object != WATER)
			rspeak(112);

		else {
			pspeak(PLANT, prop[PLANT] + 1);
			prop[PLANT] = (prop[PLANT] + 2) % 6;
			prop[PLANT2] = prop[PLANT] / 2;
			describe();
		}
	}

	else {
		if (at(DOOR)) {
			prop[DOOR] = (object == OIL ? 1 : 0);
			rspeak(113 + prop[DOOR]);
		}

		else
			rspeak(77);
	}
	return;
}


/*
        EAT
*/
void veat(void)
{
	auto short msg;
	switch (object) {
	case FOOD:
		dstroy(FOOD);
		msg = 72;
		break;
	case BIRD:
	case SNAKE:
	case CLAM:
	case OYSTER:
	case DWARF:
	case DRAGON:
	case TROLL:
	case BEAR:
		msg = 71;
		break;
	default:
		actspk(verb);
		return;
	}
	rspeak(msg);
	return;
}


/*
        DRINK
*/
void vdrink(void)
{
	if (object != WATER)
		rspeak(110);

	else {
		if (liq() != WATER || !here(BOTTLE))
			actspk(verb);

		else {
			prop[BOTTLE] = 1;
			place[WATER] = 0;
			rspeak(74);
		}
	}
	return;
}


/*
        THROW etc.
*/
void vthrow(void)
{
	auto short msg;
	auto short i;
	if (toting(ROD2) && object == ROD && !toting(ROD))
		object = ROD2;
	if (!toting(object)) {
		actspk(verb);
		return;
	}

	/* treasure to troll */
	if (at(TROLL) && object >= 50 && object < MAXOBJ) {
		rspeak(159);
		drop(object, 0);
		move(TROLL, 0);
		move((TROLL + MAXOBJ), 0);
		drop(TROLL2, 117);
		drop((TROLL2 + MAXOBJ), 122);
		juggle(CHASM);
		return;
	}

	/* feed the bears... */
	if (object == FOOD && here(BEAR)) {
		object = BEAR;
		vfeed();
		return;
	}

	/* if not axe, same as drop... */
	if (object != AXE) {
		vdrop();
		return;
	}

	/* AXE is THROWN */
	/* at a dwarf... */
	if (i = dcheck()) {
		msg = 48;
		if (pct(33)) {
			dseen[i] = dloc[i] = 0;
			msg = 47;
			++dkill;
			if (dkill == 1)
				msg = 149;
		}
	}

	else {			/* at a dragon... */

		if (at(DRAGON) && prop[DRAGON] == 0)
			msg = 152;

		else {		/* at the troll... */

			if (at(TROLL))
				msg = 158;

			else {	/* at the bear... */

				if (here(BEAR) && prop[BEAR] == 0) {
					rspeak(164);
					drop(AXE, game.loc);
					fixed[AXE] = -1;
					prop[AXE] = 1;
					juggle(BEAR);
					return;
				}

				else {	/* otherwise it is an attack */

					verb = KILL;
					object = 0;
					itverb();
					return;
				}
			}
		}
	}

	/* handle the left over axe... */
	rspeak(msg);
	drop(AXE, game.loc);
	describe();
	return;
}


/*
        INVENTORY, FIND etc.
*/
void vfind(void)
{
	auto short msg;
	if (toting(object))
		msg = 24;

	else {
		if (closed)
			msg = 138;

		else {
			if (dcheck() && dflag >= 2 && object == DWARF)
				msg = 94;

			else {
				if (at(object) || (liq() == object && here(BOTTLE)) || object == liqloc(game.loc))
					msg = 94;

				else {
					actspk(verb);
					return;
				}
			}
		}
	}
	rspeak(msg);
	return;
}


/*
        FILL
*/
void vfill(void)
{
	auto short msg;
	auto short i;
	switch (object) {
	case BOTTLE:
		if (liq() != 0)
			msg = 105;

		else {
			if (liqloc(game.loc) == 0)
				msg = 106;

			else {
				prop[BOTTLE] = cond[game.loc] & WATOIL;
				i = liq();
				if (toting(BOTTLE))
					place[i] = -1;
				msg = (i == OIL ? 108 : 107);
			}
		}
		break;
	case VASE:
		if (liqloc(game.loc) == 0) {
			msg = 144;
			break;
		}
		if (!toting(VASE)) {
			msg = 29;
			break;
		}
		rspeak(145);
		vdrop();
		return;
	default:
		msg = 29;
	}
	rspeak(msg);
}


/*
        FEED
*/
void vfeed(void)
{
	auto short msg;
	switch (object) {
	case BIRD:
		msg = 100;
		break;
	case DWARF:
		if (!here(FOOD)) {
			actspk(verb);
			return;
		}
		++dflag;
		msg = 103;
		break;
	case BEAR:
		if (!here(FOOD)) {
			if (prop[BEAR] == 0)
				msg = 102;

			else {
				if (prop[BEAR] == 3)
					msg = 110;

				else {
					actspk(verb);
					return;
				}
			}
			break;
		}
		dstroy(FOOD);
		prop[BEAR] = 1;
		fixed[AXE] = 0;
		prop[AXE] = 0;
		msg = 168;
		break;
	case DRAGON:
		msg = (prop[DRAGON] != 0 ? 110 : 102);
		break;
	case TROLL:
		msg = 182;
		break;
	case SNAKE:
		if (closed || !here(BIRD)) {
			msg = 102;
			break;
		}
		msg = 101;
		dstroy(BIRD);
		prop[BIRD] = 0;
		++tally2;
		break;
	default:
		msg = 14;
	}
	rspeak(msg);
}


/*
        READ etc.
*/
void vread(void)
{
	auto short msg;
	msg = 0;
	if (dark()) {
		writes("I see no ");
		writes(probj(object));
		writes(" here.\n");
		return;
	}
	switch (object) {
	case MAGAZINE:
		msg = 190;
		break;
	case TABLET:
		msg = 196;
		break;
	case MESSAGE:
		msg = 191;
		break;
	case OYSTER:
		if (!toting(OYSTER) || !closed)
			break;
		yes(192, 193, 54);
		return;
	default:
		;
	}
	if (msg)
		rspeak(msg);

	else
		actspk(verb);
	return;
}


/*
        BLAST etc.
*/
void vblast(void)
{
	if (prop[ROD2] < 0 || !closed)
		actspk(verb);

	else {
		bonus = 133;
		if (game.loc == 115)
			bonus = 134;
		if (here(ROD2))
			bonus = 135;
		rspeak(bonus);
		normend();
	}
	return;
}


/*
        BREAK etc.
*/
void vbreak(void)
{
	auto short msg;
	if (object == MIRROR) {
		msg = 148;
		if (closed) {
			rspeak(197);
			dwarfend();
		}
	}

	else {
		if (object == VASE && prop[VASE] == 0) {
			msg = 198;
			if (toting(VASE))
				drop(VASE, game.loc);
			prop[VASE] = 2;
			fixed[VASE] = -1;
		}

		else {
			actspk(verb);
			return;
		}
	}
	rspeak(msg);
	return;
}


/*
        WAKE etc.
*/
void vwake(void)
{
	if (object != DWARF || !closed)
		actspk(verb);

	else {
		rspeak(199);
		dwarfend();
	}
	return;
}


/*
        Routine to speak default verb message
*/
void actspk(short verb)
{
	auto short i;
	if (verb < 1 || verb > 31)
		bug(39);
	i = actmsg[verb];
	if (i)
		rspeak(i);
	return;
}


/*
        Routine to indicate no reasonable
        object for verb found.  Used mostly by
        intransitive verbs.
*/
void needobj(void)
{
	auto short wtype, wval;
	analyze(word1, &wtype, &wval);
	writes(wtype == 2 ? word1 : word2);
	writes(" what?\n");
	return;
}
