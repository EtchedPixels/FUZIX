#ifndef _LIBINTL_H
#define _LIBINTL_H

#define _(s)	(s)
#define N_(s)

extern char *bindtextdomain(const char *domainname, const char *dirname);

extern char *gettext(const char *msgid);
extern char *dgettext(const char *domainname, const char *msgid);
extern char *dcgettext(const char *domainnanem, const char *msgid, int category);

extern char *ngettext(const char *msgid, const char *msgid_plural, unsigned long n);
extern char *dngettext(const char *domainname, const char *msgid, const char *msgid_plural, unsigned long n);
extern char *dcngettext(const char *domainname, const char *msgid, const char *msgid_plural, unsigned long n, int category);

extern char *textdomain(const char *domainname);

#endif
