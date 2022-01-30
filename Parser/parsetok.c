
/* Parser-tokenizer link implementation */

#include "pgenheaders.h"
#include "tokenizer.h"
#include "node.h"
#include "grammar.h"
#include "parser.h"
#include "parsetok.h"
#include "errcode.h"
#include "chineseencode.h"

int Py_TabcheckFlag;


/* Forward */
static node *parsetok(struct tok_state *, grammar *, int, perrdetail *);

/* Parse input coming from a string.  Return error code, print some errors. */

node *
PyParser_ParseString(char *s, grammar *g, int start, perrdetail *err_ret)
{
	struct tok_state *tok;
	node *n;

	err_ret->error = E_OK;
	err_ret->filename = NULL;
	err_ret->lineno = 0;
	err_ret->offset = 0;
	err_ret->text = NULL;
	err_ret->token = -1;
	err_ret->expected = -1;

	if ((tok = PyTokenizer_FromString(s)) == NULL) {
		err_ret->error = E_NOMEM;
		return NULL;
	}

	if (Py_TabcheckFlag || Py_VerboseFlag) {
		/*tok->filename = "<string>";*/
		tok->filename = "<¦r¦ê>";
		tok->altwarning = (tok->filename != NULL);
		if (Py_TabcheckFlag >= 2)
			tok->alterror++;
	}

	n = parsetok(tok, g, start, err_ret);
	File_Encoding = Current_Encoding;
	return n;
}


/* Parse input coming from a file.  Return error code, print some errors. */

node *
PyParser_ParseFile(FILE *fp, char *filename, grammar *g, int start,
		   char *ps1, char *ps2, perrdetail *err_ret)
{
	struct tok_state *tok;
	node *n;

	err_ret->error = E_OK;
	err_ret->filename = filename;
	err_ret->lineno = 0;
	err_ret->offset = 0;
	err_ret->text = NULL;

	if ((tok = PyTokenizer_FromFile(fp, ps1, ps2)) == NULL) {
		err_ret->error = E_NOMEM;
		return NULL;
	}
	if (Py_TabcheckFlag || Py_VerboseFlag) {
		tok->filename = filename;
		tok->altwarning = (filename != NULL);
		if (Py_TabcheckFlag >= 2)
			tok->alterror++;
	}

	n = parsetok(tok, g, start, err_ret);
	File_Encoding = Current_Encoding;
	return n;
}

/* Parse input coming from the given tokenizer structure.
   Return error code. */

static node *
parsetok(struct tok_state *tok, grammar *g, int start, perrdetail *err_ret)
{
	parser_state *ps;
	node *n;
	int started = 0;

	if ((ps = PyParser_New(g, start)) == NULL) {
		fprintf(stderr, "no mem for new parser\n");
		err_ret->error = E_NOMEM;
		return NULL;
	}

	for (;;) {
		char *a, *b;
		int type;
		size_t len;
		char *str, *converted;
		char tokentype;

		type = PyTokenizer_Get(tok, &a, &b);
		if (type == ERRORTOKEN) {
			err_ret->error = tok->done;
			break;
		}
		if (type == ENDMARKER && started) {
			type = NEWLINE; /* Add an extra newline */
			started = 0;
		}
		else
			started = 1;
		len = b - a; /* XXX this may compute NULL - NULL */
		str = PyMem_NEW(char, len + 1);
		if (str == NULL) {
			fprintf(stderr, "no mem for next token\n");
			err_ret->error = E_NOMEM;
			break;
		}
		if (len > 0)
			strncpy(str, a, len);
		str[len] = '\0';
		/* convert from GBK to BIG5 when interpreter instance is 
	  	   GBK or when the pure python module imported is in GBK.
		   Note that STRING type tokens are not converted since 
		   I think they shouldn't be. 
		*/
		if (type != STRING)
			tokentype = 'o';
		/* glace-begin untouch triple quote
		else if (len >= 6 && (strncmp(str,"\"\"\"",3) == 0
				|| strncmp(str,"'''",3) == 0)) 
			tokentype = 'm';
		*/
		else
			tokentype = 's';

		switch (tokentype) {
			case 's': /* string types are not converted */
				converted = str;
				break;
			case 'o': /* non string types */
				if (File_Encoding == GBK) {
					converted = gbk_to_big5(str);
					PyMem_DEL(str);
				}
				else
					converted = str;
				break;
			/* glace leave alone since case will never be m now */
			case 'm': 
				/* triple quoted are messages and converted */
				/* now all convert to source encoding(BIG5) */
				if (File_Encoding == GBK) {
					converted = gbk_to_big5(str);
					PyMem_DEL(str);
				}
				/*
				else if(Current_Encoding == GBK 
					&& File_Encoding == BIG5) {
						converted = big5_to_gbk(str);
						PyMem_DEL(str);
				}
				*/
				else {
					converted = str;
				}
				break;
			default:
				converted = str;
		}

		/* printf("parsetok converted:%s\n",converted); */
		if ((err_ret->error =
		     PyParser_AddToken(ps, (int)type, converted, tok->lineno,
				       &(err_ret->expected))) != E_OK) {
			if (err_ret->error != E_DONE)
				PyMem_DEL(converted);
			break;
		}
	}

	if (err_ret->error == E_DONE) {
		n = ps->p_tree;
		ps->p_tree = NULL;
	}
	else
		n = NULL;

	PyParser_Delete(ps);

	if (n == NULL) {
		if (tok->lineno <= 1 && tok->done == E_EOF)
			err_ret->error = E_EOF;
		err_ret->lineno = tok->lineno;
		err_ret->offset = tok->cur - tok->buf;
		if (tok->buf != NULL) {
			size_t len = tok->inp - tok->buf;
			err_ret->text = PyMem_NEW(char, len + 1);
			if (err_ret->text != NULL) {
				if (len > 0)
					strncpy(err_ret->text, tok->buf, len);
				err_ret->text[len] = '\0';
			}
		}
	}

	PyTokenizer_Free(tok);

	return n;
}
