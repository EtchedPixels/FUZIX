/*
 *	Debug aid for token stream
 */


#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"


unsigned getbyte(void)
{
	int n = getchar();
	if (n == EOF) {
		printf("EOF\n");
		exit(0);
	}
	return n;
}

unsigned getpair(void)
{
	unsigned n;
	n = getbyte();
	n |= getbyte() << 8;
	return n;
}


unsigned getquad(void)
{
	unsigned n;
	n = getbyte();
	n |= getbyte() << 8;
	n |= getbyte() << 16;
	n |= getbyte() << 24;
	return n;
}

unsigned gettok(void)
{
	unsigned n = getbyte();
	n |= getbyte() << 8;
	return n;
}

void pchar(unsigned c)
{
	if (c > 126 || c < 32)
		printf("\\x%02x", c);
	else
		printf("%c", c);
}

void dostring(void)
{
	unsigned n;
	do {
		n = getbyte();
		if (n == 255)
			pchar(getbyte());
		else if (n)
			pchar(n);
	} while (n);
	printf("\n");
}

unsigned decode_token(void)
{
	unsigned n = gettok();
	switch (n) {
	case T_EOF:
		printf("EOF\n");
		return 0;
	case T_INVALID:
		printf("INVALID\n");
		return 0;
	case T_POT:
		printf("CONFUSED\n");
		return 0;

	case T_SHLEQ:
		printf(">>=\n");
		break;
	case T_SHREQ:
		printf("<<=\n");
		break;
	case T_POINTSTO:
		printf("->\n");
		break;

	case T_PLUSPLUS:
		printf("++\n");
		break;
	case T_MINUSMINUS:
		printf("--\n");
		break;
	case T_EQEQ:
		printf("==\n");
		break;
	case T_LTLT:
		printf("<<\n");
		break;
	case T_GTGT:
		printf(">>\n");
		break;
	case T_OROR:
		printf("||\n");
		break;
	case T_ANDAND:
		printf("&&\n");

	case T_PLUSEQ:
		printf("+=\n");
		break;
	case T_MINUSEQ:
		printf("-=\n");
		break;
	case T_SLASHEQ:
		printf("/=\n");
		break;
	case T_STAREQ:
		printf("*=\n");
		break;
	case T_HATEQ:
		printf("^=\n");
		break;
	case T_BANGEQ:
		printf("!=\n");
		break;
	case T_OREQ:
		printf("|=\n");
		break;
	case T_ANDEQ:
		printf("&=\n");
		break;
	case T_PERCENTEQ:
		printf("%%=\n");
		break;

	case T_LPAREN:
		printf("(\n");
		break;
	case T_RPAREN:
		printf(")\n");
		break;
	case T_LSQUARE:
		printf("[\n");
		break;
	case T_LCURLY:
		printf("{\n");
		break;
	case T_RCURLY:
		printf("}\n");
		break;
	case T_AND:
		printf("&\n");
		break;
	case T_STAR:
		printf("*\n");
		break;
	case T_SLASH:
		printf("/\n");
		break;
	case T_PERCENT:
		printf("%%\n");
		break;
	case T_PLUS:
		printf("+\n");
		break;
	case T_MINUS:
		printf("-\n");
		break;
	case T_QUESTION:
		printf("?\n");
		break;
	case T_COLON:
		printf(":\n");
		break;
	case T_HAT:
		printf("^\n");
		break;
	case T_LT:
		printf("<\n");
		break;
	case T_GT:
		printf(">\n");
		break;
	case T_OR:
		printf("|\n");
		break;
	case T_TILDE:
		printf("~\n");
		break;
	case T_BANG:
		printf("!\n");
		break;
	case T_EQ:
		printf("=\n");
		break;
	case T_SEMICOLON:
		printf(";\n");
		break;
	case T_DOT:
		printf(".\n");
		break;
	case T_COMMA:
		printf(",\n");
		break;
	case T_AUTO:
		printf("auto\n");
		break;
	case T_CHAR:
		printf("char\n");
		break;
	case T_CONST:
		printf("const\n");
		break;
	case T_DOUBLE:
		printf("double\n");
		break;
	case T_ENUM:
		printf("enum\n");
		break;
	case T_EXTERN:
		printf("extern\n");
		break;
	case T_FLOAT:
		printf("float\n");
		break;
	case T_INT:
		printf("int\n");
		break;
	case T_LONG:
		printf("long\n");
		break;
	case T_REGISTER:
		printf("register\n");
		break;
	case T_SHORT:
		printf("short\n");
		break;
	case T_SIGNED:
		printf("signed\n");
		break;
	case T_STATIC:
		printf("static\n");
		break;
	case T_STRUCT:
		printf("struct\n");
		break;
	case T_UNION:
		printf("union\n");
		break;
	case T_UNSIGNED:
		printf("unsigned\n");
		break;
	case T_VOID:
		printf("void\n");
		break;
	case T_VOLATILE:
		printf("volatile\n");
		break;

	case T_BREAK:
		printf("break\n");
		break;
	case T_CASE:
		printf("case\n");
		break;
	case T_CONTINUE:
		printf("continue\n");
		break;
	case T_DEFAULT:
		printf("default\n");
		break;
	case T_DO:
		printf("do\n");
		break;
	case T_ELSE:
		printf("else\n");
		break;
	case T_FOR:
		printf("for\n");
		break;
	case T_GOTO:
		printf("goto\n");
		break;
	case T_IF:
		printf("if\n");
		break;
	case T_RETURN:
		printf("return\n");
		break;
	case T_SIZEOF:
		printf("sizeof\n");
		break;
	case T_SWITCH:
		printf("switch\n");
		break;
	case T_TYPEDEF:
		printf("typedef\n");
		break;
	case T_WHILE:
		printf("while\n");
		break;

	case T_INTVAL:
		printf("int %d\n", getquad());
		break;
	case T_UINTVAL:
		printf("uint %u\n", getquad());
		break;
		/* We are using 32bit longs for target so this isnt portable but ok for
		   debugging on Linux */
	case T_LONGVAL:
		printf("long %d\n", getquad());
		break;
	case T_ULONGVAL:
		printf("ulong %u\n", getquad());
		break;
	case T_STRING:
		printf("string: ");
		dostring();
		break;
	case T_STRING_END:
		printf("$end\n");
		break;
	case T_LINE:
		printf("Line %d\n", getpair());
		break;
	default:
		if (n >= T_SYMBOL)
			printf("Symbol %d\n", n - T_SYMBOL);
		else {
			printf("Invalid %04x\n", n);
			exit(1);
		}
	}
	return 1;
}


int main(int argc, char *argv[])
{
	while (decode_token());
	return 0;
}
