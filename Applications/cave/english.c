
/*  ENGLISH.C   no mod for V 1.43
  
                Changed all calls of rand() to irand() for Eco-C88 (BW)
*/

#include "advent.h"

/*
        Analyze a two word sentence
*/
uint8_t english(void)
{
	auto const char *msg;
	auto short type1, type2, val1, val2;
	verb = object = motion = 0;
	type2 = val2 = -1;
	type1 = val1 = -1;
	msg = "bad grammar...";
	getin();
	if (!analyze(word1, &type1, &val1))
		return (FALSE);
	if (type1 == 2 && val1 == SAY) {
		verb = SAY;
		object = 1;
		return (TRUE);
	}
	if (strcmp(word2, "")) {
		if (!analyze(word2, &type2, &val2))
			return (FALSE);
	}

	/* check his grammar */
	if (type1 == 3) {
		rspeak(val1);
		return (FALSE);
	}

	else {
		if (type2 == 3) {
			rspeak(val2);
			return (FALSE);
		}

		else {
			if (type1 == 0) {
				if (type2 == 0) {
					writes(msg);
					nl();
					return (FALSE);
				}

				else
					motion = val1;
			}

			else {
				if (type2 == 0)
					motion = val2;

				else {
					if (type1 == 1) {
						object = val1;
						if (type2 == 2)
							verb = val2;
						if (type2 == 1) {
							writes(msg);
							nl();
							return (FALSE);
						}
					}

					else {
						if (type1 == 2) {
							verb = val1;
							if (type2 == 1)
								object = val2;
							if (type2 == 2) {
								writes(msg);
								nl();
								return (FALSE);
							}
						}

						else
							bug(36);
					}
				}
			}
		}
	}
	return (TRUE);
}


/*
                Routine to analyze a word.
*/
uint8_t analyze(char *word, short *type, short *value)
{
	auto short wordval, msg;

	/* make sure I understand */
	if ((wordval = vocab(word, 0)) == -1) {
		switch (rand() % 3) {
		case 0:
			msg = 60;
			break;
		case 1:
			msg = 61;
			break;
		default:
			msg = 13;
		}
		rspeak(msg);
		return (FALSE);
	}

	/* FIXME use powers of 2 ! */
	*type = wordval / 1000;
	*value = wordval % 1000;
	return (TRUE);
}


/*
        routine to get two words of input
        and convert them to lower case
*/
void getin(void)
{
	auto char *bptr;
	auto char linebuff[65];
	writes(">");
	word1[0] = word2[0] = '\0';
	bptr = linebuff;
	getinp(linebuff, 65);
	skipspc(&bptr);
	getword(&bptr, word1);
	if (!*bptr)
		return;
	while (!isspace(*bptr)) {
		if (!*bptr++)
			return;
	}
	skipspc(&bptr);
	getword(&bptr, word2);
	return;
}


/*
        Routine to extract one word from a buffer
*/
void getword(char **buff, char *word)
{
	register short i;
	for (i = 0; i < WORDSIZE; ++i) {
		if (!**buff || isspace(**buff)) {
			*word = NUL;
			return;
		}
		*word++ = (char) tolower(**buff);
		(*buff)++;
	} *--word = NUL;
	return;
}


/*
        Routine to skip spaces
*/
void skipspc(char **buff)
{
	while (isspace(**buff))
		++(*buff);
	return;
}
