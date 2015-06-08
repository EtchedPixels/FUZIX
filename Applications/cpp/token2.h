/* C code produced by gperf version 2.7.1 (19981006 egcs) */
/* Command-line: gperf -aptTc -k1,3 -N is_ckey -H hash2 token2.tok  */

#define TOTAL_KEYWORDS 34
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 8
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 69
/* maximum key range = 68, duplicates = 0 */

#ifdef __GNUC__
__inline
#endif
static unsigned int
hash2 (str, len)
     register const char *str;
     register unsigned int len;
{
  static unsigned char asso_values[] =
    {
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
       5, 70, 70, 70, 70, 70,  0, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70,  0, 70,  5,  5, 10,
      10, 20, 20, 25, 70,  0, 70, 70, 50, 70,
       0, 15,  0, 70, 15,  0, 40, 20,  0,  0,
      70, 70, 10, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
      70, 70, 70, 70, 70, 70
    };
  register int hval = len;

  switch (hval)
    {
      default:
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

#ifdef __GNUC__
__inline
#endif
struct token_trans *
is_ckey (str, len)
     register const char *str;
     register unsigned int len;
{
  static struct token_trans wordlist[] =
    {
      {""}, {""},
      {"if",		TK_IF},
      {""},
      {"void",		TK_VOID},
      {"while",		TK_WHILE},
      {"switch",		TK_SWITCH},
      {""},
      {"__LINE__",	TK_LINE},
      {""}, {""},
      {"static",		TK_STATIC},
      {"do",		TK_DO},
      {"__FILE__",	TK_FILE},
      {"case",		TK_CASE},
      {"const",		TK_CONST},
      {"sizeof",		TK_SIZEOF},
      {""},
      {"continue",	TK_CONTINUE},
      {"char",		TK_CHAR},
      {"short",		TK_SHORT},
      {"struct",		TK_STRUCT},
      {""}, {""},
      {"else",		TK_ELSE},
      {"union",		TK_UNION},
      {""}, {""},
      {"unsigned",	TK_UNSIGNED},
      {""},
      {"break",		TK_BREAK},
      {"signed",		TK_SIGNED},
      {""}, {""}, {""}, {""},
      {"double",		TK_DOUBLE},
      {"default",	TK_DEFAULT},
      {"for",		TK_FOR},
      {""},
      {"float",		TK_FLOAT},
      {""}, {""},
      {"int",		TK_INT},
      {"enum",		TK_ENUM},
      {""}, {""},
      {"typedef",	TK_TYPEDEF},
      {"register",	TK_REGISTER},
      {"auto",		TK_AUTO},
      {""}, {""}, {""}, {""},
      {"long",		TK_LONG},
      {""}, {""}, {""},
      {"volatile",	TK_VOLATILE},
      {""}, {""},
      {"return",		TK_RETURN},
      {""}, {""}, {""}, {""},
      {"extern",		TK_EXTERN},
      {""}, {""},
      {"goto",		TK_GOTO}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash2 (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strncmp (str + 1, s + 1, len - 1))
            return &wordlist[key];
        }
    }
  return 0;
}
