
/* Built-in functions */

#include "Python.h"

#include "node.h"
#include "compile.h"
#include "eval.h"

#include <ctype.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/* Forward */
static PyObject *filterstring(PyObject *, PyObject *);
static PyObject *filtertuple (PyObject *, PyObject *);

static PyObject *
builtin___import__(PyObject *self, PyObject *args)
{
	char *name;
	PyObject *globals = NULL;
	PyObject *locals = NULL;
	PyObject *fromlist = NULL;

	if (!PyArg_ParseTuple(args, "s|OOO:__import__",
			&name, &globals, &locals, &fromlist))
		return NULL;

	return PyImport_ImportModuleEx(name, globals, locals, fromlist);
}

static char import_doc[] =
/*"__import__(name, globals, locals, fromlist) -> module\n\
\n\
Import a module.  The globals are only used to determine the context;\n\
they are not modified.  The locals are currently unused.  The fromlist\n\
should be a list of names to emulate ``from name import ...'', or an\n\
empty list to emulate ``import name''.\n\
When importing a module from a package, note that __import__('A.B', ...)\n\
returns package A when fromlist is empty, but its submodule B when\n\
fromlist is not empty.";*/
"__更__(嘿, ノ跑计, ╬Τ跑计, 更嘿) -> 家舱\n\
\n\
更家舱.  ノ跑计把计琌ノㄓ∕﹚笲衡办τず甧ぃ穦砆э跑;\n\
╬Τ跑计把计既⊿Τノ. 材 3 把计琌, \n\
ノㄓ家览 ``眖 嘿 更 ..../from name import ...'',\n\
狦琌玥ボ ``更 嘿/import name''.\n\
眖甅ンい更家舱, 猔種 __更__('A.B', ...)\n\
讽嘿把计琌穦肚甅ン A , 讽赣把计\n\
ぃ琌玥肚 B.";


static PyObject *
builtin_abs(PyObject *self, PyObject *args)
{
	PyObject *v;

	if (!PyArg_ParseTuple(args, "O:abs", &v))
		return NULL;
	return PyNumber_Absolute(v);
}

static char abs_doc[] =
/*"abs(number) -> number\n\
\n\
Return the absolute value of the argument.";*/
"荡癸(计) -> 计\n\
\n\
肚计荡癸 (奔タ璽腹计).";


static PyObject *
builtin_apply(PyObject *self, PyObject *args)
{
	PyObject *func, *alist = NULL, *kwdict = NULL;
	PyObject *t = NULL, *retval = NULL;

	if (!PyArg_ParseTuple(args, "O|OO:apply", &func, &alist, &kwdict))
		return NULL;
	if (alist != NULL) {
		if (!PyTuple_Check(alist)) {
			if (!PySequence_Check(alist)) {
				PyErr_Format(PyExc_TypeError,
				     /*"apply() arg 2 expect sequence, found %s",*/
				     "apply/甅ノ() 材把计莱琌,Μ琌 %s",
					     alist->ob_type->tp_name);
				return NULL;
			}
			t = PySequence_Tuple(alist);
			if (t == NULL)
				return NULL;
			alist = t;
		}
	}
	if (kwdict != NULL && !PyDict_Check(kwdict)) {
		PyErr_Format(PyExc_TypeError,
			     /*"apply() arg 3 expected dictionary, found %s",*/
			     "apply/甅ノ() 材把计莱琌ㄥ,Μ琌 %s",
			     kwdict->ob_type->tp_name);
		goto finally;
	}
	retval = PyEval_CallObjectWithKeywords(func, alist, kwdict);
  finally:
	Py_XDECREF(t);
	return retval;
}

static char apply_doc[] =
/*"apply(object[, args[, kwargs]]) -> value\n\
\n\
Call a callable object with positional arguments taken from the tuple args,\n\
and keyword arguments taken from the optional dictionary kwargs.\n\
Note that classes are callable, as are instances with a __call__() method.";*/
"甅ノ(龟砰[, 把计[, ㄥ摸把计]]) -> \n\
\n\
秸ノ㊣把计 '龟砰', ㄤ秸ノ把计 '把计',\n\
┪ 'ㄥ摸把计'.\n\
猔種﹚竡龟砰  __call__/__㊣__() よ猭ㄓ龟砰跑Θ砆秸ノ.";


static PyObject *
builtin_buffer(PyObject *self, PyObject *args)
{
	PyObject *ob;
	int offset = 0;
	int size = Py_END_OF_BUFFER;

	if ( !PyArg_ParseTuple(args, "O|ii:buffer", &ob, &offset, &size) )
	    return NULL;
	return PyBuffer_FromObject(ob, offset, size);
}

static char buffer_doc[] =
/*"buffer(object [, offset[, size]]) -> object\n\
\n\
Create a new buffer object which references the given object.\n\
The buffer will reference a slice of the target object from the\n\
start of the object (or at the specified offset). The slice will\n\
extend to the end of the target object (or with the specified size).";*/
"既(龟砰 [, 熬簿[, ]]) -> 龟砰\n\
\n\
沮倒 '龟砰' 把计玻ネ穝絯侥既龟砰.\n\
赣既穦ヘ夹龟砰ち, 眖龟砰秨﹍竚衡癬,\n\
(狦Τ倒 '簿从' 玥眖赣竚秨﹍衡)\n\
赣ち摸穦 '龟砰' ソ狠. \n\
(埃獶把计 '' い﹚ち).";


static PyObject *
builtin_unicode(PyObject *self, PyObject *args)
{
        PyObject *v;
	char *encoding = NULL;
	char *errors = NULL;

	if ( !PyArg_ParseTuple(args, "O|ss:unicode", &v, &encoding, &errors) )
	    return NULL;
	return PyUnicode_FromEncodedObject(v, encoding, errors);
}

static char unicode_doc[] =
/*"unicode(string [, encoding[, errors]]) -> object\n\
\n\
Create a new Unicode object from the given encoded string.\n\
encoding defaults to the current default string encoding and \n\
errors, defining the error handling, to 'strict'.";*/
"参絏(﹃ [, 絪絏[, 岿粇矪瞶]]) -> 龟砰\n\
\n\
沮把计ㄓ玻ネ参絏絪絏﹃摸.\n\
絪絏箇砞琌╰参箇砞絪絏.\n\
岿粇矪瞶匡兜玥 'strict'.";


static PyObject *
builtin_callable(PyObject *self, PyObject *args)
{
	PyObject *v;

	if (!PyArg_ParseTuple(args, "O:callable", &v))
		return NULL;
	return PyInt_FromLong((long)PyCallable_Check(v));
}

static char callable_doc[] =
/*"callable(object) -> Boolean\n\
\n\
Return whether the object is callable (i.e., some kind of function).\n\
Note that classes are callable, as are instances with a __call__() method.";*/
"㊣(龟砰) -> 痷\n\
\n\
龟砰琌砆讽ㄧ计ㄓ秸ノ杠玥肚痷. \n\
猔種狦龟砰﹚竡いΤ __call__/㊣() 硂疭よ猭杠玥赣龟砰穦Θ㊣.";


static PyObject *
builtin_filter(PyObject *self, PyObject *args)
{
	PyObject *func, *seq, *result;
	PySequenceMethods *sqf;
	int len;
	register int i, j;

	if (!PyArg_ParseTuple(args, "OO:filter", &func, &seq))
		return NULL;

	if (PyString_Check(seq)) {
		PyObject *r = filterstring(func, seq);
		return r;
	}

	if (PyTuple_Check(seq)) {
		PyObject *r = filtertuple(func, seq);
		return r;
	}

	sqf = seq->ob_type->tp_as_sequence;
	if (sqf == NULL || sqf->sq_length == NULL || sqf->sq_item == NULL) {
		PyErr_SetString(PyExc_TypeError,
			   /*"filter() arg 2 must be a sequence");*/
			   "filter/筁耾() 材把计莱琌");
		goto Fail_2;
	}

	if ((len = (*sqf->sq_length)(seq)) < 0)
		goto Fail_2;

	if (PyList_Check(seq) && seq->ob_refcnt == 1) {
		Py_INCREF(seq);
		result = seq;
	}
	else {
		if ((result = PyList_New(len)) == NULL)
			goto Fail_2;
	}

	for (i = j = 0; ; ++i) {
		PyObject *item, *good;
		int ok;

		if ((item = (*sqf->sq_item)(seq, i)) == NULL) {
			if (PyErr_ExceptionMatches(PyExc_IndexError)) {
				PyErr_Clear();
				break;
			}
			goto Fail_1;
		}

		if (func == Py_None) {
			good = item;
			Py_INCREF(good);
		}
		else {
			PyObject *arg = Py_BuildValue("(O)", item);
			if (arg == NULL)
				goto Fail_1;
			good = PyEval_CallObject(func, arg);
			Py_DECREF(arg);
			if (good == NULL) {
				Py_DECREF(item);
				goto Fail_1;
			}
		}
		ok = PyObject_IsTrue(good);
		Py_DECREF(good);
		if (ok) {
			if (j < len) {
				if (PyList_SetItem(result, j++, item) < 0)
					goto Fail_1;
			}
			else {
				int status = PyList_Append(result, item);
				j++;
				Py_DECREF(item);
				if (status < 0)
					goto Fail_1;
			}
		} else {
			Py_DECREF(item);
		}
	}


	if (j < len && PyList_SetSlice(result, j, len, NULL) < 0)
		goto Fail_1;

	return result;

Fail_1:
	Py_DECREF(result);
Fail_2:
	return NULL;
}

static char filter_doc[] =
/*"filter(function, sequence) -> list\n\
\n\
Return a list containing those items of sequence for which function(item)\n\
is true.  If function is None, return a list of items that are true.";*/
"筁耾(ㄧ计, ) -> \n\
\n\
把计 'ㄧ计' 把计肚痷. 筁耾穦珹\n\
把计いㄏ 'ㄧ计' 肚痷じ.";


static PyObject *
builtin_chr(PyObject *self, PyObject *args)
{
	long x;
	char s[1];

	if (!PyArg_ParseTuple(args, "l:chr", &x))
		return NULL;
	if (x < 0 || x >= 256) {
		PyErr_SetString(PyExc_ValueError,
				/*"chr() arg not in range(256)");*/
				"chr/じ() 把计禬 絛瞅(256)");
		return NULL;
	}
	s[0] = (char)x;
	return PyString_FromStringAndSizeAndEncode(s, 1, Current_Encoding);
}

static char chr_doc[] =
/*"chr(i) -> character\n\
\n\
Return a string of one character with ordinal i; 0 <= i < 256.";*/
"じ(i) -> じ才\n\
\n\
肚じ﹃, ㄤ i; 0 <= i < 256.";


static PyObject *
builtin_unichr(PyObject *self, PyObject *args)
{
	long x;
	Py_UNICODE s[1];

	if (!PyArg_ParseTuple(args, "l:unichr", &x))
		return NULL;
	if (x < 0 || x >= 65536) {
		PyErr_SetString(PyExc_ValueError,
				/*"unichr() arg not in range(65536)");*/
				"unichr/参絏() 把计禬 絛瞅(65536)");
		return NULL;
	}
	s[0] = (Py_UNICODE)x;
	return PyUnicode_FromUnicode(s, 1);
}

static char unichr_doc[] =
/*"unichr(i) -> Unicode character\n\
\n\
Return a Unicode string of one character with ordinal i; 0 <= i < 65536.";*/
"参絏(i) -> 参絏才\n\
\n\
肚じ参絏﹃. ㄤ i; 0 <= i < 65536.";


static PyObject *
builtin_cmp(PyObject *self, PyObject *args)
{
	PyObject *a, *b;
	int c;

	if (!PyArg_ParseTuple(args, "OO:cmp", &a, &b))
		return NULL;
	if (PyObject_Cmp(a, b, &c) < 0)
		return NULL;
	return PyInt_FromLong((long)c);
}

static char cmp_doc[] =
/*"cmp(x, y) -> integer\n\
\n\
Return negative if x<y, zero if x==y, positive if x>y.";*/
"ゑ耕(ヒ, ) -> 计\n\
\n\
 ヒ <  肚璽, ヒ单肚 0 , ヒ >  玥肚タ.";


static PyObject *
builtin_coerce(PyObject *self, PyObject *args)
{
	PyObject *v, *w;
	PyObject *res;

	if (!PyArg_ParseTuple(args, "OO:coerce", &v, &w))
		return NULL;
	if (PyNumber_Coerce(&v, &w) < 0)
		return NULL;
	res = Py_BuildValue("(OO)", v, w);
	Py_DECREF(v);
	Py_DECREF(w);
	return res;
}

static char coerce_doc[] =
/*"coerce(x, y) -> None or (x1, y1)\n\
\n\
When x and y can be coerced to values of the same type, return a tuple\n\
containing the coerced values.  When they can't be coerced, return None.";*/
"て(ヒ, ) -> 礚 ┪ (ヒ1, 1)\n\
\n\
 'ヒ' '' 锣传Θ摸计沮玥肚じ舱, ㄤい珹锣传.\n\
ぃ锣传Θ摸玥肚 '礚'/None.";


static PyObject *
builtin_compile(PyObject *self, PyObject *args)
{
	char *str;
	char *filename;
	char *startstr;
	int start;

	if (!PyArg_ParseTuple(args, "sss:compile", &str, &filename, &startstr))
		return NULL;
	if (strcmp(startstr, "exec") == 0 ||
	    strcmp(startstr, "磅︽") == 0)
		start = Py_file_input;
	else if (strcmp(startstr, "eval") == 0 ||
		 strcmp(startstr, "簍衡") == 0)
		start = Py_eval_input;
	else if (strcmp(startstr, "single") == 0 ||
		 strcmp(startstr, "虫︽") == 0)
		start = Py_single_input;
	else {
		PyErr_SetString(PyExc_ValueError,
		   /*"compile() arg 3 must be 'exec' or 'eval' or 'single'");*/
		   "compile/絪亩() 材把计斗 'exec'/'磅︽','eval'/'簍衡','single'/'虫︽'");
		return NULL;
	}
	if (PyEval_GetNestedScopes()) {
		PyCompilerFlags cf;
		cf.cf_nested_scopes = 1;
		return Py_CompileStringFlags(str, filename, start, &cf);
	} else
		return Py_CompileString(str, filename, start);
}

static char compile_doc[] =
/*"compile(source, filename, mode) -> code object\n\
\n\
Compile the source string (a Python module, statement or expression)\n\
into a code object that can be executed by the exec statement or eval().\n\
The filename will be used for run-time error messages.\n\
The mode must be 'exec' to compile a module, 'single' to compile a\n\
single (interactive) statement, or 'eval' to compile an expression.";*/
"絪亩(祘Α絏, 郎, 家Α) -> 絏摸龟砰\n\
\n\
絪亩祘Α絏(﹃篈), 琌い怜家舱, 笷Α┪粂.\n\
肚挡狦絏摸,  磅︽() ┪ 崩衡() 秸ノ把计.\n\
'郎'ノ琌ノㄓ岿粇癟.\n\
把计 '家Α' 斗: 狦琌家舱玥ノ 'exec/磅︽', \n\
琌粂玥ノ 'single/虫︽',\n\
琌笷Α玥ノ 'eval/崩衡'.";


#ifndef WITHOUT_COMPLEX

static PyObject *
complex_from_string(PyObject *v)
{
	extern double strtod(const char *, char **);
	const char *s, *start;
	char *end;
	double x=0.0, y=0.0, z;
	int got_re=0, got_im=0, done=0;
	int digit_or_dot;
	int sw_error=0;
	int sign;
	char buffer[256]; /* For errors */
	char s_buffer[256];
	int len;

	if (PyString_Check(v)) {
		s = PyString_AS_STRING(v);
		len = PyString_GET_SIZE(v);
	}
	else if (PyUnicode_Check(v)) {
		if (PyUnicode_GET_SIZE(v) >= sizeof(s_buffer)) {
			PyErr_SetString(PyExc_ValueError,
				 /*"complex() literal too large to convert");*/
				 "complex/狡计() び,礚猭锣传");
			return NULL;
		}
		if (PyUnicode_EncodeDecimal(PyUnicode_AS_UNICODE(v),
					    PyUnicode_GET_SIZE(v),
					    s_buffer,
					    NULL))
			return NULL;
		s = s_buffer;
		len = (int)strlen(s);
	}
	else if (PyObject_AsCharBuffer(v, &s, &len)) {
		PyErr_SetString(PyExc_TypeError,
				/*"complex() arg is not a string");*/
				"complex/狡计() 把计莱赣琌﹃");
		return NULL;
	}

	/* position on first nonblank */
	start = s;
	while (*s && isspace(Py_CHARMASK(*s)))
		s++;
	if (s[0] == '\0') {
		PyErr_SetString(PyExc_ValueError,
				/*"complex() arg is an empty string");*/
				"complex/狡计() 把计ぃ琌フ﹃");
		return NULL;
	}

	z = -1.0;
	sign = 1;
	do {

		switch (*s) {

		case '\0':
			if (s-start != len) {
				PyErr_SetString(
					PyExc_ValueError,
					/*"complex() arg contains a null byte");*/
					"complex/狡计() 把计いΤ null 才");
				return NULL;
			}
			if(!done) sw_error=1;
			break;

		case '-':
			sign = -1;
				/* Fallthrough */
		case '+':
			if (done)  sw_error=1;
			s++;
			if  (  *s=='\0'||*s=='+'||*s=='-'  ||
			       isspace(Py_CHARMASK(*s))  )  sw_error=1;
			break;

		case 'J':
		case 'j':
			if (got_im || done) {
				sw_error = 1;
				break;
			}
			if  (z<0.0) {
				y=sign;
			}
			else{
				y=sign*z;
			}
			got_im=1;
			s++;
			if  (*s!='+' && *s!='-' )
				done=1;
			break;

		default:
			if (isspace(Py_CHARMASK(*s))) {
				while (*s && isspace(Py_CHARMASK(*s)))
					s++;
				if (s[0] != '\0')
					sw_error=1;
				else
					done = 1;
				break;
			}
			digit_or_dot =
				(*s=='.' || isdigit(Py_CHARMASK(*s)));
			if  (done||!digit_or_dot) {
				sw_error=1;
				break;
			}
			errno = 0;
			PyFPE_START_PROTECT("strtod", return 0)
				z = strtod(s, &end) ;
			PyFPE_END_PROTECT(z)
				if (errno != 0) {
					sprintf(buffer,
					  /*"float() out of range: %.150s", s);*/
					  "float/疊翴计() 禬絛瞅: %.150s", s);
					PyErr_SetString(
						PyExc_ValueError,
						buffer);
					return NULL;
				}
			s=end;
			if  (*s=='J' || *s=='j') {

				break;
			}
			if  (got_re) {
				sw_error=1;
				break;
			}

				/* accept a real part */
			x=sign*z;
			got_re=1;
			if  (got_im)  done=1;
			z = -1.0;
			sign = 1;
			break;

		}  /* end of switch  */

	} while (*s!='\0' && !sw_error);

	if (sw_error) {
		PyErr_SetString(PyExc_ValueError,
				/*"complex() arg is a malformed string");*/
				"complex/狡计() 把计﹃Αぃ癸");
		return NULL;
	}

	return PyComplex_FromDoubles(x,y);
}

static PyObject *
builtin_complex(PyObject *self, PyObject *args)
{
	PyObject *r, *i, *tmp;
	PyNumberMethods *nbr, *nbi = NULL;
	Py_complex cr, ci;
	int own_r = 0;

	i = NULL;
	if (!PyArg_ParseTuple(args, "O|O:complex", &r, &i))
		return NULL;
	if (PyString_Check(r) || PyUnicode_Check(r))
		return complex_from_string(r);
	if ((nbr = r->ob_type->tp_as_number) == NULL ||
	    nbr->nb_float == NULL ||
	    (i != NULL &&
	     ((nbi = i->ob_type->tp_as_number) == NULL ||
	      nbi->nb_float == NULL))) {
		PyErr_SetString(PyExc_TypeError,
			   /*"complex() arg can't be converted to complex");*/
			   "complex/狡计() 把计礚猭锣传Θ狡计篈");
		return NULL;
	}
	/* XXX Hack to support classes with __complex__ method */
	if (PyInstance_Check(r)) {
		static PyObject *complexstr, *complexstr_cp;
		PyObject *f;
		if (complexstr == NULL) {
			complexstr = PyString_InternFromString("__complex__");
			if (complexstr == NULL)
				return NULL;
		}
		if (complexstr_cp == NULL) {
			complexstr_cp = PyString_InternFromString("__狡计__");
			if (complexstr_cp == NULL)
				return NULL;
		}
		f = PyObject_GetAttr(r, complexstr);
		if (f == NULL) {
			PyErr_Clear();
			f = PyObject_GetAttr(r, complexstr_cp);
		}
		if (f == NULL)
			PyErr_Clear();
		else {
			PyObject *args = Py_BuildValue("()");
			if (args == NULL)
				return NULL;
			r = PyEval_CallObject(f, args);
			Py_DECREF(args);
			Py_DECREF(f);
			if (r == NULL)
				return NULL;
			own_r = 1;
		}
	}
	if (PyComplex_Check(r)) {
		cr = ((PyComplexObject*)r)->cval;
		if (own_r) {
			Py_DECREF(r);
		}
	}
	else {
		tmp = PyNumber_Float(r);
		if (own_r) {
			Py_DECREF(r);
		}
		if (tmp == NULL)
			return NULL;
		if (!PyFloat_Check(tmp)) {
			PyErr_SetString(PyExc_TypeError,
					/*"float(r) didn't return a float");*/
					"float/疊翴计(r) 肚ぃ疊翴计");
			Py_DECREF(tmp);
			return NULL;
		}
		cr.real = PyFloat_AsDouble(tmp);
		Py_DECREF(tmp);
		cr.imag = 0.0;
	}
	if (i == NULL) {
		ci.real = 0.0;
		ci.imag = 0.0;
	}
	else if (PyComplex_Check(i))
		ci = ((PyComplexObject*)i)->cval;
	else {
		tmp = (*nbi->nb_float)(i);
		if (tmp == NULL)
			return NULL;
		ci.real = PyFloat_AsDouble(tmp);
		Py_DECREF(tmp);
		ci.imag = 0.;
	}
	cr.real -= ci.imag;
	cr.imag += ci.real;
	return PyComplex_FromCComplex(cr);
}

static char complex_doc[] =
/*"complex(real[, imag]) -> complex number\n\
\n\
Create a complex number from a real part and an optional imaginary part.\n\
This is equivalent to (real + imag*1j) where imag defaults to 0.";*/
"狡计(龟场[, 店场]) -> 狡计\n\
\n\
沮把计龟场㎝店场玻ネ店计计沮.\n\
讽 (龟场 + 店场*1j). ㄤい店场箇砞 0.";


#endif

static PyObject *
builtin_dir_cp(PyObject *self, PyObject *args)
{
	static char *attrlist[] = {"__members__", "__methods__", NULL};
	PyObject *v = NULL, *l = NULL, *m = NULL;
	PyObject *d, *x;
	int i;
	char **s;

	PyObject *newl = NULL;
	int length=0, j, ischinese;
	char *news;

	if (!PyArg_ParseTuple(args, "|O:dir", &v))
		return NULL;
	if (v == NULL) {
		x = PyEval_GetLocals();
		if (x == NULL)
			goto error;
		l = PyMapping_Keys(x);
		if (l == NULL)
			goto error;
	}
	else {
		d = PyObject_GetAttrString(v, "__dict__");
		if (d == NULL)
			PyErr_Clear();
		else {
			l = PyMapping_Keys(d);
			if (l == NULL)
				PyErr_Clear();
			Py_DECREF(d);
		}
		if (l == NULL) {
			l = PyList_New(0);
			if (l == NULL)
				goto error;
		}
		for (s = attrlist; *s != NULL; s++) {
			m = PyObject_GetAttrString(v, *s);
			if (m == NULL) {
				PyErr_Clear();
				continue;
			}
			for (i = 0; ; i++) {
				x = PySequence_GetItem(m, i);
				if (x == NULL) {
					PyErr_Clear();
					break;
				}
				if (PyList_Append(l, x) != 0) {
					Py_DECREF(x);
					Py_DECREF(m);
					goto error;
				}
				Py_DECREF(x);
			}
			Py_DECREF(m);
		}
	}

	newl = PyList_New(0);
	if (newl == NULL)
		goto error;
	for(i = 0; ; i++) {
		x = PySequence_GetItem(l, i);
		if (x == NULL) {
			PyErr_Clear();
			break;
		}
		news = PyString_AsString(x);	
		length = PyString_Size(x);
		ischinese = 0;
		for(j=0;j<length;j++) {		
			if(ishighascii(*(news+j))) {
				ischinese = 1;
				break;
			}
		}
		if(ischinese) {
			if (PyList_Append(newl, x) != 0) {
				Py_DECREF(x);
				goto error;
			}
		}
		Py_DECREF(x);
	}
	Py_DECREF(l);
	return newl;
  error:
	Py_XDECREF(l);
	return NULL;
}

static char dir_cp_doc[] =
"ず甧([龟砰]) -> ﹃ \n\
\n\
㎝ 羆ず甧([]) 肚琌いゅ场だ.";

static PyObject *
builtin_dir(PyObject *self, PyObject *args)
{
	static char *attrlist[] = {"__members__", "__methods__", NULL};
	PyObject *v = NULL, *l = NULL, *m = NULL;
	PyObject *d, *x;
	int i;
	char **s;

	if (!PyArg_ParseTuple(args, "|O:dir", &v))
		return NULL;
	if (v == NULL) {
		x = PyEval_GetLocals();
		if (x == NULL)
			goto error;
		l = PyMapping_Keys(x);
		if (l == NULL)
			goto error;
	}
	else {
		d = PyObject_GetAttrString(v, "__dict__");
		if (d == NULL)
			PyErr_Clear();
		else {
			l = PyMapping_Keys(d);
			if (l == NULL)
				PyErr_Clear();
			Py_DECREF(d);
		}
		if (l == NULL) {
			l = PyList_New(0);
			if (l == NULL)
				goto error;
		}
		for (s = attrlist; *s != NULL; s++) {
			m = PyObject_GetAttrString(v, *s);
			if (m == NULL) {
				PyErr_Clear();
				continue;
			}
			for (i = 0; ; i++) {
				x = PySequence_GetItem(m, i);
				if (x == NULL) {
					PyErr_Clear();
					break;
				}
				if (PyList_Append(l, x) != 0) {
					Py_DECREF(x);
					Py_DECREF(m);
					goto error;
				}
				Py_DECREF(x);
			}
			Py_DECREF(m);
		}
	}
	if (PyList_Sort(l) != 0)
		goto error;
	return l;
  error:
	Py_XDECREF(l);
	return NULL;
}

static char dir_doc[] =
/*"dir([object]) -> list of strings\n\
\n\
Return an alphabetized list of names comprising (some of) the attributes\n\
of the given object.  Without an argument, the names in the current scope\n\
are listed.  With an instance argument, only the instance attributes are\n\
returned.  With a class argument, attributes of the base class are not\n\
returned.  For other types or arguments, this may list members or methods.";*/
"羆ず甧([龟砰]) -> ﹃\n\
\n\
肚﹃. 赣い把计龟砰い (ぃ琌场) 妮┦.\n\
狦⊿Τ倒把计玥肚ヘ玡笲衡办い﹚竡跑计,嘿.\n\
狦把计琌龟砰杠玥肚赣龟砰い妮┦τぃ珹ㄤいよ猭.\n\
狦把计琌摸杠玥肚いぃ珹ㄤ阀├い妮┦.\n\
癸ㄤ把计玥肚よ猭┪妮┦单.";


static PyObject *
builtin_divmod(PyObject *self, PyObject *args)
{
	PyObject *v, *w;

	if (!PyArg_ParseTuple(args, "OO:divmod", &v, &w))
		return NULL;
	return PyNumber_Divmod(v, w);
}

static char divmod_doc[] =
/*"divmod(x, y) -> (div, mod)\n\
\n\
Return the tuple ((x-x%y)/y, x%y).  Invariant: div*y + mod == x.";*/
"坝緇计(x, y) -> (坝, 緇计)\n\
\n\
肚じ舱, ず甧 ((x-x%y)/y, x%y).  猔種Τ硂㏕﹚闽玒: div*y + mod == x.";


static PyObject *
builtin_eval(PyObject *self, PyObject *args)
{
	PyObject *cmd;
	PyObject *globals = Py_None, *locals = Py_None;
	char *str;

	if (!PyArg_ParseTuple(args, "O|O!O!:eval",
			&cmd,
			&PyDict_Type, &globals,
			&PyDict_Type, &locals))
		return NULL;
	if (globals == Py_None) {
		globals = PyEval_GetGlobals();
		if (locals == Py_None)
			locals = PyEval_GetLocals();
	}
	else if (locals == Py_None)
		locals = globals;
	if (PyDict_GetItemString(globals, "__builtins__") == NULL) {
		if (PyDict_SetItemString(globals, "__builtins__",
					 PyEval_GetBuiltins()) != 0)
			return NULL;
	}
	if (PyCode_Check(cmd))
		return PyEval_EvalCode((PyCodeObject *) cmd, globals, locals);
	if (!PyString_Check(cmd) &&
	    !PyUnicode_Check(cmd)) {
		PyErr_SetString(PyExc_TypeError,
			   /*"eval() arg 1 must be a string or code object");*/
			   "eval/簍衡() 材把计斗﹃┪祘Α絏摸");
		return NULL;
	}
	if (PyString_AsStringAndSize(cmd, &str, NULL))
		return NULL;
	while (*str == ' ' || *str == '\t')
		str++;
	return PyRun_String(str, Py_eval_input, globals, locals);
}

static char eval_doc[] =
/*"eval(source[, globals[, locals]]) -> value\n\
\n\
Evaluate the source in the context of globals and locals.\n\
The source may be a string representing a Python expression\n\
or a code object as returned by compile().\n\
The globals and locals are dictionaries, defaulting to the current\n\
globals and locals.  If only globals is given, locals defaults to it.";*/
"崩衡(祘Α絏[, ノ跑计[, ╬Τ跑计]]) -> \n\
\n\
ノ跑计㎝╬Τ跑计秈︽崩衡┮ノ笲衡办, ㄥ篈.\n\
祘Α絏琌い怜Α絏 (﹃Α) ┪琌ノ \n\
絪亩/compile() 肚絏摸计沮.\n\
笲衡办箇砞琌ヘ玡笲衡办, 狦倒ノ跑计玥╬Τ跑计\n\
穦砞Θ㎝ウ妓.";


static PyObject *
builtin_execfile(PyObject *self, PyObject *args)
{
	char *filename;
	PyObject *globals = Py_None, *locals = Py_None;
	PyObject *res;
	FILE* fp;

	if (!PyArg_ParseTuple(args, "s|O!O!:execfile",
			&filename,
			&PyDict_Type, &globals,
			&PyDict_Type, &locals))
		return NULL;
	if (globals == Py_None) {
		globals = PyEval_GetGlobals();
		if (locals == Py_None)
			locals = PyEval_GetLocals();
	}
	else if (locals == Py_None)
		locals = globals;
	if (PyDict_GetItemString(globals, "__builtins__") == NULL) {
		if (PyDict_SetItemString(globals, "__builtins__",
					 PyEval_GetBuiltins()) != 0)
			return NULL;
		if (PyDict_SetItemString(globals, "__ず__",
					 PyEval_GetBuiltins()) != 0)
			return NULL;
	}
	Py_BEGIN_ALLOW_THREADS
	fp = fopen(filename, "r");
	Py_END_ALLOW_THREADS
	if (fp == NULL) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}
	if (PyEval_GetNestedScopes()) {
		PyCompilerFlags cf;
		cf.cf_nested_scopes = 1;
		res = PyRun_FileExFlags(fp, filename, Py_file_input, globals,
				   locals, 1, &cf);
	} else
		res = PyRun_FileEx(fp, filename, Py_file_input, globals,
				   locals, 1);
	return res;
}

static char execfile_doc[] =
/*"execfile(filename[, globals[, locals]])\n\
\n\
Read and execute a Python script from a file.\n\
The globals and locals are dictionaries, defaulting to the current\n\
globals and locals.  If only globals is given, locals defaults to it.";*/
"磅︽郎(郎[, ノ跑计[, ╬Τ跑计]])\n\
\n\
弄磅︽郎い祘Αず甧.\n\
ノ跑计㎝╬Τ跑计ㄥ摸跑计, 箇砞\n\
碞琌ヘ玡笲衡办い.  狦倒ノ跑计杠玥\n\
╬Τ跑计砞Θ㎝ウ妓.";


static PyObject *
builtin_getattr(PyObject *self, PyObject *args)
{
	PyObject *v, *result, *dflt = NULL;
	PyObject *name;

	if (!PyArg_ParseTuple(args, "OO|O:getattr", &v, &name, &dflt))
		return NULL;
	result = PyObject_GetAttr(v, name);
	if (result == NULL && dflt != NULL) {
		PyErr_Clear();
		Py_INCREF(dflt);
		result = dflt;
	}
	return result;
}

static char getattr_doc[] =
/*"getattr(object, name[, default]) -> value\n\
\n\
Get a named attribute from an object; getattr(x, 'y') is equivalent to x.y.\n\
When a default argument is given, it is returned when the attribute doesn't\n\
exist; without it, an exception is raised in that case.";*/
"弄妮┦(龟砰, 嘿[, 箇砞]) -> \n\
\n\
把计龟砰い弄 '嘿' 妮┦; 弄妮┦(x, 'y') 讽 x.y\n\
箇砞狦тぃ赣兜妮┦肚.\n\
⊿Τ '箇砞' 硂把计玥тぃ赣妮┦穦ま祇钵盽.";


static PyObject *
builtin_globals(PyObject *self, PyObject *args)
{
	PyObject *d;

	if (!PyArg_ParseTuple(args, ":globals"))
		return NULL;
	d = PyEval_GetGlobals();
	Py_INCREF(d);
	return d;
}

static char globals_doc[] =
/*"globals() -> dictionary\n\
\n\
Return the dictionary containing the current scope's global variables.";*/
"ノ跑计() -> ㄥ\n\
\n\
肚ヘ玡笲衡办いノ跑计.";


static PyObject *
builtin_hasattr(PyObject *self, PyObject *args)
{
	PyObject *v;
	PyObject *name;

	if (!PyArg_ParseTuple(args, "OO:hasattr", &v, &name))
		return NULL;
	v = PyObject_GetAttr(v, name);
	if (v == NULL) {
		PyErr_Clear();
		Py_INCREF(Py_False);
		return Py_False;
	}
	Py_DECREF(v);
	Py_INCREF(Py_True);
	return Py_True;
}

static char hasattr_doc[] =
/*"hasattr(object, name) -> Boolean\n\
\n\
Return whether the object has an attribute with the given name.\n\
(This is done by calling getattr(object, name) and catching exceptions.)";*/
"Τ妮┦(龟砰, ) -> 痷\n\
\n\
把计龟砰いΤ '' 硂妮┦玥肚痷, 玥肚安.\n\
(暗猭琌秸ノ 弄妮┦(龟砰, ) 礛浪琩Τ⊿Τま祇钵盽."; 


static PyObject *
builtin_id(PyObject *self, PyObject *args)
{
	PyObject *v;

	if (!PyArg_ParseTuple(args, "O:id", &v))
		return NULL;
	return PyLong_FromVoidPtr(v);
}

static char id_doc[] =
/*"id(object) -> integer\n\
\n\
Return the identity of an object.  This is guaranteed to be unique among\n\
simultaneously existing objects.  (Hint: it's the object's memory address.)";*/
"腹(龟砰) -> 俱计\n\
\n\
肚把计龟砰ō腹絏.  硂腹玂靡癸龟砰摸琌縒礚.\n\
(矗ボ: ウ膀セ赣龟砰癘拘砰, 腹龟砰ゲ﹚琌龟砰)";


static PyObject *
builtin_map(PyObject *self, PyObject *args)
{
	typedef struct {
		PyObject *seq;
		PySequenceMethods *sqf;
		int saw_IndexError;
	} sequence;

	PyObject *func, *result;
	sequence *seqs = NULL, *sqp;
	int n, len;
	register int i, j;

	n = PyTuple_Size(args);
	if (n < 2) {
		PyErr_SetString(PyExc_TypeError,
				/*"map() requires at least two args");*/
				"map/癸莱() 惠璶癬絏ㄢ把计");
		return NULL;
	}

	func = PyTuple_GetItem(args, 0);
	n--;

	if (func == Py_None && n == 1) {
		/* map(None, S) is the same as list(S). */
		return PySequence_List(PyTuple_GetItem(args, 1));
	}

	if ((seqs = PyMem_NEW(sequence, n)) == NULL) {
		PyErr_NoMemory();
		goto Fail_2;
	}

	/* Do a first pass to (a) verify the args are sequences; (b) set
	 * len to the largest of their lengths; (c) initialize the seqs
	 * descriptor vector.
	 */
	for (len = 0, i = 0, sqp = seqs; i < n; ++i, ++sqp) {
		int curlen;
		PySequenceMethods *sqf;

		if ((sqp->seq = PyTuple_GetItem(args, i + 1)) == NULL)
			goto Fail_2;

		sqp->saw_IndexError = 0;

		sqp->sqf = sqf = sqp->seq->ob_type->tp_as_sequence;
		if (sqf == NULL ||
		    sqf->sq_item == NULL)
		{
			static char errmsg[] =
			    /*"argument %d to map() must be a sequence object";*/
			    "map/癸莱() 把计 %d 斗篈";
			char errbuf[sizeof(errmsg) + 25];

			sprintf(errbuf, errmsg, i+2);
			PyErr_SetString(PyExc_TypeError, errbuf);
			goto Fail_2;
		}

		if (sqf->sq_length == NULL)
			/* doesn't matter -- make something up */
			curlen = 8;
		else
			curlen = (*sqf->sq_length)(sqp->seq);
		if (curlen < 0)
			goto Fail_2;
		if (curlen > len)
			len = curlen;
	}

	if ((result = (PyObject *) PyList_New(len)) == NULL)
		goto Fail_2;

	/* Iterate over the sequences until all have raised IndexError. */
	for (i = 0; ; ++i) {
		PyObject *alist, *item=NULL, *value;
		int any = 0;

		if (func == Py_None && n == 1)
			alist = NULL;
		else {
			if ((alist = PyTuple_New(n)) == NULL)
				goto Fail_1;
		}

		for (j = 0, sqp = seqs; j < n; ++j, ++sqp) {
			if (sqp->saw_IndexError) {
				Py_INCREF(Py_None);
				item = Py_None;
			}
			else {
				item = (*sqp->sqf->sq_item)(sqp->seq, i);
				if (item == NULL) {
					if (PyErr_ExceptionMatches(
						PyExc_IndexError))
					{
						PyErr_Clear();
						Py_INCREF(Py_None);
						item = Py_None;
						sqp->saw_IndexError = 1;
					}
					else {
						goto Fail_0;
					}
				}
				else
					any = 1;

			}
			if (!alist)
				break;
			if (PyTuple_SetItem(alist, j, item) < 0) {
				Py_DECREF(item);
				goto Fail_0;
			}
			continue;

		Fail_0:
			Py_XDECREF(alist);
			goto Fail_1;
		}

		if (!alist)
			alist = item;

		if (!any) {
			Py_DECREF(alist);
			break;
		}

		if (func == Py_None)
			value = alist;
		else {
			value = PyEval_CallObject(func, alist);
			Py_DECREF(alist);
			if (value == NULL)
				goto Fail_1;
		}
		if (i >= len) {
			int status = PyList_Append(result, value);
			Py_DECREF(value);
			if (status < 0)
				goto Fail_1;
		}
		else {
			if (PyList_SetItem(result, i, value) < 0)
				goto Fail_1;
		}
	}

	if (i < len && PyList_SetSlice(result, i, len, NULL) < 0)
		goto Fail_1;

	PyMem_DEL(seqs);
	return result;

Fail_1:
	Py_DECREF(result);
Fail_2:
	if (seqs) PyMem_DEL(seqs);
	return NULL;
}

static char map_doc[] =
/*"map(function, sequence[, sequence, ...]) -> list\n\
\n\
Return a list of the results of applying the function to the items of\n\
the argument sequence(s).  If more than one sequence is given, the\n\
function is called with an argument list consisting of the corresponding\n\
item of each sequence, substituting None for missing values when not all\n\
sequences have the same length.  If the function is None, return a list of\n\
the items of the sequence (or a list of tuples if more than one sequence).";*/
"癸莱(ㄧ计, [, , ...]) -> \n\
\n\
秸ノㄧ计 'ㄧ计', 秸ノ把计 ''.\n\
狦倒玥穦ㄌΩ秸ノ 'ㄧ计', р┮Τㄧ计肚抖 ''い.\n\
狦ぃ璓玥穦 '礚'/'None' ㄓ恶干.\n\
狦 '礚'/'None'  'ㄧ计', 玥肚 '', \n\
┪讽Τ禬筁肚じ舱. (珃, 硈иΤ翴絢额.)";


static PyObject *
builtin_setattr(PyObject *self, PyObject *args)
{
	PyObject *v;
	PyObject *name;
	PyObject *value;

	if (!PyArg_ParseTuple(args, "OOO:setattr", &v, &name, &value))
		return NULL;
	if (PyObject_SetAttr(v, name, value) != 0)
		return NULL;
	Py_INCREF(Py_None);
	return Py_None;
}

static char setattr_doc[] =
/*"setattr(object, name, value)\n\
\n\
Set a named attribute on an object; setattr(x, 'y', v) is equivalent to\n\
``x.y = v''.";*/
"砞妮┦(龟砰, 嘿, )\n\
\n\
把计龟砰い妮┦; 砞妮┦(x, 'y', v) \n\
单 ``x.y = v''.";


static PyObject *
builtin_delattr(PyObject *self, PyObject *args)
{
	PyObject *v;
	PyObject *name;

	if (!PyArg_ParseTuple(args, "OO:delattr", &v, &name))
		return NULL;
	if (PyObject_SetAttr(v, name, (PyObject *)NULL) != 0)
		return NULL;
	Py_INCREF(Py_None);
	return Py_None;
}

static char delattr_doc[] =
/*"delattr(object, name)\n\
\n\
Delete a named attribute on an object; delattr(x, 'y') is equivalent to\n\
``del x.y''.";*/
"妮┦(龟砰, 嘿)\n\
\n\
埃龟砰い '嘿' 硂妮┦; 妮┦(x, 'y') \n\
单 ``del x.y''.";


static PyObject *
builtin_hash(PyObject *self, PyObject *args)
{
	PyObject *v;
	long x;

	if (!PyArg_ParseTuple(args, "O:hash", &v))
		return NULL;
	x = PyObject_Hash(v);
	if (x == -1)
		return NULL;
	return PyInt_FromLong(x);
}

static char hash_doc[] =
/*"hash(object) -> integer\n\
\n\
Return a hash value for the object.  Two objects with the same value have\n\
the same hash value.  The reverse is not necessarily true, but likely.";*/
"磣(龟砰) -> 俱计\n\
\n\
肚龟砰磣. 龟砰Τ磣.\n\
ㄢ龟砰Τ磣ボウ琌, ぃ玂靡﹚.";


static PyObject *
builtin_hex(PyObject *self, PyObject *args)
{
	PyObject *v;
	PyNumberMethods *nb;

	if (!PyArg_ParseTuple(args, "O:hex", &v))
		return NULL;

	if ((nb = v->ob_type->tp_as_number) == NULL ||
	    nb->nb_hex == NULL) {
		PyErr_SetString(PyExc_TypeError,
			   /*"hex() argument can't be converted to hex");*/
			   "hex/せ秈() 把计礚猭锣传Θせ秈计");
		return NULL;
	}
	return (*nb->nb_hex)(v);
}

static char hex_doc[] =
/*"hex(number) -> string\n\
\n\
Return the hexadecimal representation of an integer or long integer.";*/
"せ秈(计) -> ﹃\n\
\n\
肚せ秈ボ计";


static PyObject *builtin_raw_input(PyObject *, PyObject *);

static PyObject *
builtin_input(PyObject *self, PyObject *args)
{
	PyObject *line;
	char *str;
	PyObject *res;
	PyObject *globals, *locals;

	line = builtin_raw_input(self, args);
	if (line == NULL)
		return line;
	if (!PyArg_Parse(line, "s;embedded '\\0' in input line", &str))
		return NULL;
	while (*str == ' ' || *str == '\t')
			str++;
	globals = PyEval_GetGlobals();
	locals = PyEval_GetLocals();
	if (PyDict_GetItemString(globals, "__builtins__") == NULL) {
		if (PyDict_SetItemString(globals, "__builtins__",
					 PyEval_GetBuiltins()) != 0)
			return NULL;
		if (PyDict_SetItemString(globals, "__ず__",
					 PyEval_GetBuiltins()) != 0)
			return NULL;
	}
	res = PyRun_String(str, Py_eval_input, globals, locals);
	Py_DECREF(line);
	return res;
}

static char input_doc[] =
/*"input([prompt]) -> value\n\
\n\
Equivalent to eval(raw_input(prompt)).";*/
"崩衡弄([矗ボ]) -> \n\
\n\
讽 崩衡(弄(矗ボ)) / eval(raw_input(prompt)).";


static PyObject *
builtin_intern(PyObject *self, PyObject *args)
{
	PyObject *s;
	if (!PyArg_ParseTuple(args, "S:intern", &s))
		return NULL;
	Py_INCREF(s);
	PyString_InternInPlace(&s);
	return s;
}

static char intern_doc[] =
/*"intern(string) -> string\n\
\n\
``Intern'' the given string.  This enters the string in the (global)\n\
table of interned strings whose purpose is to speed up dictionary lookups.\n\
Return the string itself or the previously interned string object with the\n\
same value.";*/
"ず(﹃) -> ﹃\n\
\n\
``Intern/ず'' 倒把计﹃.  硂妓暗穦р赣﹃ (ノ)\n\
ずて﹃ぇい, 暗ㄥま琩т硉穦еㄇ.\n\
肚﹃セō, 狦玡巨竒ずて﹃玥肚玡.";


static PyObject *
builtin_int(PyObject *self, PyObject *args)
{
	PyObject *v;
	int base = -909;		     /* unlikely! */

	if (!PyArg_ParseTuple(args, "O|i:int", &v, &base))
		return NULL;
	if (base == -909)
		return PyNumber_Int(v);
	else if (PyString_Check(v))
		return PyInt_FromString(PyString_AS_STRING(v), NULL, base);
	else if (PyUnicode_Check(v))
		return PyInt_FromUnicode(PyUnicode_AS_UNICODE(v),
					 PyUnicode_GET_SIZE(v),
					 base);
	else {
		PyErr_SetString(PyExc_TypeError,
				/*"int() can't convert non-string with explicit base");*/
				"int/俱计() ぃр獶﹃篈戈锣传Θ﹚计╰");
		return NULL;
	}
}

static char int_doc[] =
/*"int(x[, base]) -> integer\n\
\n\
Convert a string or number to an integer, if possible.  A floating point\n\
argument will be truncated towards zero (this does not include a string\n\
representation of a floating point number!)  When converting a string, use\n\
the optional base.  It is an error to supply a base when converting a\n\
non-string.";*/
"俱计(x) -> 俱计\n\
俱计(x, 膀┏) -> 俱计\n\
\n\
р﹃┪计锣传Θ俱计篈.\n\
疊翴计穦传Θ俱计. (猔種硂ぃ珹﹃篈ボ疊翴计.\n\
狦锣传琌﹃, 玥穦倒 '膀┏' 锣传计╰.\n\
狦锣传ぃ琌﹃, 玥ぃノ '膀┏' 把计.";


static PyObject *
builtin_long(PyObject *self, PyObject *args)
{
	PyObject *v;
	int base = -909;		     /* unlikely! */

	if (!PyArg_ParseTuple(args, "O|i:long", &v, &base))
		return NULL;
	if (base == -909)
		return PyNumber_Long(v);
	else if (PyString_Check(v))
		return PyLong_FromString(PyString_AS_STRING(v), NULL, base);
	else if (PyUnicode_Check(v))
		return PyLong_FromUnicode(PyUnicode_AS_UNICODE(v),
					  PyUnicode_GET_SIZE(v),
					  base);
	else {
		PyErr_SetString(PyExc_TypeError,
				/*"long() can't convert non-string with explicit base");*/
				"long/俱计() ぃр獶﹃篈戈锣传Θ﹚计╰");
		return NULL;
	}
}

static char long_doc[] =
/*"long(x) -> long integer\n\
long(x, base) -> long integer\n\
\n\
Convert a string or number to a long integer, if possible.  A floating\n\
point argument will be truncated towards zero (this does not include a\n\
string representation of a floating point number!)  When converting a\n\
string, use the given base.  It is an error to supply a base when\n\
converting a non-string.";*/
"俱计(x) -> 俱计\n\
俱计(x, 膀┏) -> 俱计\n\
\n\
р﹃┪计锣传Θ俱计篈.\n\
疊翴计穦传Θ俱计. (猔種硂ぃ珹﹃篈ボ疊翴计.\n\
狦锣传琌﹃, 玥穦倒 '膀┏' 锣传计╰.\n\
狦锣传ぃ琌﹃, 玥ぃノ '膀┏' 把计.";


static PyObject *
builtin_float(PyObject *self, PyObject *args)
{
	PyObject *v;

	if (!PyArg_ParseTuple(args, "O:float", &v))
		return NULL;
	if (PyString_Check(v))
		return PyFloat_FromString(v, NULL);
	return PyNumber_Float(v);
}

static char float_doc[] =
/*"float(x) -> floating point number\n\
\n\
Convert a string or number to a floating point number, if possible.";*/
"疊翴计(x) -> 疊翴计\n\
\n\
杠, р把计﹃┪计ヘ跑Θ疊翴计.";


static PyObject *
builtin_len(PyObject *self, PyObject *args)
{
	PyObject *v;
	long res;

	if (!PyArg_ParseTuple(args, "O:len", &v))
		return NULL;
	res = PyObject_Size(v);
	if (res < 0 && PyErr_Occurred())
		return NULL;
	return PyInt_FromLong(res);
}

static char len_doc[] =
/*"len(object) -> integer\n\
\n\
Return the number of items of a sequence or mapping.";*/
"(龟砰) -> 俱计\n\
\n\
肚┪ㄥ摸いじ计ヘ, ﹃のΤ﹚竡龟砰.";


static PyObject *
builtin_list(PyObject *self, PyObject *args)
{
	PyObject *v;

	if (!PyArg_ParseTuple(args, "O:list", &v))
		return NULL;
	return PySequence_List(v);
}

static char list_doc[] =
/*"list(sequence) -> list\n\
\n\
Return a new list whose items are the same as those of the argument sequence.";*/
"跑(摸) -> \n\
\n\
肚把计 '摸' 跑Θ.";


static PyObject *
builtin_slice(PyObject *self, PyObject *args)
{
	PyObject *start, *stop, *step;

	start = stop = step = NULL;

	if (!PyArg_ParseTuple(args, "O|OO:slice", &start, &stop, &step))
		return NULL;

	/* This swapping of stop and start is to maintain similarity with
	   range(). */
	if (stop == NULL) {
		stop = start;
		start = NULL;
	}
	return PySlice_New(start, stop, step);
}

static char slice_doc[] =
/*"slice([start,] stop[, step]) -> slice object\n\
\n\
Create a slice object.  This is used for slicing by the Numeric extensions.";*/
"ち([﹍,] 沧[, ˙秈]) -> ち摸\n\
\n\
玻ネち摸龟砰. 硂倒 Numeric extensions(计耎甶家舱) ノ.";


static PyObject *
builtin_locals(PyObject *self, PyObject *args)
{
	PyObject *d;

	if (!PyArg_ParseTuple(args, ":locals"))
		return NULL;
	d = PyEval_GetLocals();
	Py_INCREF(d);
	return d;
}

static char locals_doc[] =
/*"locals() -> dictionary\n\
\n\
Return the dictionary containing the current scope's local variables.";*/
"╬Τ跑计() -> ㄥ\n\
\n\
肚ヘ玡笲衡办いセ跑计.";


static PyObject *
min_max(PyObject *args, int op)
{
	int i;
	PyObject *v, *w, *x;
	PySequenceMethods *sq;

	if (PyTuple_Size(args) > 1)
		v = args;
	else if (!PyArg_ParseTuple(args, "O:min/max", &v))
		return NULL;
	sq = v->ob_type->tp_as_sequence;
	if (sq == NULL || sq->sq_item == NULL) {
		PyErr_SetString(PyExc_TypeError,
				/*"min() or max() arg must be a sequence");*/
				"min/程(), max/程() 把计斗摸");
		return NULL;
	}
	w = NULL;
	for (i = 0; ; i++) {
		x = (*sq->sq_item)(v, i); /* Implies INCREF */
		if (x == NULL) {
			if (PyErr_ExceptionMatches(PyExc_IndexError)) {
				PyErr_Clear();
				break;
			}
			Py_XDECREF(w);
			return NULL;
		}
		if (w == NULL)
			w = x;
		else {
			int cmp = PyObject_RichCompareBool(x, w, op);
			if (cmp > 0) {
				Py_DECREF(w);
				w = x;
			}
			else if (cmp < 0) {
				Py_DECREF(x);
				Py_XDECREF(w);
				return NULL;
			}
			else
				Py_DECREF(x);
		}
	}
	if (w == NULL)
		PyErr_SetString(PyExc_ValueError,
				/*"min() or max() arg is an empty sequence");*/
				"min/程(), max/程() 把计ぃ琌");
	return w;
}

static PyObject *
builtin_min(PyObject *self, PyObject *v)
{
	return min_max(v, Py_LT);
}

static char min_doc[] =
/*"min(sequence) -> value\n\
min(a, b, c, ...) -> value\n\
\n\
With a single sequence argument, return its smallest item.\n\
With two or more arguments, return the smallest argument.";*/
"程() -> \n\
程(a, b, c, ...) -> \n\
\n\
Τ把计, 倒い程じ.\n\
狦Τㄢ┪把计玥肚┮Τ把计い程.";


static PyObject *
builtin_max(PyObject *self, PyObject *v)
{
	return min_max(v, Py_GT);
}

static char max_doc[] =
/*"max(sequence) -> value\n\
max(a, b, c, ...) -> value\n\
\n\
With a single sequence argument, return its largest item.\n\
With two or more arguments, return the largest argument.";*/
"程() -> \n\
程(a, b, c, ...) -> \n\
\n\
狦Τ把计, 肚赣把计い程じ.\n\
狦禬筁把计, 肚┮Τ把计い程.";


static PyObject *
builtin_oct(PyObject *self, PyObject *args)
{
	PyObject *v;
	PyNumberMethods *nb;

	if (!PyArg_ParseTuple(args, "O:oct", &v))
		return NULL;
	if (v == NULL || (nb = v->ob_type->tp_as_number) == NULL ||
	    nb->nb_oct == NULL) {
		PyErr_SetString(PyExc_TypeError,
			   /*"oct() argument can't be converted to oct");*/
			   "oct/秈() 把计ぃ锣传Θ秈计");
		return NULL;
	}
	return (*nb->nb_oct)(v);
}

static char oct_doc[] =
/*"oct(number) -> string\n\
\n\
Return the octal representation of an integer or long integer.";*/
"秈(计) -> ﹃\n\
\n\
肚俱计把计秈ボΑ.";


static PyObject *
builtin_open(PyObject *self, PyObject *args)
{
	char *name;
	char *mode = "r";
	int bufsize = -1;
	PyObject *f;

	if (!PyArg_ParseTuple(args, "s|si:open", &name, &mode, &bufsize))
		return NULL;
	f = PyFile_FromString(name, mode);
	if (f != NULL)
		PyFile_SetBufSize(f, bufsize);
	return f;
}

static char open_doc[] =
/*"open(filename[, mode[, buffering]]) -> file object\n\
\n\
Open a file.  The mode can be 'r', 'w' or 'a' for reading (default),\n\
writing or appending.  The file will be created if it doesn't exist\n\
when opened for writing or appending; it will be truncated when\n\
opened for writing.  Add a 'b' to the mode for binary files.\n\
Add a '+' to the mode to allow simultaneous reading and writing.\n\
If the buffering argument is given, 0 means unbuffered, 1 means line\n\
buffered, and larger numbers specify the buffer size."*/
"ゴ秨(郎[, 家Α[, 絯侥]]) -> 郎摸 \n\
\n\
ゴ秨郎.  家Α琌 'r', 'w', 'a' 弄 (箇砞),\n\
糶, の .  糶の家Α狦⊿Τ赣郎玥穦玻ネ穝郎.\n\
安竒Τ郎ê糶家Α侣郎穦砆滦籠奔.\n\
璶弄秈郎叫 'b' . 璶弄糶玥 '+' 才腹.\n\
'絯侥' : 0 ぃノ既, 1 既︽ず甧, \n\
计ボ赣絯侥.";


static PyObject *
builtin_ord(PyObject *self, PyObject *args)
{
	PyObject *obj;
	long ord;
	int size;

	if (!PyArg_ParseTuple(args, "O:ord", &obj))
		return NULL;

	if (PyString_Check(obj)) {
		size = PyString_GET_SIZE(obj);
		if (size == 1) {
			ord = (long)((unsigned char)*PyString_AS_STRING(obj));
			return PyInt_FromLong(ord);
		}
	} else if (PyUnicode_Check(obj)) {
		size = PyUnicode_GET_SIZE(obj);
		if (size == 1) {
			ord = (long)*PyUnicode_AS_UNICODE(obj);
			return PyInt_FromLong(ord);
		}
	} else {
		PyErr_Format(PyExc_TypeError,
			     /*"ord() expected string of length 1, but " \
			     "%.200s found", obj->ob_type->tp_name);*/
			     "ord/絏() 把计莱琌 1 ﹃, Μ " \
			     "%.200s", obj->ob_type->tp_name);
		return NULL;
	}

	PyErr_Format(PyExc_TypeError,
		     /*"ord() expected a character, "
		     "but string of length %d found",*/
		     "ord/絏() 把计莱虫才, "
		     "Μ %d ﹃",
		     size);
	return NULL;
}

static char ord_doc[] =
/*"ord(c) -> integer\n\
\n\
Return the integer ordinal of a one-character string.";*/
"絏(c) -> 俱计\n\
\n\
肚虫じ﹃絏. ( 127 玥琌赣じ ASCII 絏)";


static PyObject *
builtin_pow(PyObject *self, PyObject *args)
{
	PyObject *v, *w, *z = Py_None;

	if (!PyArg_ParseTuple(args, "OO|O:pow", &v, &w, &z))
		return NULL;
	return PyNumber_Power(v, w, z);
}

static char pow_doc[] =
/*"pow(x, y[, z]) -> number\n\
\n\
With two arguments, equivalent to x**y.  With three arguments,\n\
equivalent to (x**y) % z, but may be more efficient (e.g. for longs).";*/
"经(x, y[, z]) -> 计\n\
\n\
ㄢ把计薄猵讽 x**y.  把计薄猵,\n\
讽 (x**y) % z, 计讽穦耕 (x**y) % z 蔼.";


/* Return number of items in range/xrange (lo, hi, step).  step > 0
 * required.  Return a value < 0 if & only if the true value is too
 * large to fit in a signed long.
 */
static long
get_len_of_range(long lo, long hi, long step)
{
	/* -------------------------------------------------------------
	If lo >= hi, the range is empty.
	Else if n values are in the range, the last one is
	lo + (n-1)*step, which must be <= hi-1.  Rearranging,
	n <= (hi - lo - 1)/step + 1, so taking the floor of the RHS gives
	the proper value.  Since lo < hi in this case, hi-lo-1 >= 0, so
	the RHS is non-negative and so truncation is the same as the
	floor.  Letting M be the largest positive long, the worst case
	for the RHS numerator is hi=M, lo=-M-1, and then
	hi-lo-1 = M-(-M-1)-1 = 2*M.  Therefore unsigned long has enough
	precision to compute the RHS exactly.
	---------------------------------------------------------------*/
	long n = 0;
	if (lo < hi) {
		unsigned long uhi = (unsigned long)hi;
		unsigned long ulo = (unsigned long)lo;
		unsigned long diff = uhi - ulo - 1;
		n = (long)(diff / (unsigned long)step + 1);
	}
	return n;
}

static PyObject *
builtin_range(PyObject *self, PyObject *args)
{
	long ilow = 0, ihigh = 0, istep = 1;
	long bign;
	int i, n;

	PyObject *v;

	if (PyTuple_Size(args) <= 1) {
		if (!PyArg_ParseTuple(args,
				/*"l;range() requires 1-3 int arguments",*/
				"l;range/絛瞅() 惠璶 1  3 俱计把计",
				&ihigh))
			return NULL;
	}
	else {
		if (!PyArg_ParseTuple(args,
				/*"ll|l;range() requires 1-3 int arguments",*/
				"ll|l;range/絛瞅() 惠璶 1  3 俱计把计",
				&ilow, &ihigh, &istep))
			return NULL;
	}
	if (istep == 0) {
		/*PyErr_SetString(PyExc_ValueError, "range() arg 3 must not be zero");*/
		PyErr_SetString(PyExc_ValueError, "range/絛瞅() 材把计ぃ琌 0 ");
		return NULL;
	}
	if (istep > 0)
		bign = get_len_of_range(ilow, ihigh, istep);
	else
		bign = get_len_of_range(ihigh, ilow, -istep);
	n = (int)bign;
	if (bign < 0 || (long)n != bign) {
		PyErr_SetString(PyExc_OverflowError,
				/*"range() result has too many items");*/
				"range/絛瞅() 肚じ计ヘび");
		return NULL;
	}
	v = PyList_New(n);
	if (v == NULL)
		return NULL;
	for (i = 0; i < n; i++) {
		PyObject *w = PyInt_FromLong(ilow);
		if (w == NULL) {
			Py_DECREF(v);
			return NULL;
		}
		PyList_SET_ITEM(v, i, w);
		ilow += istep;
	}
	return v;
}

static char range_doc[] =
/*"range([start,] stop[, step]) -> list of integers\n\
\n\
Return a list containing an arithmetic progression of integers.\n\
range(i, j) returns [i, i+1, i+2, ..., j-1]; start (!) defaults to 0.\n\
When step is given, it specifies the increment (or decrement).\n\
For example, range(4) returns [0, 1, 2, 3].  The end point is omitted!\n\
These are exactly the valid indices for a list of 4 elements.";*/
"絛瞅([﹍,] 沧[, ˙秈]) -> 俱计\n\
\n\
肚, 赣い抖逼衡砃计.\n\
絛瞅(ヒ, ) ボ [ヒ, ヒ+1, ヒ+2, ..., -1]; '﹍' (!) 箇砞 0.\n\
狦倒把计 '˙秈', 玥ボい綟计畉.\n\
ㄒ: 絛瞅(4) ボ [0, 1, 2, 3].  猔種絛瞅いぃ珹 '4'!";

static PyObject *
builtin_xrange(PyObject *self, PyObject *args)
{
	long ilow = 0, ihigh = 0, istep = 1;
	long n;

	if (PyTuple_Size(args) <= 1) {
		if (!PyArg_ParseTuple(args,
				/*"l;xrange() requires 1-3 int arguments",*/
				"l;xrange/虏絛瞅() 惠璶 1  3 俱计把计",
				&ihigh))
			return NULL;
	}
	else {
		if (!PyArg_ParseTuple(args,
				/*"ll|l;xrange() requires 1-3 int arguments",*/
				"ll|l;xrange/虏絛瞅() 惠璶 1  3 俱计把计",
				&ilow, &ihigh, &istep))
			return NULL;
	}
	if (istep == 0) {
		/*PyErr_SetString(PyExc_ValueError, "xrange() arg 3 must not be zero");*/
		PyErr_SetString(PyExc_ValueError, "xrange/虏絛瞅() 材把计ぃ琌 0 ");
		return NULL;
	}
	if (istep > 0)
		n = get_len_of_range(ilow, ihigh, istep);
	else
		n = get_len_of_range(ihigh, ilow, -istep);
	if (n < 0) {
		PyErr_SetString(PyExc_OverflowError,
				/*"xrange() result has too many items");*/
				"xrange/虏絛瞅() 肚じ计ヘび");
		return NULL;
	}
	return PyRange_New(ilow, n, istep, 1);
}

static char xrange_doc[] =
/*"xrange([start,] stop[, step]) -> xrange object\n\
\n\
Like range(), but instead of returning a list, returns an object that\n\
generates the numbers in the range on demand.  This is slightly slower\n\
than range() but more memory efficient.";*/
"虏絛瞅([﹍,] 沧[, ˙秈]) -> 虏絛瞅摸龟砰\n\
\n\
㎝ 絛瞅() , 肚ぃ琌τ琌龟砰, \n\
赣龟砰穦ㄌ惠璶τ玻ネ癸莱絛瞅い计. \n\
㎝ 絛瞅() 耕璶篊ㄇ琌竊ぃぶ癘拘砰.";


static PyObject *
builtin_raw_input(PyObject *self, PyObject *args)
{
	PyObject *v = NULL;
	PyObject *f;

	if (!PyArg_ParseTuple(args, "|O:[raw_]input", &v))
		return NULL;
	if (PyFile_AsFile(PySys_GetObject("stdin")) == stdin &&
	    PyFile_AsFile(PySys_GetObject("stdout")) == stdout &&
	    isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		PyObject *po;
		char *prompt;
		char *s;
		PyObject *result;
		if (v != NULL) {
			po = PyObject_Str(v);
			if (po == NULL)
				return NULL;
			prompt = PyString_AsString(po);
			if (prompt == NULL)
				return NULL;
		}
		else {
			po = NULL;
			prompt = "";
		}
		s = PyOS_Readline(prompt);
		Py_XDECREF(po);
		if (s == NULL) {
			PyErr_SetNone(PyExc_KeyboardInterrupt);
			return NULL;
		}
		if (*s == '\0') {
			PyErr_SetNone(PyExc_EOFError);
			result = NULL;
		}
		else { /* strip trailing '\n' */
			size_t len = strlen(s);
			if (len > INT_MAX) {
				/*PyErr_SetString(PyExc_OverflowError, "input too long");*/
				PyErr_SetString(PyExc_OverflowError, "块び");
				result = NULL;
			}
			else {
				result = PyString_FromStringAndSizeAndEncode(s, 
						(int)(len-1), Current_Encoding);
			}
		}
		PyMem_FREE(s);
		return result;
	}
	if (v != NULL) {
		f = PySys_GetObject("stdout");
		if (f == NULL) {
			/*PyErr_SetString(PyExc_RuntimeError, "lost sys.stdout");*/
			PyErr_SetString(PyExc_RuntimeError, "sys.stdout/╰参.夹非块 メ");
			return NULL;
		}
		if (Py_FlushLine() != 0 ||
		    PyFile_WriteObject(v, f, Py_PRINT_RAW) != 0)
			return NULL;
	}
	f = PySys_GetObject("stdin");
	if (f == NULL) {
		/*PyErr_SetString(PyExc_RuntimeError, "lost sys.stdin");*/
		PyErr_SetString(PyExc_RuntimeError, "sys.stdin/╰参.夹非块 メ");
		return NULL;
	}
	return PyFile_GetLine(f, -1);
}

static char raw_input_doc[] =
/*"raw_input([prompt]) -> string\n\
\n\
Read a string from standard input.  The trailing newline is stripped.\n\
If the user hits EOF (Unix: Ctl-D, Windows: Ctl-Z+Return), raise EOFError.\n\
On Unix, GNU readline is used if enabled.  The prompt string, if given,\n\
is printed without a trailing newline before reading.";*/
"弄([矗ボ]) -> \n\
\n\
眖夹非块杆竚弄﹃. 挡Ю传︽才腹穦砆掉. \n\
狦ノめ块龄 EOF 癘腹 (Unix: Ctl-D, Windows: Ctl-Z+Return), \n\
玥ま祇 郎沧挡钵盽/EOFError.\n\
 Unix, 狦杠穦ㄏノ GNU readline. '矗ボ' 穦陪ボㄓ,\n\
礛癬︽秈︽弄巨.";


static PyObject *
builtin_reduce(PyObject *self, PyObject *args)
{
	PyObject *seq, *func, *result = NULL;
	PySequenceMethods *sqf;
	register int i;

	if (!PyArg_ParseTuple(args, "OO|O:reduce", &func, &seq, &result))
		return NULL;
	if (result != NULL)
		Py_INCREF(result);

	sqf = seq->ob_type->tp_as_sequence;
	if (sqf == NULL || sqf->sq_item == NULL) {
		PyErr_SetString(PyExc_TypeError,
		    /*"reduce() arg 2 must be a sequence");*/
		    "reduce/て虏() 材把计斗琌");
		return NULL;
	}

	if ((args = PyTuple_New(2)) == NULL)
		goto Fail;

	for (i = 0; ; ++i) {
		PyObject *op2;

		if (args->ob_refcnt > 1) {
			Py_DECREF(args);
			if ((args = PyTuple_New(2)) == NULL)
				goto Fail;
		}

		if ((op2 = (*sqf->sq_item)(seq, i)) == NULL) {
			if (PyErr_ExceptionMatches(PyExc_IndexError)) {
				PyErr_Clear();
				break;
			}
			goto Fail;
		}

		if (result == NULL)
			result = op2;
		else {
			PyTuple_SetItem(args, 0, result);
			PyTuple_SetItem(args, 1, op2);
			if ((result = PyEval_CallObject(func, args)) == NULL)
				goto Fail;
		}
	}

	Py_DECREF(args);

	if (result == NULL)
		PyErr_SetString(PyExc_TypeError,
			   /*"reduce() of empty sequence with no initial value");*/
			   "reduce/て虏() ⊿倒箇砞");

	return result;

Fail:
	Py_XDECREF(args);
	Py_XDECREF(result);
	return NULL;
}

static char reduce_doc[] =
/*"reduce(function, sequence[, initial]) -> value\n\
\n\
Apply a function of two arguments cumulatively to the items of a sequence,\n\
from left to right, so as to reduce the sequence to a single value.\n\
For example, reduce(lambda x, y: x+y, [1, 2, 3, 4, 5]) calculates\n\
((((1+2)+3)+4)+5).  If initial is present, it is placed before the items\n\
of the sequence in the calculation, and serves as a default when the\n\
sequence is empty.";*/
"て虏(ㄧ计, [, 癬﹍]) -> \n\
\n\
把计 'ㄧ计' 琌弄ㄢ把计肚巨. 赣ㄧ计穦\n\
仓縩眖オノ '' 肚程挡狦.\n\
ㄒ: て虏(lambda x, y: x+y, [1, 2, 3, 4, 5]) 穦璸衡\n\
((((1+2)+3)+4)+5).  狦倒把计 '癬﹍', 璸衡赣穦い\n\
材じ, 琌薄猵箇璸.";


static PyObject *
builtin_reload(PyObject *self, PyObject *args)
{
	PyObject *v;

	if (!PyArg_ParseTuple(args, "O:reload", &v))
		return NULL;
	return PyImport_ReloadModule(v);
}

static char reload_doc[] =
/*"reload(module) -> module\n\
\n\
Reload the module.  The module must have been successfully imported before.";*/
"更(家舱) -> 家舱\n\
\n\
穝更更家舱. 家舱ゲ斗Ν玡Θ\更筁.";


static PyObject *
builtin_repr(PyObject *self, PyObject *args)
{
	PyObject *v;

	if (!PyArg_ParseTuple(args, "O:repr", &v))
		return NULL;
	return PyObject_Repr(v);
}

static char repr_doc[] =
/*"repr(object) -> string\n\
\n\
Return the canonical string representation of the object.\n\
For most object types, eval(repr(object)) == object.";*/
"ゅボ(龟砰) -> ﹃\n\
\n\
肚把计 '龟砰' タ砏﹃ボ.\n\
癸场だ龟砰ㄓ弧 崩衡(ゅボ(龟砰)) == 龟砰 / eval(repr(object)) == object.";


static PyObject *
builtin_round(PyObject *self, PyObject *args)
{
	double x;
	double f;
	int ndigits = 0;
	int i;

	if (!PyArg_ParseTuple(args, "d|i:round", &x, &ndigits))
			return NULL;
	f = 1.0;
	i = abs(ndigits);
	while  (--i >= 0)
		f = f*10.0;
	if (ndigits < 0)
		x /= f;
	else
		x *= f;
	if (x >= 0.0)
		x = floor(x + 0.5);
	else
		x = ceil(x - 0.5);
	if (ndigits < 0)
		x *= f;
	else
		x /= f;
	return PyFloat_FromDouble(x);
}

static char round_doc[] =
/*"round(number[, ndigits]) -> floating point number\n\
\n\
Round a number to a given precision in decimal digits (default 0 digits).\n\
This always returns a floating point number.  Precision may be negative.";*/
"彼(计[, 计]) -> 疊翴计\n\
\n\
肚把计 '计' 彼き '计'. (箇砞计 0).\n\
肚﹚琌疊翴计. 弘絋璽计";


static PyObject *
builtin_str(PyObject *self, PyObject *args)
{
	PyObject *v;

	if (!PyArg_ParseTuple(args, "O:str", &v))
		return NULL;
	return PyObject_Str(v);
}

static char str_doc[] =
/*"str(object) -> string\n\
\n\
Return a nice string representation of the object.\n\
If the argument is a string, the return value is the same object.";*/
"跑﹃(龟砰) -> ﹃\n\
\n\
﹃篈ボ把计.\n\
狦把计セㄓ碞琌﹃摸玥肚把计セō.";


static PyObject *
builtin_tuple(PyObject *self, PyObject *args)
{
	PyObject *v;

	if (!PyArg_ParseTuple(args, "O:tuple", &v))
		return NULL;
	return PySequence_Tuple(v);
}

static char tuple_doc[] =
/*"tuple(sequence) -> list\n\
\n\
Return a tuple whose items are the same as those of the argument sequence.\n\
If the argument is a tuple, the return value is the same object.";*/
"跑じ舱(摸) -> \n\
\n\
肚じ舱, ㄤいじ㎝把计い妓.\n\
狦把计セō琌じ舱,玥肚把计セō.";


static PyObject *
builtin_type(PyObject *self, PyObject *args)
{
	PyObject *v;

	if (!PyArg_ParseTuple(args, "O:type", &v))
		return NULL;
	v = (PyObject *)v->ob_type;
	Py_INCREF(v);
	return v;
}

static char type_doc[] =
/*"type(object) -> type object\n\
\n\
Return the type of the object.";*/
"摸(龟砰) -> 摸\n\
\n\
肚龟砰摸.";


static PyObject *
builtin_vars(PyObject *self, PyObject *args)
{
	PyObject *v = NULL;
	PyObject *d;

	if (!PyArg_ParseTuple(args, "|O:vars", &v))
		return NULL;
	if (v == NULL) {
		d = PyEval_GetLocals();
		if (d == NULL) {
			if (!PyErr_Occurred())
				PyErr_SetString(PyExc_SystemError,
						/*"no locals!?");*/
						"⊿Τ╬Τ跑计!?");
		}
		else
			Py_INCREF(d);
	}
	else {
		d = PyObject_GetAttrString(v, "__dict__");
		if (d == NULL) {
			PyErr_SetString(PyExc_TypeError,
			    /*"vars() argument must have __dict__ attribute");*/
			    "vars/跑Μ() 把计莱璶Τ __dict__/__ㄥ__ 硂妮┦");
			return NULL;
		}
	}
	return d;
}

static char vars_doc[] =
/*"vars([object]) -> dictionary\n\
\n\
Without arguments, equivalent to locals().\n\
With an argument, equivalent to object.__dict__.";*/
"跑计([龟砰]) -> ㄥ\n\
\n\
⊿Τ把计, 单 locals/╬Τ跑计().\n\
Τ把计, 单 龟砰.__ㄥ__/object.__dict__";

static PyObject *
builtin_isinstance(PyObject *self, PyObject *args)
{
	PyObject *inst;
	PyObject *cls;
	int retval;

	if (!PyArg_ParseTuple(args, "OO:isinstance", &inst, &cls))
		return NULL;

	retval = PyObject_IsInstance(inst, cls);
	if (retval < 0)
		return NULL;
	return PyInt_FromLong(retval);
}

static char isinstance_doc[] =
/*"isinstance(object, class-or-type) -> Boolean\n\
\n\
Return whether an object is an instance of a class or of a subclass thereof.\n\
With a type as second argument, return whether that is the object's type.";*/
"琌龟砰(龟砰, 阀├┪摸) -> 痷\n\
\n\
 '龟砰' 琌琘阀├┪ㄤまビ阀├龟瞷玥肚痷.\n\
倒材 2  把计, 玥肚 '龟砰' 琌㎝赣把计(龟砰┪计沮摸)摸.";


static PyObject *
builtin_issubclass(PyObject *self, PyObject *args)
{
	PyObject *derived;
	PyObject *cls;
	int retval;

	if (!PyArg_ParseTuple(args, "OO:issubclass", &derived, &cls))
		return NULL;

	retval = PyObject_IsSubclass(derived, cls);
	if (retval < 0)
		return NULL;
	return PyInt_FromLong(retval);
}

static char issubclass_doc[] =
/*"issubclass(C, B) -> Boolean\n\
\n\
Return whether class C is a subclass (i.e., a derived class) of class B.";*/
"琌阀├(C, B) -> 痷┪安\n\
\n\
 C 琌 B 阀├阀├(膥┯闽玒)玥肚 1, 玥肚 0.";


static PyObject*
builtin_zip(PyObject *self, PyObject *args)
{
	PyObject *ret;
	int itemsize = PySequence_Length(args);
	int i, j;

	if (itemsize < 1) {
		PyErr_SetString(PyExc_TypeError,
				/*"zip() requires at least one sequence");*/
				"zip/癸() 把计癬絏璶Τ");
		return NULL;
	}
	/* args must be a tuple */
	assert(PyTuple_Check(args));

	if ((ret = PyList_New(0)) == NULL)
		return NULL;

	for (i = 0;; i++) {
		PyObject *next = PyTuple_New(itemsize);
		if (!next) {
			Py_DECREF(ret);
			return NULL;
		}
		for (j = 0; j < itemsize; j++) {
			PyObject *seq = PyTuple_GET_ITEM(args, j);
			PyObject *item = PySequence_GetItem(seq, i);

			if (!item) {
				if (PyErr_ExceptionMatches(PyExc_IndexError)) {
					PyErr_Clear();
					Py_DECREF(next);
					return ret;
				}
				Py_DECREF(next);
				Py_DECREF(ret);
				return NULL;
			}
			PyTuple_SET_ITEM(next, j, item);
		}
		PyList_Append(ret, next);
		Py_DECREF(next);
	}
	/* no return */
}


static char zip_doc[] =
/*"zip(seq1 [, seq2 [...]]) -> [(seq1[0], seq2[0] ...), (...)]\n\
\n\
Return a list of tuples, where each tuple contains the i-th element\n\
from each of the argument sequences.  The returned list is truncated\n\
in length to the length of the shortest argument sequence.";*/
"癸(1 [, 2 [...]]) -> [(1[0], 2[0] ...), (...)]\n\
\n\
肚じ舱, い材 i じ舱Τ┮Τ把计材 i じ.\n\
肚把计い程祏非.";


static PyMethodDef builtin_methods[] = {
	{"__import__",	builtin___import__, 1, import_doc},
	{"__更__",	builtin___import__, 1, import_doc},
	{"abs",		builtin_abs, 1, abs_doc},
	{"荡癸",		builtin_abs, 1, abs_doc},
	{"apply",	builtin_apply, 1, apply_doc},
	{"甅ノ",	builtin_apply, 1, apply_doc},
	{"buffer",	builtin_buffer, 1, buffer_doc},
	{"既",	builtin_buffer, 1, buffer_doc},
	{"callable",	builtin_callable, 1, callable_doc},
	{"㊣",	builtin_callable, 1, callable_doc},
	{"chr",		builtin_chr, 1, chr_doc},
	{"じ",		builtin_chr, 1, chr_doc},
	{"cmp",		builtin_cmp, 1, cmp_doc},
	{"ゑ耕",		builtin_cmp, 1, cmp_doc},
	{"coerce",	builtin_coerce, 1, coerce_doc},
	{"て",	builtin_coerce, 1, coerce_doc},
	{"compile",	builtin_compile, 1, compile_doc},
	{"絪亩",	builtin_compile, 1, compile_doc},
#ifndef WITHOUT_COMPLEX
	{"complex",	builtin_complex, 1, complex_doc},
	{"狡计",	builtin_complex, 1, complex_doc},
#endif
	{"delattr",	builtin_delattr, 1, delattr_doc},
	{"妮┦",	builtin_delattr, 1, delattr_doc},
	{"dir",		builtin_dir, 1, dir_doc},
	{"羆ず甧",	builtin_dir, 1, dir_doc},
	{"cdir",	builtin_dir_cp, 1, dir_cp_doc},
	{"ず甧",	builtin_dir_cp, 1, dir_cp_doc},
	{"divmod",	builtin_divmod, 1, divmod_doc},
	{"坝緇计",	builtin_divmod, 1, divmod_doc},
	{"eval",	builtin_eval, 1, eval_doc},
	{"崩衡",	builtin_eval, 1, eval_doc},
	{"execfile",	builtin_execfile, 1, execfile_doc},
	{"磅︽ゅン",	builtin_execfile, 1, execfile_doc},
	{"filter",	builtin_filter, 1, filter_doc},
	{"筁耾",	builtin_filter, 1, filter_doc},
	{"float",	builtin_float, 1, float_doc},
	{"疊翴计",	builtin_float, 1, float_doc},
	{"getattr",	builtin_getattr, 1, getattr_doc},
	{"弄妮┦",	builtin_getattr, 1, getattr_doc},
	{"globals",	builtin_globals, 1, globals_doc},
	{"ノ跑计",	builtin_globals, 1, globals_doc},
	{"hasattr",	builtin_hasattr, 1, hasattr_doc},
	{"Τ妮┦",	builtin_hasattr, 1, hasattr_doc},
	{"hash",	builtin_hash, 1, hash_doc},
	{"磣",	builtin_hash, 1, hash_doc},
	{"hex",		builtin_hex, 1, hex_doc},
	{"せ秈",		builtin_hex, 1, hex_doc},
	{"id",		builtin_id, 1, id_doc},
	{"腹",		builtin_id, 1, id_doc},
	{"input",	builtin_input, 1, input_doc},
	{"崩衡弄",	builtin_input, 1, input_doc},
	{"intern",	builtin_intern, 1, intern_doc},
	{"ず",	builtin_intern, 1, intern_doc},
	{"int",		builtin_int, 1, int_doc},
	{"俱计",		builtin_int, 1, int_doc},
	{"isinstance",  builtin_isinstance, 1, isinstance_doc},
	{"琌龟砰",  builtin_isinstance, 1, isinstance_doc},
	{"issubclass",  builtin_issubclass, 1, issubclass_doc},
	{"琌阀├",  builtin_issubclass, 1, issubclass_doc},
	{"len",		builtin_len, 1, len_doc},
	{"",		builtin_len, 1, len_doc},
	{"list",	builtin_list, 1, list_doc},
	{"跑",	builtin_list, 1, list_doc},
	{"locals",	builtin_locals, 1, locals_doc},
	{"╬Τ跑计",	builtin_locals, 1, locals_doc},
	{"long",	builtin_long, 1, long_doc},
	{"俱计",	builtin_long, 1, long_doc},
	{"map",		builtin_map, 1, map_doc},
	{"癸莱",		builtin_map, 1, map_doc},
	{"max",		builtin_max, 1, max_doc},
	{"程",		builtin_max, 1, max_doc},
	{"min",		builtin_min, 1, min_doc},
	{"程",		builtin_min, 1, min_doc},
	{"oct",		builtin_oct, 1, oct_doc},
	{"秈",		builtin_oct, 1, oct_doc},
	{"open",	builtin_open, 1, open_doc},
	{"ゴ秨",	builtin_open, 1, open_doc},
	{"ord",		builtin_ord, 1, ord_doc},
	{"絏",		builtin_ord, 1, ord_doc},
	{"pow",		builtin_pow, 1, pow_doc},
	{"―经",		builtin_pow, 1, pow_doc},
	{"range",	builtin_range, 1, range_doc},
	{"絛瞅",	builtin_range, 1, range_doc},
	{"raw_input",	builtin_raw_input, 1, raw_input_doc},
	{"弄",	builtin_raw_input, 1, raw_input_doc},
	{"reduce",	builtin_reduce, 1, reduce_doc},
	{"て虏",	builtin_reduce, 1, reduce_doc},
	{"reload",	builtin_reload, 1, reload_doc},
	{"更",	builtin_reload, 1, reload_doc},
	{"repr",	builtin_repr, 1, repr_doc},
	{"ゅボ",	builtin_repr, 1, repr_doc},
	{"round",	builtin_round, 1, round_doc},
	{"",	builtin_round, 1, round_doc},
	{"setattr",	builtin_setattr, 1, setattr_doc},
	{"砞妮┦",	builtin_setattr, 1, setattr_doc},
	{"slice",       builtin_slice, 1, slice_doc},
	{"ち",       builtin_slice, 1, slice_doc},
	{"str",		builtin_str, 1, str_doc},
	{"跑﹃",		builtin_str, 1, str_doc},
	{"tuple",	builtin_tuple, 1, tuple_doc},
	{"跑じ舱",	builtin_tuple, 1, tuple_doc},
	{"type",	builtin_type, 1, type_doc},
	{"摸",	builtin_type, 1, type_doc},
	{"unicode",	builtin_unicode, 1, unicode_doc},
	{"参絏",	builtin_unicode, 1, unicode_doc},
	{"unichr",	builtin_unichr, 1, unichr_doc},
	{"参絏",	builtin_unichr, 1, unichr_doc},
	{"vars",	builtin_vars, 1, vars_doc},
	{"跑计",	builtin_vars, 1, vars_doc},
	{"xrange",	builtin_xrange, 1, xrange_doc},
	{"虏絛瞅",	builtin_xrange, 1, xrange_doc},
 	{"zip",         builtin_zip, 1, zip_doc},
 	{"癸",         builtin_zip, 1, zip_doc},
	{NULL,		NULL},
};

static char builtin_doc[] =
/*"Built-in functions, exceptions, and other objects.\n\
\n\
Noteworthy: None is the `nil' object; Ellipsis represents `...' in slices.";*/
"ずㄧ计, 钵盽摸のㄤ龟砰.\n\
\n\
猔種: 礚/None  `ぐ或ぃ琌' 龟砰; 菠腹/Ellipsis 玥ちい `...'.";

PyObject *
_PyBuiltin_Init(void)
{
	PyObject *mod, *dict, *debug;
	mod = Py_InitModule4("__builtin__", builtin_methods,
			     builtin_doc, (PyObject *)NULL,
			     PYTHON_API_VERSION);
	if (mod == NULL)
		return NULL;
	dict = PyModule_GetDict(mod);
	if (PyDict_SetItemString(dict, "None", Py_None) < 0)
		return NULL;
	if (PyDict_SetItemString(dict, "礚", Py_None) < 0)
		return NULL;
	if (PyDict_SetItemString(dict, "Ellipsis", Py_Ellipsis) < 0)
		return NULL;
	if (PyDict_SetItemString(dict, "菠腹", Py_Ellipsis) < 0)
		return NULL;
	if (PyDict_SetItemString(dict, "NotImplemented",
				 Py_NotImplemented) < 0)
		return NULL;
	if (PyDict_SetItemString(dict, "﹟ゼЧ",
				 Py_NotImplemented) < 0)
		return NULL;
	debug = PyInt_FromLong(Py_OptimizeFlag == 0);
	if (PyDict_SetItemString(dict, "__debug__", debug) < 0) {
		Py_XDECREF(debug);
		return NULL;
	}
	if (PyDict_SetItemString(dict, "__埃岿__", debug) < 0) {
		Py_XDECREF(debug);
		return NULL;
	}
	Py_XDECREF(debug);

	return mod;
}

/* Helper for filter(): filter a tuple through a function */

static PyObject *
filtertuple(PyObject *func, PyObject *tuple)
{
	PyObject *result;
	register int i, j;
	int len = PyTuple_Size(tuple);

	if (len == 0) {
		Py_INCREF(tuple);
		return tuple;
	}

	if ((result = PyTuple_New(len)) == NULL)
		return NULL;

	for (i = j = 0; i < len; ++i) {
		PyObject *item, *good;
		int ok;

		if ((item = PyTuple_GetItem(tuple, i)) == NULL)
			goto Fail_1;
		if (func == Py_None) {
			Py_INCREF(item);
			good = item;
		}
		else {
			PyObject *arg = Py_BuildValue("(O)", item);
			if (arg == NULL)
				goto Fail_1;
			good = PyEval_CallObject(func, arg);
			Py_DECREF(arg);
			if (good == NULL)
				goto Fail_1;
		}
		ok = PyObject_IsTrue(good);
		Py_DECREF(good);
		if (ok) {
			Py_INCREF(item);
			if (PyTuple_SetItem(result, j++, item) < 0)
				goto Fail_1;
		}
	}

	if (_PyTuple_Resize(&result, j, 0) < 0)
		return NULL;

	return result;

Fail_1:
	Py_DECREF(result);
	return NULL;
}


/* Helper for filter(): filter a string through a function */

static PyObject *
filterstring(PyObject *func, PyObject *strobj)
{
	PyObject *result;
	register int i, j;
	int len = PyString_Size(strobj);

	if (func == Py_None) {
		/* No character is ever false -- share input string */
		Py_INCREF(strobj);
		return strobj;
	}
	if ((result = PyString_FromStringAndSizeAndEncode(NULL, len, 
					Source_Encoding)) == NULL)
		return NULL;

	for (i = j = 0; i < len; ++i) {
		PyObject *item, *arg, *good;
		int ok;

		item = (*strobj->ob_type->tp_as_sequence->sq_item)(strobj, i);
		if (item == NULL)
			goto Fail_1;
		arg = Py_BuildValue("(O)", item);
		if (arg == NULL) {
			Py_DECREF(item);
			goto Fail_1;
		}
		good = PyEval_CallObject(func, arg);
		Py_DECREF(arg);
		if (good == NULL) {
			Py_DECREF(item);
			goto Fail_1;
		}
		ok = PyObject_IsTrue(good);
		Py_DECREF(good);
		if (ok)
			PyString_AS_STRING((PyStringObject *)result)[j++] =
				PyString_AS_STRING((PyStringObject *)item)[0];
		Py_DECREF(item);
	}

	if (j < len && _PyString_Resize(&result, j) < 0)
		return NULL;

	return result;

Fail_1:
	Py_DECREF(result);
	return NULL;
}
