/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

#
/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#define TYPE	typedef
#define STRUCT	TYPE struct
#define UNION	TYPE union
#define REG	register

#define IF	if(
#define THEN	){
#define ELSE	} else {
#define FI	;}

#define FOR	for(
#define WHILE	while(
#define DO	){
#define OD	;}
#define REP	do{
#define PER	}while(
#define DONE	);

#define TRUE	(-1)
#define FALSE	0
#define LOBYTE	0377
#define STRIP	0177
#define QUOTE	0200

#define EOF	0
#define NL	'\n'
#define SP	' '
#define LQ	'`'
#define RQ	'\''
#define MINUS	'-'
#define COLON	':'

#define MAX(a,b)	((a)>(b)?(a):(b))
