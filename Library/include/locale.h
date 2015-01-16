#ifndef _LOCALE_H
#define _LOCALE_H

typedef void * locale_t;

struct lconv {
	char *decimal_point;
	char *thousands_sep;
	char *grouping;
	char *int_curr_symbol;
	char *currency_symbol;
	char *mon_decimal_pont;
	char *mon_thousands_sep;
	char *positive_sign;
	char *negative_sign;
	char frac_digits;
	char p_cs_precedes;
	char p_sep_by_space;
	char p_sign_posn;
	char n_sign_posn;
	/* ISO C99 added */
	char int_p_cs_precedes;
	char int_p_sep_by_space;
	char int_n_cs_precedes;
	char int_n_sep_by_space;
	char int_p_sign_posn;
	char int_n_sign_posn;
};

extern char *setlocale(int, const char *);
extern struct lconv *localeconv(void);

#define LC_ALL			0
#define LC_ADDRESS		1
#define LC_COLLATE		2
#define LC_CTYPE		3
#define LC_IDENTIFICATION	4
#define LC_MEASUREMENT		5
#define LC_MESSAGES		6
#define LC_MONETARY		7
#define LC_NAME			8
#define LC_NUMERIC		9
#define LC_PAPER		10
#define LC_TELEPHONE		11
#define LC_TIME			12

#define LC_GLOBAL_LOCALE	((void *)1)

#define tolower_l(a,b)	tolower(a)
#define toupper_l(a,b)	toupper(a)

#define isascii_l(a,b)	isascii(a)
#define isalnum_l(a,b)	isalnum(a)
#define isalpha_l(a,b)	isalpha(a)
#define isblank_l(a,b)	isblank(a)
#define iscntrl_l(a,b)	iscntrl(a)
#define isdigit_l(a,b)	isdigit(a)
#define isgraph_l(a,b)	isgraph(a)
#define islower_l(a,b)	islower(a)
#define isprint_l(a,b)	isprint(a)
#define ispunct_l(a,b)	ispunct(a)
#define isspace_l(a,b)	isspace(a)
#define isupper_l(a,b)	isupper(a)
#define isxdigit_l(a,b)	isxdigit(a)

#endif
