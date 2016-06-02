/*
 * rstring.c - generic string-handling functions
 * as of version 1.49, this contains the contents of the `parse' library
 */

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "rstring.h"

/*
 * Name    : strupr
 * Descrip : Changes s to all uppercase
 * Input   : s = pointer to a string to uppercase
 * Output  : returns a pointer to s
 * Notes   : none
 * Example : strupr( "proper-noun" ) - returns "PROPER-NOUN"
 */

char *strupr( char *s )
{
  int i;

  for(i=0; i<strlen(s); i++ ) {
    s[i] = toupper(s[i]);
  }
  
  return s;
}

/*
 * Name    : strset
 * Descrip : Sets all characters in s to ch
 * Input   : s = pointer to a string to set
 *           ch = character to set all positions in s to 
 * Output  : returns a pointer to s
 * Notes   : None
 * Example : char cstring[10];
             strset( cstring, 'C' ) - returns "CCCCCCCCC"
 */

char *strset( char *s, int ch )
{
  unsigned char c;
  
  for(c=0; s[c] != 0; c++)
    s[c] = ch;
  
  return s;
}

// make a copy of a string
char *new_string( char *src ) 
{
  char *str;

  str = (char *)malloc( strlen(src)+1 );
  strncpy( str, src, strlen(src) );

  // prevent junk from being tacked on the end
  if( strlen(str) > strlen(src) ) {
    str[strlen(src)] = 0;
  }
  
  return str;
}

// make a new string from src, starting at char index and going for count chars
char *new_string_from( char *src, int index, int count )
{
  char *str;

  str = (char *)malloc( count + 1 );
  memset( str, 0, count+1 );
  strncpy( str, src+index, count );

  //  str[strlen(str)+1] = 0;
  //  str[count+1] = 0;

  return str;
}

/*
 * Name: firstnot - Finds first instance of a character which is not the
 *                     one specified
 * Input: d - Pointer to string to search through
 *        e - Char to search against
 *        first - Position in d to start the search at
 * Output: Returns the position of the first character which is not e.
 *         Returns -1 if there was an error
 * Example: firstnot( "ggggXgggX", "g", 0 ) returns 4
 */

int firstnot( const char *d, const char e, int first )
{
  char k;
  
  for(k=first; k<strlen(d); k++) {
    if( d[k] != e )
      return k;
  }
  return -1;
}

/*
 * Name: neither -  Finds the first character after the two specified
 * Input: d - Pointer to the string to search through
 *        e - The first char to search against
 *        f - The second char to search against
 *        first - The location in d to start searching at
 * Output: Returns the location of the first char which is not e or f.
 *         Returns -1 on errors
 * Notes: This is just like firstnot() except it takes to chars as args.
 * Example: neither( "ggggXgggXzgg", 'g', 'X', 0 ) returns 9.
 */

int neither( const char *d, const char e, const char f, char first )
{
  char k;
  
  for(k=first; k<strlen(d); k++) {
    if( (d[k] != e) && (d[k] != f) )
      return k;
  }
  return -1;
}

/*
 * Name: find - Search for any chars in the string e in string d
 * Input: d - pointer to a string to search through
 *        e - pointer to a list of chars to search for
 *        first - location in d to start search at
 * Output: Returns the location of the first occurence of a char in e in d.
 *         Returns -1 on errors
 * Example: find( "xrcedfg", "dg", 0 ) returns 4.
 */

int find( const char *d, const char *e, int first )
{
  int k, k2;
  
  for(k=first; k<strlen(d); k++) {
    for(k2=0; k2<strlen(e); k2++) {
      if( d[k] == e[k2] )
	return k;
    }
  }
  return -1;
}

/*
 * Name    : scopy
 * Descrip : Like the Pascal Copy function, scopy copies a portion of a
 *           string from src to dest, starting at index, and going for
 *           count characters.
 * Input   : dest - pointer to a string to recieve the copied portion
 *           src - pointer to a string to use as input
 *           index - character to start copying at
 *           count - number of characters to copy
 * Output  : dest contains the slice of src[index..index+count]
 * Notes   : None
 * Example : scopy( last, "Charlie Brown", 8, 5 ) - puts "Brown" in last.
 * History : the parse() function was originally coded in Pascal. To support
 *           the translation, this function was implemented, but now its proved
 *           useful on its own.
 */

char *scopy( char *dest, char *src, int index, int count )
{
  int k;
  
  for(k=0;k<=count;k++) {
    dest[k] = src[k+index];
  }
  dest[k] = '\0';
  return dest;
}

/*
 * Name: numstr - Returns number of substrings in a string
 * Input : d - a pointer to the string to search through
 * Output: returns number of substrings in string d
 * Example: numstr( "bob and    Figment  are THE  Bombz  " ) returns 6
 */
int numstr( const char *d )
{

  int k2, k3;
  int cnt = 0;

  k3 = -1;

  /* new version */  
  do {
    k2 = wspace( d, k3+1 ); /* find start of substring */
    if( k2 == -1 ) /* if there is no start, we return */
      return cnt;
    k3 = find( d, " \t", k2 ); /* find end of substring */
    if( k3 == -1 )
      return ++cnt;
    ++cnt;   /* increase counter if there is a start & a finish */
  } while( k3 != -1 );
  return -1;
}

/*
 * Name: parse - Returns specified substrings seperated by whitespace
 * Input: d - a pointer to the string to parse
 *		  i - the number of the substring you want
 *		  c - a pointer to the string where we place the substring
 * Output: Returns nonzero on errors and zero when there are no errors
 * Example: parse( " bob ate  cheese", 3, buffer ) places "cheese" in buffer
 */

char *parse( const char *src, int num )
{
  char *srccopy;
  char *ret = NULL;
  int currws, nextws;
  int i;
  
  srccopy = (char *)malloc(strlen(src)+1);
  strcpy( srccopy, src );
  
  total_trim( srccopy );
  
  if( !strcmp(srccopy, "") ) {
    return NULL;
  }
  
  currws = find( srccopy, " \t", 1 );

  if( currws == -1 ) { // no spaces found, so only one string available
    return srccopy;
  } 

  if( num == 1 ) {
    ret = new_string_from( srccopy, 0, currws );
    free( srccopy );
    return ret;
  }
  
  /*
    01234567890123456789
    "hello said the fig"
  */          
  
  for( i=1; i<num; i++ ) {
    nextws = find( srccopy, " \t", currws+1 );
    if( i+1 != num ) {
      currws = nextws;
    }
  }

  if( nextws == -1 ) {  // last bit in src, no whitespace after
    ret = new_string_from( srccopy, currws+1, strlen(srccopy)-currws-1 );
  } else {
    ret = new_string_from( srccopy, currws+1, nextws-currws-1 );
  }

  free( srccopy );
  return ret;
}

/*
 * Name    : rtrim
 * Descrip : Removes all trailing whitespace from a string
 * Input   : str = pointer to a string to strip
 * Output  : Returns a pointer to str
 * Notes   : None.
 * Example : rtrim( "Bob was busy   " ) - returns "Bob was busy"
 */

char *rtrim( char *str )
{

  int i = strlen( str ) - 1;
  
  while( (isspace(str[i])) && (i>=0) )
    str[i--] = '\0';
  
  return str;
}

/*
 * Name    : ltrim
 * Descrip : Removes all leading whitespace from a string
 * Input   : str = pointer to a string to strip
 * Output  : Retruns a pointer to str
 * Notes   : None
 * Example : ltrim( "  Woof! " ) - returns "Woof! "
 */

char *ltrim( char *str )
{                                  
  int i;
  int spc = ws( str );

  if( spc == -1 ) {  // blank line
    return "";
  }
  
  for( i=0; i<strlen(str); i++ ) {
    str[i] = str[i+spc];
  }
  str[i] = 0;
  
  return str;
}

/*
 * Name    : rws
 * Descrip : Reverse WhiteSpace: Finds the last charater in a string which
 *           is not a space or a tab
 * Input   : str = pointer to a string to search through
 * Output  : Returns the position of the last non-whitespace character
 * Notes   : Just like, ws(), but backwards
 * Example : rws( "Hey, you!  " ) - returns 8
 */

int rws( const char *str)
{
  int k;
  
  for(k=strlen(str);k>-1;k--) {
    if( (str[k] != ' ') && (str[k] != '\t') )
      return k;
  }
  return -1;
}


/*
 * Name    : ws
 * Descrip : WhiteSpace: finds the first character which isn't a tab or space
 * Input   : str = pointer to string to use as input
 * Output  : Returns position of fitsrt non-whitespace character
 * Notes   : none
 * Example : ws( "   Howdy, world!" ) - returns 3
 */

int ws( const char *str)
{
  int k;

  for(k=0;k<strlen(str);k++) {
    if( (str[k] != ' ') && (str[k] != '\t') )
      return k;
  }
  return -1;
}


int wspace( const char *str, int first )
{
  return neither( str, ' ', '\t', first );
}


char *trim( char *str ) 
{
  if( !strcmp(str, "") ) {
    return "";
  }

  ltrim( str );
  rtrim( str );
  
  return str;
}



int findchar( const char *str, char c )
{
  int i;
 
  for(i=0; i<strlen(str); i++) {
    if( str[i] == c )
      return i;
  }
  return -1;
}



/*
**  total_trim() - Remove leading, trailing, & excess embedded spaces
**  from the Snippets collection (TRIM.C)
**  public domain by Bob Stout & Michael Dehlwes 
*/
char *total_trim( char *str )
{
  char *ibuf, *obuf;
 
  if( !strcmp(str, "") ) {
    return str;
  }

  if( str ) {
    for( ibuf = obuf = str; *ibuf; ){
      while (*ibuf && (isspace (*ibuf))) {
	ibuf++;
      }
      if (*ibuf && (obuf != str)) {
	*(obuf++) = ' ';
      }
      while (*ibuf && (!isspace (*ibuf))) {
	*(obuf++) = *(ibuf++);
      }
    }
    *obuf = 0;
  }
  return str;
}


#ifdef STANDALONE

int main( int argc, char *argv[] )
{
  char *tmp;
  const char *text = "  hello, said the       happy       figment!     ";
  int i;
  int num = numstr( text );
  
  printf( "%d bits in `text'\n", num );
  
  for( i=1; i<=num; i++ ) {
    tmp = parse( text, i );
    puts( tmp );
    free( tmp );
  }

  return 0;
}

#endif
