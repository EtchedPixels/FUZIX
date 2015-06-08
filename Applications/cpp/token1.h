/* C code produced by gperf version 2.7.1 (19981006 egcs) */
/* Command-line: gperf -aptTc -N is_ctok -H hash1 token1.tok  */

#define TOTAL_KEYWORDS 23
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 3
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 63
/* maximum key range = 62, duplicates = 0 */

#ifdef __GNUC__
__inline
#endif
static unsigned int
hash1 (str, len)
     register const char *str;
     register unsigned int len;
{
  static unsigned char asso_values[] =
    {
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64,  1, 64, 64, 64,  3, 25, 64,
      64, 64, 13, 18, 64,  8, 30, 15, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
       5,  0, 20, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 30, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 23, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64
    };
  return len + asso_values[(unsigned char)str[len - 1]] + asso_values[(unsigned char)str[0]];
}

#ifdef __GNUC__
__inline
#endif
struct token_trans *
is_ctok (str, len)
     register const char *str;
     register unsigned int len;
{
  static struct token_trans wordlist[] =
    {
      {""}, {""},
      {"==",	TK_EQ_OP},
      {"!=",	TK_NE_OP},
      {""},
      {"%=",	TK_MOD_ASSIGN},
      {""},
      {"<=",	TK_LE_OP},
      {"<<=",	TK_LEFT_ASSIGN},
      {""},
      {"-=",	TK_SUB_ASSIGN},
      {""},
      {"<<",	TK_LEFT_OP},
      {""}, {""},
      {"*=",	TK_MUL_ASSIGN},
      {""},
      {"/=",	TK_DIV_ASSIGN},
      {"--",	TK_DEC_OP},
      {""},
      {"+=",	TK_ADD_ASSIGN},
      {""},
      {">=",	TK_GE_OP},
      {">>=",	TK_RIGHT_ASSIGN},
      {""},
      {"|=",	TK_OR_ASSIGN},
      {""},
      {"&=",	TK_AND_ASSIGN},
      {""}, {""},
      {"->",	TK_PTR_OP},
      {""},
      {"^=",	TK_XOR_ASSIGN},
      {""}, {""}, {""}, {""}, {""},
      {"++",	TK_INC_OP},
      {""}, {""}, {""},
      {">>",	TK_RIGHT_OP},
      {""}, {""}, {""}, {""}, {""},
      {"||",	TK_OR_OP},
      {""}, {""}, {""},
      {"&&",	TK_AND_OP},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {"..",	TK_WORD},
      {"...",	TK_ELLIPSIS}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash1 (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strncmp (str + 1, s + 1, len - 1))
            return &wordlist[key];
        }
    }
  return 0;
}
