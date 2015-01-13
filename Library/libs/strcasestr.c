/* Another BSDism */

#include <string.h>
#include <ctype.h>

char *strcasestr(const char *needle, const char *haystack)
{
  size_t s = strlen(needle);
  char c = tolower(needle[0]);

  while(*haystack) {
    /* check the lead byte here for speed */
    if (tolower(*haystack) == c) {
      if (strncasecmp(needle, haystack, s) == 0)
        return (char *)haystack;
    }
    haystack++;
  }
  return NULL;
}
