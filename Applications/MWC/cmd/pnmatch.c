
/* 
 * pnmatch(string, pattern, unanchored)
 * returns 1 if pattern matches in string.
 * pattern:
 *	[c1c2...cn-cm]	class of characters.
 *	?		any character.
 *	*		any # of any character.
 *	^		beginning of string (if unanchored)
 *	$		end of string (if unanchored)
 * unanch:
 *	0		normal (anchored) pattern.
 *	1		unanchored (^$ also metacharacters)
 *	>1		end unanchored.
 * >1 is used internally but should not be used by the user.
 */
pnmatch(s, p, unanch)
register char *s, *p;
{
	register c1;
	int c2;

	if (unanch == 1) {
		while (*s)
			if (pnmatch(s++, p, ++unanch))
				return (1);
		return (0);
	}
	while (c2 = *p++) {
		c1 = *s++;
		switch(c2) {
		case '^':
			if (unanch == 2) {
				s--;
				continue;
			} else if (unanch == 0)
				break;
			else
				return (0);

		case '$':
			if (unanch)
				return (c1 == '\0');
			break;

		case '[':
			for (;;) {
				c2 = *p++;
				if (c2=='\0' || c2==']')
					return (0);
				if (c2 == '\\' && *p == '-') 
					c2 = *p++;
				if (c2 == c1)
					break;
				if (*p == '-')
					if (c1<=*++p && c1>=c2)
						break;
			}
			while (*p && *p++!=']')
				;

		case '?':
			if (c1)
				continue;
			return(0);

		case '*':
			if (!*p)
				return(1);
			s--;
			do {
				if (pnmatch(s, p, unanch))
					return (1);
			} while(*s++ != '\0');
			return(0);

		case '\\':
			if ((c2 = *p++) == '\0')
				return (0);
		}
		if (c1 != c2)
			return (0);
	}
	return(unanch ? 1 : !*s);
}
