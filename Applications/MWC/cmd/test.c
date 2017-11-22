/*
 * New, improved version of the "test" utility, hopefully P1003.2 compliant.
 *
 * "test" has represented a problem to implementors because of the possibility
 * of aliasing between operators and operands; while it is possible to resolve
 * this ambiguity with conventional tools, the resulting program is rather
 * subtle and easily broken.
 *
 * The observation the underpins this implementation is that the "test" input
 * is more accurately parsed /from the right/ than from the left. Because the
 * last element of a form is always an operand rather than an operator, and
 * because unary and binary primaries are lexically distinct, it is possible
 * to create an unambiguous parse with a single right->left scan of the input.
 *
 * Parentheses are a unique problem, however. The easiest way to support them
 * is to say that parentheses are matched only if they are not consumed by
 * any operators, i.e. \( "a" \) would fail to match any operators and so the
 * parenthesis would match.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define	WRITESTR(s)	write (2, s, strlen (s))

enum {
	SYNTAX_ERROR = 2
};


typedef int (*compare_p) (long left, long right);
typedef int (*file_binop_p) (const struct stat * left,
			     const struct stat * right);
typedef int (*log_binop_p) (int left, int right);
typedef int (*string_unop_p) (char *arg);
typedef int (*file_unop_p) (struct stat * stat);

/*
 * Miscellaneous comparison operators.
 */

#define	CMP_OP2(name)	static int name (long arg1, long arg2)

CMP_OP2(cmp_eq)
{
	return arg1 != arg2;
}

CMP_OP2(cmp_neq)
{
	return arg1 == arg2;
}

CMP_OP2(cmp_lt)
{
	return arg1 >= arg2;
}

CMP_OP2(cmp_gt)
{
	return arg1 <= arg2;
}

CMP_OP2(cmp_le)
{
	return arg1 > arg2;
}

CMP_OP2(cmp_ge)
{
	return arg1 < arg2;
}


/*
 * Miscellaneous file operators.
 */

#define	FILE_BINOP(name) \
 static	int name (const struct stat * left, const struct stat * right)

FILE_BINOP(file_eq)
{
	return left->st_dev != right->st_dev
	    || left->st_ino != right->st_ino;
}

FILE_BINOP(file_newer)
{
	return left->st_mtime <= right->st_mtime;
}

FILE_BINOP(file_older)
{
	return left->st_mtime >= right->st_mtime;
}

#define	FILE_UNOP(name)	static int name (const struct stat * stat)

FILE_UNOP(file_exists)
{
	return 0;
}

FILE_UNOP(file_blockspecial)
{
	return (stat->st_mode & S_IFMT) != S_IFBLK;
}

FILE_UNOP(file_charspecial)
{
	return (stat->st_mode & S_IFMT) != S_IFCHR;
}

FILE_UNOP(file_directory)
{
	return (stat->st_mode & S_IFMT) != S_IFDIR;
}

FILE_UNOP(file_pipe)
{
	return (stat->st_mode & S_IFMT) != S_IFIFO;
}

FILE_UNOP(file_regular)
{
	return (stat->st_mode & S_IFMT) != S_IFREG;
}

FILE_UNOP(file_setgid)
{
	return (stat->st_mode & S_ISGID) == 0;
}

FILE_UNOP(file_setuid)
{
	return (stat->st_mode & S_ISUID) == 0;
}

FILE_UNOP(file_sticky)
{
	return (stat->st_mode & S_ISVTX) == 0;
}

FILE_UNOP(file_linked)
{
	return stat->st_nlink == 0;
}

FILE_UNOP(file_nonempty)
{
	return stat->st_size == 0;
}

/*
 * Miscellaneous string operators.
 */

#define	STRING_UNOP(name)	static int name (const char * str)

STRING_UNOP(string_empty)
{
	return str[0] != 0;
}

STRING_UNOP(string_nonempty)
{
	return str[0] == 0;
}

STRING_UNOP(strfile_istty)
{
	return !isatty(atoi(str));
}

STRING_UNOP(strfile_readable)
{
	return access(str, R_OK);
}

STRING_UNOP(strfile_writeable)
{
	return access(str, W_OK);
}

STRING_UNOP(strfile_executable)
{
	return access(str, X_OK);
}

/*
 * Declare and fill in some tables of function names and pointers to the
 * implementations.
 */

struct op {
	const char *op_name;
	void (*op_func)(void);
};
#define	OP(name,func)	{ name, (void (*)(void))func }

const struct op string_binop[] = {
	OP("=", cmp_eq), OP("!=", cmp_neq), OP("<", cmp_lt),
	OP(">", cmp_gt), OP("<=", cmp_le), OP(">=", cmp_ge)
};

const struct op arith_binop[] = {
	OP("-eq", cmp_eq), OP("-ne", cmp_neq), OP("-lt", cmp_lt),
	OP("-gt", cmp_gt), OP("-le", cmp_le), OP("-ge", cmp_ge)
};

const struct op file_binop[] = {
	OP("-ef", file_eq), OP("-nt", file_newer), OP("-ot", file_older)
};

const struct op string_unop[] = {
	OP("-z", string_empty), OP("-n", string_nonempty),
	OP("-t", strfile_istty), OP("-r", strfile_readable),
	OP("-w", strfile_writeable), OP("-x", strfile_executable)
/*	OP ("!", string_empty) */
};

const struct op file_unop[] = {
	OP("-b", file_blockspecial), OP("-c", file_charspecial),
	OP("-d", file_directory), OP("-p", file_pipe),
	OP("-f", file_regular), OP("-g", file_setgid),
	OP("-u", file_setuid), OP("-K", file_sticky),
	OP("-L", file_linked), OP("-s", file_nonempty),
	OP("-e", file_exists)
};

#define	FIND_OP(table,str)	find_op (sizeof (table) / sizeof (* table),\
					 table, str)
#define	is_string_binop(str)	((compare_p) FIND_OP (string_binop, str))
#define	is_arith_binop(str)	((compare_p) FIND_OP (arith_binop, str))
#define	is_file_binop(str)	((file_binop_p) FIND_OP (file_binop, str))
#define	is_string_unop(str)	((string_unop_p) FIND_OP (string_unop, str))
#define	is_file_unop(str)	((file_unop_p) FIND_OP (file_unop, str))

/*
 * Generic operator table search routine.
 */

static void *find_op(int size, const struct op *table, const char *str)
{
	do
		if (strcmp(table->op_name, str) == 0)
			return table->op_func;
	while (table++, --size > 0);

	return NULL;
}


/*
 * Convert an operand of a numeric expression into a number. The rules for
 * how strict the conversion to integer is are not clear.
 *
 * We return 0 on success, non-zero on failure.
 */

int convert_number(const char *string, long *numberp)
{
	char *cp;

	*numberp = strtol(string, &cp, 0);

	/* If strtol failed, it leaves cp at start of str. */
	/* Empty arg "" represents 0 for Unix compatability. */
	return (string == cp) ? (*cp != 0) : 0;
}

enum {
	NO_PAREN,
	IN_PAREN
};

int test_boolor(int argc, char *argv[], int *matchedp, int paren);


/*
 * Process a non-empty element of a test-expression (see test () below).
 * The fundamental premise of this code is that argv [argc - 1] is an
 * operand, and thus that argv [argc - 2] is an operator of some form.
 */

int test_primop(int argc, char *argv[], int *matchedp, int paren)
{
	int result;
	int operator_flag = 1;

	if (argc < 1) {
		return SYNTAX_ERROR;	/* syntax error in subexpr */
	}

	/*
	 * Begin by looking for binary operator at argv [argc - 2].
	 * String operators only match if we have 3 arguments, and the
	 * same is true of arithmetic operators.
	 */

	if (argc >= 3) {
		compare_p cmp;
		file_binop_p file;

		*matchedp = 3;

		if ((cmp = is_string_binop(argv[argc - 2])) != NULL) {
			result = (*cmp) (strcmp(argv[argc - 3],
						argv[argc - 1]), 0);
			goto negate;
		}
		if ((cmp = is_arith_binop(argv[argc - 2])) != NULL) {
			long left;
			long right;

			if (convert_number(argv[argc - 3], &left) == 0 &&
			    convert_number(argv[argc - 1], &right) == 0) {

				result = (*cmp) (left, right);
				goto negate;
			}

			return SYNTAX_ERROR;
		}

		if ((file = is_file_binop(argv[argc - 2])) != NULL) {
			struct stat left;
			struct stat right;

			if (stat(argv[argc - 3], &left) < 0 ||
			    stat(argv[argc - 1], &right) < 0)
				result = 1;
			else
				result = (*file) (&left, &right);
			goto negate;
		}
	}

	if (argc >= 2) {
		string_unop_p string;
		file_unop_p file;

		*matchedp = 2;

		if ((string = is_string_unop(argv[argc - 2])) != NULL) {
			result = (*string) (argv[argc - 1]);
			goto negate;
		}
		if ((file = is_file_unop(argv[argc - 2])) != NULL) {
			struct stat statbuf;

			if (stat(argv[argc - 1], &statbuf) < 0)
				result = 1;
			else
				result = (*file) (&statbuf);
			goto negate;
		}
	}

      not_really_operator:
	operator_flag = 0;

	if (argc > 2 && strcmp(argv[argc - 1], ")") == 0 &&
	    (result = test_boolor(argc - 1, argv, matchedp,
				  IN_PAREN)) != SYNTAX_ERROR &&
	    argc - 1 > *matchedp &&
	    strcmp(argv[argc - 2 - *matchedp], "(") == 0)
		(*matchedp) += 2;
	else {
		*matchedp = 1;
		result = argv[argc - 1][0] == 0;
	}

      negate:
	/*
	 * Try and extend the subexpression on the right by looking
	 * for negations.
	 */

	while (argc > *matchedp) {
		char *next = argv[argc - 1 - *matchedp];

		if (strcmp(next, "!") == 0) {
			(*matchedp)++;
			result = !result;
			continue;
		}

		/*
		 * We now have encountered something on the left... if it is
		 * a conjunction or disjunction operator, we are OK, otherwise
		 * we have hit a syntax error. If we got here by matching an
		 * operator, we can try matching a parenthesis and/or string
		 * as a fallback.
		 */

		if (strcmp(next, "-a") != 0 && strcmp(next, "-o") != 0 &&
		    (paren != IN_PAREN || strcmp(next, "(") != 0) &&
		    operator_flag)
			goto not_really_operator;

		break;
	}

	return result;
}


/*
 * Process an optional sequence of boolean conjunctions. This is separated
 * from disjunction because conjunction has higher "precedence".
 */

int test_booland(int argc, char *argv[], int *matchedp, int paren)
{
	int right = 0;		/* "true" */

	*matchedp = 0;

	/*
	 * Match a sequence of the form:
	 *      and_expr = and_expr AND prim_expr
	 *               | prim_expr ;
	 * Note that this grammar matches the definition of "and" as left-
	 * associative, but in fact since conjunction is commutative there is
	 * no great magic to this and we right-associate instead.
	 */

	for (;;) {
		int left;
		int matched;

		if ((left = test_primop(argc, argv, &matched,
					paren)) == SYNTAX_ERROR) {
			return SYNTAX_ERROR;
		}

		right = right || left;	/* "and" */
		*matchedp += matched;
		argc -= matched;

		if (argc < 2 || strcmp(argv[argc - 1], "-a") != 0)
			return right;

		(*matchedp)++;
		argc--;
	}
}


/*
 * Process an optional sequence of boolean disjunctions.
 */

int test_boolor(int argc, char *argv[], int *matchedp, int paren)
{
	int right = 1;		/* "false" */

	*matchedp = 0;

	/*
	 * Match an expression of the form:
	 *      or_expr = or_expr OR and_expr
	 *              | and_expr ;
	 * Note that this grammar matches the definition of "or" as being
	 * left-associative, but since disjunction is commutative this does
	 * not really matter and we actually associate to the right.
	 */

	for (;;) {
		int left;
		int matched;

		if ((left = test_booland(argc, argv, &matched,
					 paren)) == SYNTAX_ERROR) {
			return SYNTAX_ERROR;
		}

		(*matchedp) += matched;
		argc -= matched;
		right = right && left;	/* "or" */

		if (argc < 2 || strcmp(argv[argc - 1], "-o") != 0)
			return right;

		(*matchedp)++;
		argc--;
	}
}


/*
 * Process a test-expression. The "argc" argument specifies the number of
 * elements in the argument-vector "argv", where each element is a string.
 * Note that unlike main (), the first element of argv [] is a real
 * argument.
 */

int test(int argc, char *argv[])
{
	int matched;
	int value;

	/*
	 * Special cases to allow some simple tests. All these are available
	 * other ways, but they are historically significant.
	 */

	if (argc == 0)
		return 1;	/* false */

#if 0
	if (argc == 1)
		return argv[0][0] == 0;	/* => -n <arg> */
#endif

	if ((value = test_boolor(argc, argv, &matched,
				 NO_PAREN)) == SYNTAX_ERROR ||
	    matched != argc) {
		return SYNTAX_ERROR;
	}

	return value;
}


#if	! _SHELL

#define	WRITECONST(s)	write (2, s, sizeof (s))

void error(char **argv, const char *str)
{
	WRITESTR(*argv);
	WRITECONST(" ");

	while (*++argv != NULL) {
		WRITESTR(*argv);
		WRITECONST(" ");
	}
	WRITECONST(": ");
	WRITESTR(str);
	WRITECONST("\n");

#if	0
	WRITECONST("Unary primaries:\n"
		   "\t-b file\t\tfile exists and is a block special file\n"
		   "\t-c file\t\tfile exists and is a character special file\n"
		   "\t-d file\t\tfile exists and is a directory\n"
		   "\t-e file\t\tfile exists\n"
		   "\t-f file\t\tfile exists and is a regular file\n"
		   "\t-g file\t\tfile exists and is setgid\n"
		   "\t-k file\t\tfile exists and has sticky bit set\t(not Posix)\n"
		   "\t-L file\t\tfile is a link\t\t\t\t(not Posix)\n"
		   "\t-n string\tstring length is nonzero\n"
		   "\t-p file\t\tfile exists and is a named pipe (FIFO)\n"
		   "\t-r file\t\tfile exists and is readable\n"
		   "\t-s file\t\tfile exists and has nonzero size\n"
		   "\t-t fd\t\tfd is the file descriptor of a terminal\n"
		   "\t-u file\t\tfile exists and is setuid\n"
		   "\t-w file\t\tfile exists and is writable\n"
		   "\t-x file\t\tfile exists and is executable\n"
		   "\t-z string\tstring length is zero\n"
		   "\tstring\t\tstring is not the empty string\n");
	WRITECONST("Binary primaries:\n"
		   "\ts1 = s2\t\tstrings s1 and s2 are identical\n"
		   "\ts1 != s2\tstrings s1 and s2 are not identical\n"
		   "\ts1 < s2\t\tstring s1 is less than s2\t\t(not Posix)\n"
		   "\ts1 > s2\t\tstring s1 is greater than s2\t\t(not Posix)\n"
		   "\tfile1 -ef file2\tfile1 and file2 are identical\t\t(not Posix)\n"
		   "\tn1 -eq n2\tnumbers n1 and n2 are equal\n"
		   "\tn1 -ge n2\tnumber n1 is greater than or equal to n2\n"
		   "\tn1 -gt n2\tnumber n1 is greater than n2\n"
		   "\tn1 -le n2\tnumber n1 is less than or equal to n2\n"
		   "\tn1 -lt n2\tnumber n1 is less than n2\n"
		   "\tn1 -ne n2\tnumbers n1 and n2 are not equal\n"
		   "\tfile1 -nt file2\tfile1 is newer than file2\t\t(not Posix)\n"
		   "\tfile1 -ot file2\tfile1 is older than file2\t\t(not Posix)\n");
	WRITECONST("Expression grouping:\n"
		   "\t! exp\t\texp is false\n"
		   "\texp1 -a exp2\texp1 and exp2 are true\t\t\t(not Posix)\n"
		   "\texp1 -o exp2\texp1 or exp2 is true\t\t\t(not Posix)\n"
		   "\t( exp )\t\tparentheses for grouping\t\t(not Posix)\n");
#endif

	exit(2);
}


int main(int argc, char *argv[])
{
	/*
	 * Let's splurge and use a variable for arg 0 length.
	 * A small price to pay for clarity and correctness.
	 */

	int arg0len;

	/*
	 * The following comment added by Hal.  Keep it here.
	 *
	 * If argv[0] ends with '[', look for matching ']'.
	 * If we just string match "[", it's hard to debug things like
	 * /tmp/foo/[ xxx yyy zzz ].
	 */

	arg0len = strlen(argv[0]);

	if (argv[0][arg0len - 1] == '[') {
		if (strcmp(argv[argc - 1], "]") != 0)
			error(argv, "Missing ']'");

		/* Discard trailing ']'. */

		argc--;
	}

	if ((argc = test(argc - 1, argv + 1)) == 2)
		error(argv, "syntax error");

	return argc;
}

#endif				/* ! _SHELL */
