#include "Python.h"
#include "node.h"
#include "token.h"
#include "graminit.h"
#include "compile.h"
#include "symtable.h"

/*#define UNDEFINED_FUTURE_FEATURE "future feature %.100s is not defined"*/
#define UNDEFINED_FUTURE_FEATURE "future 特性 %.100s 尚未定義"
/*#define FUTURE_IMPORT_STAR "future statement does not support import *"*/
#define FUTURE_IMPORT_STAR "future 指令並不支援 載入 *"

#define FUTURE_POSSIBLE(FF) ((FF)->ff_last_lineno == -1)

static int
future_check_features(PyFutureFeatures *ff, node *n, char *filename)
{
	int i;
	char *feature;
	node *ch;

	REQ(n, import_stmt); /* must by from __future__ import ... */

	for (i = 3; i < NCH(n); i += 2) {
		ch = CHILD(n, i);
		if (TYPE(ch) == STAR) {
			PyErr_SetString(PyExc_SyntaxError,
					FUTURE_IMPORT_STAR);
			PyErr_SyntaxLocation(filename, ch->n_lineno);
			return -1;
		}
		REQ(ch, import_as_name);
		feature = STR(CHILD(ch, 0));
		if (strcmp(feature, FUTURE_NESTED_SCOPES) == 0) {
			ff->ff_nested_scopes = 1;
		} else if (strcmp(feature, "braces") == 0 ||
			   strcmp(feature, "括號") == 0) {
			PyErr_SetString(PyExc_SyntaxError,
					/*"not a chance");*/
					"絕不可能");
			PyErr_SyntaxLocation(filename, CHILD(ch, 0)->n_lineno);
			return -1;
		} else {
			PyErr_Format(PyExc_SyntaxError,
				     UNDEFINED_FUTURE_FEATURE, feature);
			PyErr_SyntaxLocation(filename, CHILD(ch, 0)->n_lineno);
			return -1;
		}
	}
	return 0;
}

static void
future_error(node *n, char *filename)
{
	PyErr_SetString(PyExc_SyntaxError,
			/*"from __future__ imports must occur at the "
			"beginning of the file");*/
			"從 __future__ 載入 語句必須在檔案的首行");
	PyErr_SyntaxLocation(filename, n->n_lineno);
	/* XXX set filename and lineno */
}

/* Relevant portions of the grammar:

single_input: NEWLINE | simple_stmt | compound_stmt NEWLINE
file_input: (NEWLINE | stmt)* ENDMARKER
stmt: simple_stmt | compound_stmt
simple_stmt: small_stmt (';' small_stmt)* [';'] NEWLINE
small_stmt: expr_stmt | print_stmt  | del_stmt | pass_stmt | flow_stmt 
    | import_stmt | global_stmt | exec_stmt | assert_stmt
import_stmt: 'import' dotted_as_name (',' dotted_as_name)* 
    | 'from' dotted_name 'import' ('*' | import_as_name (',' import_as_name)*)
import_as_name: NAME [NAME NAME]
dotted_as_name: dotted_name [NAME NAME]
dotted_name: NAME ('.' NAME)*
*/

/* future_parse() return values:
   -1 indicates an error occurred, e.g. unknown feature name
   0 indicates no feature was found
   1 indicates a feature was found
*/

static int
future_parse(PyFutureFeatures *ff, node *n, char *filename)
{
	int i, r;
 loop:

	switch (TYPE(n)) {

	case single_input:
		if (TYPE(CHILD(n, 0)) == simple_stmt) {
			n = CHILD(n, 0);
			goto loop;
		}
		return 0;

	case file_input:
		for (i = 0; i < NCH(n); i++) {
			node *ch = CHILD(n, i);
			if (TYPE(ch) == stmt) {
				r = future_parse(ff, ch, filename);
				if (!FUTURE_POSSIBLE(ff))
					return r;
			}
		}
		return 0;

	case simple_stmt:
		if (NCH(n) == 2) {
			REQ(CHILD(n, 0), small_stmt);
			n = CHILD(n, 0);
			goto loop;
		} else {
			/* Deal with the special case of a series of
			   small statements on a single line.  If a
			   future statement follows some other
			   statement, the SyntaxError is raised here.
			   In all other cases, the symtable pass
			   raises the exception.
			*/
			int found = 0, end_of_future = 0;

			for (i = 0; i < NCH(n); i += 2) {
				if (TYPE(CHILD(n, i)) == small_stmt) {
					r = future_parse(ff, CHILD(n, i), 
							 filename);
					if (r < 1)
						end_of_future = 1;
					else {
						found = 1;
						if (end_of_future) {
							future_error(n, 
								     filename);
							return -1;
						}
					}
				}
			}

			/* If we found one and only one, then the
			   current lineno is legal. 
			*/
			if (found)
				ff->ff_last_lineno = n->n_lineno + 1;
			else
				ff->ff_last_lineno = n->n_lineno;

			if (end_of_future && found)
				return 1;
			else 
				return 0;
		}
	
	case stmt:
		if (TYPE(CHILD(n, 0)) == simple_stmt) {
			n = CHILD(n, 0);
			goto loop;
		} else if (TYPE(CHILD(n, 0)) == expr_stmt) {
			n = CHILD(n, 0);
			goto loop;
		} else {
			REQ(CHILD(n, 0), compound_stmt);
			ff->ff_last_lineno = n->n_lineno;
			return 0;
		}

	case small_stmt:
		n = CHILD(n, 0);
		goto loop;

	case import_stmt: {
		node *name;
		static char *s = "從";

		if (STR(CHILD(n, 0))[0] != 'f' &&
		    STR(CHILD(n, 0))[0] != s[0]) { /* from/從  */
			ff->ff_last_lineno = n->n_lineno;
			return 0;
		}
		name = CHILD(n, 1);
		if (strcmp(STR(CHILD(name, 0)), "__future__") != 0 &&
		    strcmp(STR(CHILD(name, 0)), "__未來__") != 0 )
			return 0;
		if (future_check_features(ff, n, filename) < 0)
			return -1;
		ff->ff_last_lineno = n->n_lineno + 1;
		return 1;
	}

	/* The cases below -- all of them! -- are necessary to find
	   and skip doc strings. */
	case expr_stmt:
	case testlist:
	case test:
	case and_test:
	case not_test:
	case comparison:
	case expr:
	case xor_expr:
	case and_expr:
	case shift_expr:
	case arith_expr:
	case term:
	case factor:
	case power:
		if (NCH(n) == 1) {
			n = CHILD(n, 0);
			goto loop;
		}
		break;

	case atom:
		if (TYPE(CHILD(n, 0)) == STRING 
		    && ff->ff_found_docstring == 0) {
			ff->ff_found_docstring = 1;
			return 0;
		}
		ff->ff_last_lineno = n->n_lineno;
		return 0;

	default:
		ff->ff_last_lineno = n->n_lineno;
		return 0;
	}
	return 0;
}

PyFutureFeatures *
PyNode_Future(node *n, char *filename)
{
	PyFutureFeatures *ff;

	ff = (PyFutureFeatures *)PyMem_Malloc(sizeof(PyFutureFeatures));
	if (ff == NULL)
		return NULL;
	ff->ff_found_docstring = 0;
	ff->ff_last_lineno = -1;
	ff->ff_nested_scopes = 0;

	if (future_parse(ff, n, filename) < 0) {
		PyMem_Free((void *)ff);
		return NULL;
	}
	return ff;
}

