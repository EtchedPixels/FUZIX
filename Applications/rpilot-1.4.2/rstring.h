#ifndef _RSTRING_H_
#define _RSTRING_H_

#ifdef __cplusplus
extern "C"
{
#endif

char *strupr( char *s );
char *strset( char *s, int ch );
char *new_string( char *src );
char *new_string_from( char *src, int index, int count );

extern int firstnot( const char *d, const char e, int first );
extern int neither( const char *d, const char e, const char f, char first );
extern int find( const char *d, const char *e, int first );
extern char *parse( const char *src, int num );
extern int numstr( const char *d );
extern char *rtrim( char *str );
extern int rws( const char *str);
extern char *scopy( char *dest, char *src, int index, int count );
extern int ws( const char *str );
extern char *ltrim( char *str );
char *trim( char *str );
int wspace( const char *str, int first );
int findchar( const char *str, char c );
char *total_trim( char *str );

#ifdef __cplusplus
}
#endif

#endif /* ifndef _RSTRING_H_ */
