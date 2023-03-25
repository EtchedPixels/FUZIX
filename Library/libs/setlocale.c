#include <string.h>
#include <locale.h>
#include <limits.h>

char *setlocale(int category, const char *locale)
{
	if(strcmp(locale, "C") && strcmp(locale, "POSIX"))
		return NULL;
	return "C";
}

locale_t uselocale(locale_t newloc)
{
	return "C";
}

locale_t newlocale(int category_mask, const char *locale, locale_t base)
{
	if (strcmp(locale, "C") && strcmp(locale, "POSIX"))
		return NULL;
	return "C";
}

void freelocale(locale_t locale)
{
}

locale_t duplocale(locale_t locobj)
{
	return locobj;
}

/* Overwriting is forbidden by API call just not by the types */
static const struct lconv C_lc = {
	".",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	""
	"",
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
};

struct lconv *localeconv(void)
{
	return (struct lconv *)&C_lc;
}

/* FIXME: add nl_langinfo() */
