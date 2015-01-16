#include <libintl.h>
#include <stdlib.h>
#include <string.h>

static char *_domain;

char *textdomain(const char *domainname)
{
  if (domainname) {
    if (_domain)
      free(_domain);
    _domain = strdup(domainname);
  }
  return _domain;
}

char *bindtextdomain(const char *domainname, const char *dirname)
{
  return textdomain(domainname);
}

char *gettext(const char *msgid)
{
  return (char *)msgid;
}

char *dgettext(const char *domainname, const char *msgid)
{
  return (char *)msgid;
}

char *dcgettext(const char *domainname, const char *msgid, int category)
{
  return (char *)msgid;
}

char *ngettext(const char *msgid, const char *msgid_plural, unsigned long n)
{
  return n == 1 ? (char *)msgid : (char *)msgid_plural;
}

char *dngettext(const char *domain, const char *msgid, const char *msgid_plural, unsigned long n)
{
  return n == 1 ? (char *)msgid : (char *)msgid_plural;
}

char *dcngettext(const char *domain, const char *msgid, const char *msgid_plural, unsigned long n, int category)
{
  return n == 1 ? (char *)msgid : (char *)msgid_plural;
}
