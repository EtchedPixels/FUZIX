#ifndef _LIBINTL_H
#define _LIBINTL_H

#define _(s)	(s)
#define N_(s)	(s)

extern char *bindtextdomain(const char *__domainname, const char *__dirname);

extern char *gettext(const char *__msgid);
extern char *dgettext(const char *__domainname, const char *__msgid);
extern char *dcgettext(const char *domainnanem, const char *__msgid, int __category);

extern char *ngettext(const char *__msgid, const char *__msgid_plural, unsigned long __n);
extern char *dngettext(const char *__domainname, const char *__msgid, const char *__msgid_plural, unsigned long __n);
extern char *dcngettext(const char *__domainname, const char *__msgid, const char *__msgid_plural, unsigned long __n, int __category);

extern char *textdomain(const char *__domainname);

#endif
