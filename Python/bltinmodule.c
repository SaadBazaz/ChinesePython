
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
"__載入__(名稱, 共用變數, 私有變數, 載入名稱序列) -> 模組\n\
\n\
載入模組.  共用變數參數只是用來決定運算域而內容不會被改變;\n\
私有變數參數暫時沒有用到. 第 3 個參數是一個序列, \n\
用來模擬 ``從 名稱 載入 ..../from name import ...'',\n\
如果是空的序列則表示 ``載入 名稱/import name''.\n\
從套件中載入模組時, 注意 __載入__('A.B', ...)\n\
當名稱序列參數是空序列時會傳回套件 A , 當該參數\n\
不是空的序列時則傳回 B.";


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
"絕對值(數字) -> 數字\n\
\n\
傳回數字的絕對值 (去掉正負號的數值).";


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
				     "apply/套用() 第二個參數應是序列,但收到的是 %s",
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
			     "apply/套用() 第三個參數應是字典,但收到的是 %s",
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
"套用(實體[, 參數[, 字典類參數]]) -> 值\n\
\n\
調用一個可呼叫的參數 '實體', 其調用的參數取自 '參數',\n\
或 '字典類參數'.\n\
注意可在定義實體時加入  __call__/__呼叫__() 方法來實體變成可被調用.";


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
"暫存(實體 [, 偏移值[, 大小]]) -> 實體\n\
\n\
根據給出的 '實體' 參數產生一個新的緩衝暫存實體.\n\
該暫存會代表目標實體的一個切片, 從實體的開始位置算起,\n\
(如果有給出 '移植值' 則從該位置開始算)\n\
該切片類會一直到 '實體' 的末端. \n\
(除非在參數 '大小' 中指定了切片的長度).";


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
"統一碼(字串 [, 編碼[, 錯誤處理]]) -> 實體\n\
\n\
根據參數來產生一個統一碼編碼的字串類.\n\
編碼的預設值是系統的預設編碼.\n\
至於錯誤處理選項則為 'strict'.";


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
"可呼叫(實體) -> 真值\n\
\n\
如實體是可被當作函數來調用的話則傳回真. \n\
注意如果實體的定義中有 __call__/呼叫() 這個特殊方法的話則該實體會成為可呼叫的.";


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
			   "filter/過濾() 第二個參數應是序列");
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
"過濾(函數, 序列) -> 序列\n\
\n\
參數 '函數' 取一個參數並傳回真值. 過濾後的序列會包括\n\
參數序列中使 '函數' 傳回真的元素.";


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
				"chr/字元() 的參數超出了 範圍(256)");
		return NULL;
	}
	s[0] = (char)x;
	return PyString_FromStringAndSizeAndEncode(s, 1, Current_Encoding);
}

static char chr_doc[] =
/*"chr(i) -> character\n\
\n\
Return a string of one character with ordinal i; 0 <= i < 256.";*/
"字元(i) -> 字元符\n\
\n\
傳回一個字元的字串, 其序值為 i; 0 <= i < 256.";


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
				"unichr/統一碼字() 的參數超出了 範圍(65536)");
		return NULL;
	}
	s[0] = (Py_UNICODE)x;
	return PyUnicode_FromUnicode(s, 1);
}

static char unichr_doc[] =
/*"unichr(i) -> Unicode character\n\
\n\
Return a Unicode string of one character with ordinal i; 0 <= i < 65536.";*/
"統一碼字(i) -> 統一碼字符\n\
\n\
傳回一個字元的統一碼字串. 其序值為 i; 0 <= i < 65536.";


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
"比較(甲, 乙) -> 數字\n\
\n\
如 甲 < 乙 傳回負值, 甲乙相等傳回 0 , 甲 > 乙 則傳回正值.";


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
"同化(甲, 乙) -> 無 或 (甲1, 乙1)\n\
\n\
如 '甲' '乙' 可以轉換成同一類型的數據則傳回元素組, 其中包括轉換後的值.\n\
如不能轉換成同一類型則傳回 '無'/None.";


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
	    strcmp(startstr, "執行") == 0)
		start = Py_file_input;
	else if (strcmp(startstr, "eval") == 0 ||
		 strcmp(startstr, "演算") == 0)
		start = Py_eval_input;
	else if (strcmp(startstr, "single") == 0 ||
		 strcmp(startstr, "單行") == 0)
		start = Py_single_input;
	else {
		PyErr_SetString(PyExc_ValueError,
		   /*"compile() arg 3 must be 'exec' or 'eval' or 'single'");*/
		   "compile/編譯() 第三個參數須為 'exec'/'執行','eval'/'演算','single'/'單行'");
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
"編譯(程式碼, 檔名, 模式) -> 碼類型實體\n\
\n\
編譯程式碼(字串型態), 可以是中蟒的模組, 表達式或語句.\n\
傳回的結果為一個碼類別, 可以作為 執行() 或者 推算() 的調用參數.\n\
'檔名'的作用是用來放錯誤訊息的.\n\
參數 '模式' 須為以下值: 如果是模組則用 'exec/執行', \n\
如是一個指令語句則用 'single/單行',\n\
如是一個表達式則用 'eval/推算'.";


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
				 "complex/複數() 太大,無法轉換");
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
				"complex/複數() 的參數應該是字串");
		return NULL;
	}

	/* position on first nonblank */
	start = s;
	while (*s && isspace(Py_CHARMASK(*s)))
		s++;
	if (s[0] == '\0') {
		PyErr_SetString(PyExc_ValueError,
				/*"complex() arg is an empty string");*/
				"complex/複數() 的參數不能是空白字串");
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
					"complex/複數() 參數中有個 null 字符");
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
					  "float/浮點數() 超出範圍: %.150s", s);
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
				"complex/複數() 參數字串的格式不對");
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
			   "complex/複數() 的參數無法轉換成複數型態");
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
			complexstr_cp = PyString_InternFromString("__複數__");
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
					"float/浮點數(r) 傳回值並不浮點數");
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
"複數(實部[, 虛部]) -> 複數\n\
\n\
根據參數的實部和虛部產生一個虛數數據.\n\
相當於 (實部 + 虛部*1j). 其中虛部的預設值為 0.";


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
"內容([實體]) -> 字串序列 \n\
\n\
和 總內容([相同]) 但傳回的只是中文部分.";

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
"總內容([實體]) -> 字串序列\n\
\n\
傳回字串序列. 該序列中包含參數實體中 (但不是全部) 的屬性.\n\
如果沒有給出參數則傳回目前運算域中定義的變數,名稱.\n\
如果參數是實體的話則只傳回該實體中的屬性而不包括其中的方法.\n\
如果參數是類別的話則傳回值中不包括其父概念中的屬性.\n\
對於其他參數則可能傳回方法或屬性等的值.";


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
"商餘數(x, y) -> (商, 餘數)\n\
\n\
傳回元素組, 內容為 ((x-x%y)/y, x%y).  注意有這個固定關係: div*y + mod == x.";


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
			   "eval/演算() 第一個參數須為字串或程式碼類型");
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
"推算(程式碼[, 共用變數[, 私有變數]]) -> 值\n\
\n\
共用變數和私有變數為進行推算時所用的運算域, 為字典型態.\n\
程式碼可以是中蟒的式碼 (字串形式) 或是用 \n\
編譯/compile() 傳回的碼類型數據.\n\
運算域的預設值是目前的運算域, 如果只給出共用變數則私有變數\n\
會設成和它一樣.";


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
		if (PyDict_SetItemString(globals, "__內建__",
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
"執行檔案(檔名[, 共用變數[, 私有變數]])\n\
\n\
讀入並執行檔案中的程式內容.\n\
共用變數和私有變數為字典類變數, 預設的值\n\
就是目前運算域中的值.  如果只給出共用變數的話則\n\
私有變數設成和它一樣.";


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
"讀屬性(實體, 名稱[, 預設值]) -> 值\n\
\n\
在參數實體中讀取 '名稱' 屬性; 讀屬性(x, 'y') 相當於 x.y\n\
預設值代表了如果找不到該項屬性的傳回值.\n\
如沒有 '預設值' 這個參數則找不到該屬性會引發異常.";


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
"共用變數() -> 字典\n\
\n\
傳回目前的運算域中的共用變數表.";


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
"有屬性(實體, 名字) -> 真值\n\
\n\
如參數實體中有 '名字' 這個屬性則傳回真, 否則傳回假.\n\
(做法是調用 讀屬性(實體, 名字) 然後檢查有沒有引發異常."; 


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
"代號(實體) -> 整數\n\
\n\
傳回參數實體的身份號碼.  這個代號保證對於同時存在的實體類也是獨一無二的.\n\
(提示: 它基本上代表了該實體的記憶體位址, 因此相同代號的實體必定是指同一個實體)";


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
				"map/對應() 需要起碼兩個參數");
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
			    "map/對應() 的參數 %d 須為序列型態";
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
"對應(函數, 序列[, 序列, ...]) -> 序列\n\
\n\
調用函數 '函數', 調用時的參數取自 '序列'.\n\
如果給出多個序列則會依次調用 '函數', 並把所有的函數傳回值順序放到 '序列'中.\n\
如果序列的長度不一致則會以 '無'/'None' 來填補.\n\
如果以 '無'/'None' 作為 '函數', 則傳回 '序列', \n\
或者當有超過個序列時傳回一個包含元素組的序列. (哇, 連我自己也有點糊塗了.)";


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
"設屬性(實體, 名稱, 值)\n\
\n\
在參數實體中加入一個屬性; 設屬性(x, 'y', v) \n\
等同於 ``x.y = v''.";


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
"刪屬性(實體, 名稱)\n\
\n\
刪除實體中 '名稱' 這個屬性; 刪屬性(x, 'y') \n\
等同於 ``del x.y''.";


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
"拼揍值(實體) -> 整數\n\
\n\
傳回實體的拼揍值. 相同的實體有相同的拼揍值.\n\
兩個實體有相同的拼揍值很可能表示它們是相同的, 但並不保證一定相同.";


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
			   "hex/十六進() 的參數無法轉換成十六進位數");
		return NULL;
	}
	return (*nb->nb_hex)(v);
}

static char hex_doc[] =
/*"hex(number) -> string\n\
\n\
Return the hexadecimal representation of an integer or long integer.";*/
"十六進(數字) -> 字串\n\
\n\
傳回以十六進制表示的數字值";


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
		if (PyDict_SetItemString(globals, "__內建__",
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
"推算讀入([提示句]) -> 值\n\
\n\
相當於 推算(讀入(提示句)) / eval(raw_input(prompt)).";


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
"內存(字串) -> 字串\n\
\n\
``Intern/內存代'' 給出的參數字串.  這樣做會把該字串加到 (共用的)\n\
內存化字串表之中, 做字典索引的查找時速度會快一些.\n\
傳回字串本身, 如果在前面的操作已經內存化了相同的字串則傳回先前的值.";


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
				"int/整數() 不能把非字串型態的資料轉換成指定數系的值");
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
"整數(x) -> 整數\n\
整數(x, 基底) -> 整數\n\
\n\
把字串或數字轉換成整數型態.\n\
浮點型的數字會換成整數型. (注意這不包括字串型態表示的浮點數.\n\
如果轉換的是字串, 則會以給出的 '基底' 作為轉換的數系.\n\
如果轉換的不是字串, 則不能用 '基底' 參數了.";


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
				"long/大整數() 不能把非字串型態的資料轉換成指定數系的值");
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
"大整數(x) -> 大整數\n\
大整數(x, 基底) -> 大整數\n\
\n\
把字串或數字轉換成大整數型態.\n\
浮點型的數字會換成整數型. (注意這不包括字串型態表示的浮點數.\n\
如果轉換的是字串, 則會以給出的 '基底' 作為轉換的數系.\n\
如果轉換的不是字串, 則不能用 '基底' 參數了.";


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
"浮點數(x) -> 浮點數\n\
\n\
可能的話, 把參數字串或數目字變成浮點數.";


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
"長度(實體) -> 整數\n\
\n\
傳回序列或字典類中的元素數目, 字串的長度以及有定義長度的實體的大小.";


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
"變序列(序列類) -> 序列\n\
\n\
傳回參數 '序列類' 變成序列後的值.";


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
"切片([始,] 終[, 步進]) -> 切片類\n\
\n\
產生一個切片類實體. 這可以給 Numeric extensions(數值擴展模組) 用的.";


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
"私有變數() -> 字典\n\
\n\
傳回目前運算域中的本地變數表.";


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
				"min/最小(), max/最大() 的參數須為序列類型");
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
				"min/最小(), max/最大() 的參數不能是空的序列");
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
"最小(序列) -> 值\n\
最小(a, b, c, ...) -> 值\n\
\n\
只有一個序列參數下, 給出序列中最小的元素.\n\
如果有兩個或以上參數則傳回所有參數中的最大值.";


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
"最大(序列) -> 值\n\
最大(a, b, c, ...) -> 值\n\
\n\
如果只有一個序列參數, 傳回該參數中的最大元素.\n\
如果超過一個參數, 傳回所有參數中最大的一個.";


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
			   "oct/八進() 參數不能轉換成八進位數值");
		return NULL;
	}
	return (*nb->nb_oct)(v);
}

static char oct_doc[] =
/*"oct(number) -> string\n\
\n\
Return the octal representation of an integer or long integer.";*/
"八進(數字) -> 字串\n\
\n\
傳回整數參數的八進制表示式.";


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
"打開(檔名[, 模式[, 緩衝]]) -> 檔案類 \n\
\n\
打開一個檔案.  模式可以是 'r', 'w', 'a' 代表讀入 (預設值),\n\
寫出, 以及 附加.  在寫出及附加的模式下如果沒有該檔案則會產生一個新的檔案.\n\
假如已經有同名檔案存在那在寫出模式下舊檔案會被覆蓋掉.\n\
要讀二進位檔時請加上一個 'b' 字. 要同時讀寫則可以加上 '+' 符號.\n\
'緩衝' 值: 0 代表不用暫存, 1 代表暫存一行內容, \n\
大於一的數表示該緩衝的大小.";


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
			     "ord/字碼() 的參數應是長度為 1 的字串, 但收到 " \
			     "%.200s", obj->ob_type->tp_name);
		return NULL;
	}

	PyErr_Format(PyExc_TypeError,
		     /*"ord() expected a character, "
		     "but string of length %d found",*/
		     "ord/字碼() 的參數應為單個字符, "
		     "但收到長 %d 的字串",
		     size);
	return NULL;
}

static char ord_doc[] =
/*"ord(c) -> integer\n\
\n\
Return the integer ordinal of a one-character string.";*/
"字碼(c) -> 整數\n\
\n\
傳回單字元字串的字碼. (小於 127 的則是該字元的 ASCII 碼)";


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
"冪(x, y[, z]) -> 數字\n\
\n\
兩個參數的情況下相當於 x**y.  三個參數的情況下,\n\
相當於 (x**y) % z, 在數字相當大的時會較 (x**y) % z 更高效.";


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
				"l;range/範圍() 需要 1 到 3 個整數型參數",
				&ihigh))
			return NULL;
	}
	else {
		if (!PyArg_ParseTuple(args,
				/*"ll|l;range() requires 1-3 int arguments",*/
				"ll|l;range/範圍() 需要 1 到 3 個整數型參數",
				&ilow, &ihigh, &istep))
			return NULL;
	}
	if (istep == 0) {
		/*PyErr_SetString(PyExc_ValueError, "range() arg 3 must not be zero");*/
		PyErr_SetString(PyExc_ValueError, "range/範圍() 第三個參數不能是 0 ");
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
				"range/範圍() 傳回的序列元素數目太多");
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
"範圍([始,] 終[, 步進]) -> 整數序列\n\
\n\
傳回一個序列, 該序列中包含順序排列的算術級數列.\n\
範圍(甲, 乙) 表示 [甲, 甲+1, 甲+2, ..., 乙-1]; '始' (!) 的值預設為 0.\n\
如果給出參數 '步進', 則表示序列中相鄰數的差.\n\
例子: 範圍(4) 表示 [0, 1, 2, 3].  注意範圍中並不包括 '4'!";

static PyObject *
builtin_xrange(PyObject *self, PyObject *args)
{
	long ilow = 0, ihigh = 0, istep = 1;
	long n;

	if (PyTuple_Size(args) <= 1) {
		if (!PyArg_ParseTuple(args,
				/*"l;xrange() requires 1-3 int arguments",*/
				"l;xrange/簡範圍() 需要 1 到 3 個整數型參數",
				&ihigh))
			return NULL;
	}
	else {
		if (!PyArg_ParseTuple(args,
				/*"ll|l;xrange() requires 1-3 int arguments",*/
				"ll|l;xrange/簡範圍() 需要 1 到 3 個整數型參數",
				&ilow, &ihigh, &istep))
			return NULL;
	}
	if (istep == 0) {
		/*PyErr_SetString(PyExc_ValueError, "xrange() arg 3 must not be zero");*/
		PyErr_SetString(PyExc_ValueError, "xrange/簡範圍() 第三個參數不能是 0 ");
		return NULL;
	}
	if (istep > 0)
		n = get_len_of_range(ilow, ihigh, istep);
	else
		n = get_len_of_range(ihigh, ilow, -istep);
	if (n < 0) {
		PyErr_SetString(PyExc_OverflowError,
				/*"xrange() result has too many items");*/
				"xrange/簡範圍() 傳回序列的元素數目太多");
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
"簡範圍([始,] 終[, 步進]) -> 簡範圍類實體\n\
\n\
和 範圍() 相似, 但傳回的不是一個序列而是一個實體, \n\
該實體會依需要而產生相對應的範圍中的數字. \n\
和 範圍() 相較要慢一些但是節省不少記憶體.";


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
				PyErr_SetString(PyExc_OverflowError, "輸入值太長");
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
			PyErr_SetString(PyExc_RuntimeError, "sys.stdout/系統.標準輸出 弄丟了");
			return NULL;
		}
		if (Py_FlushLine() != 0 ||
		    PyFile_WriteObject(v, f, Py_PRINT_RAW) != 0)
			return NULL;
	}
	f = PySys_GetObject("stdin");
	if (f == NULL) {
		/*PyErr_SetString(PyExc_RuntimeError, "lost sys.stdin");*/
		PyErr_SetString(PyExc_RuntimeError, "sys.stdin/系統.標準輸入 弄丟了");
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
"讀入([提示句]) -> 字\n\
\n\
從標準輸入裝置讀取一個字串. 結尾的換行符號會被裁去. \n\
如果用戶在輸入時鍵入 EOF 記號 (Unix: Ctl-D, Windows: Ctl-Z+Return), \n\
則引發 檔案終結異常/EOFError.\n\
在 Unix, 如果可能的話會使用 GNU readline. '提示句' 會先顯示出來,\n\
然後再另起一行才進行讀取操作.";


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
		    "reduce/化簡() 第二個參數須是序列");
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
			   "reduce/化簡() 空的序列但又沒給出預設值");

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
"化簡(函數, 序列[, 起始值]) -> 值\n\
\n\
參數 '函數' 是一個讀取兩個參數並傳回一個值的操作. 該函數會\n\
累積地從左到右作用在 '序列' 上並傳回最後的結果值.\n\
例如: 化簡(lambda x, y: x+y, [1, 2, 3, 4, 5]) 會計算\n\
((((1+2)+3)+4)+5).  如果給出參數 '起始值', 在計算時該值會加到序列中作為\n\
第一個元素, 並且在序列是空的情況下作為預計值.";


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
"重載(模組) -> 模組\n\
\n\
重新加載一個已載入的模組. 模組必須在早前成功\載入過.";


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
"以文字表示(實體) -> 字串\n\
\n\
傳回參數 '實體' 正規的字串表示.\n\
對大部分實體來說 推算(以文字表示(實體)) == 實體 / eval(repr(object)) == object.";


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
"四捨(數字[, 小數位]) -> 浮點數\n\
\n\
傳回參數 '數字' 四捨五入到 '小數位'. (預設的小數位為 0).\n\
傳回的值一定是浮點數. 精確度可以為負數";


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
"變字串(實體) -> 字串\n\
\n\
以字串的型態表示參數物.\n\
如果參數本來就是字串類則傳回參數本身.";


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
"變元素組(序列類) -> 序列\n\
\n\
傳回元素組, 其中元素和參數序列中的一樣.\n\
如果參數本身已是元素組,則傳回參數本身.";


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
"類型(實體) -> 類型\n\
\n\
傳回實體的類型.";


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
						"沒有私有變數!?");
		}
		else
			Py_INCREF(d);
	}
	else {
		d = PyObject_GetAttrString(v, "__dict__");
		if (d == NULL) {
			PyErr_SetString(PyExc_TypeError,
			    /*"vars() argument must have __dict__ attribute");*/
			    "vars/變收表() 的參數應要有 __dict__/__字典__ 這個屬性");
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
"變數表([實體]) -> 字典\n\
\n\
沒有參數下, 等於 locals/私有變數().\n\
有參數下, 等於 實體.__字典__/object.__dict__";

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
"是實體(實體, 概念或類型) -> 真值\n\
\n\
如 '實體' 是某概念或其引申概念的實現則傳回真.\n\
如給出第 2  個參數, 則傳回 '實體' 是否和該參數(實體或數據類)類型相同.";


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
"是子概念(C, B) -> 真或假\n\
\n\
如 C 是 B 概念的子概念(繼承關係)則傳回 1, 否則傳回 0.";


static PyObject*
builtin_zip(PyObject *self, PyObject *args)
{
	PyObject *ret;
	int itemsize = PySequence_Length(args);
	int i, j;

	if (itemsize < 1) {
		PyErr_SetString(PyExc_TypeError,
				/*"zip() requires at least one sequence");*/
				"zip/拼對() 的參數起碼要有一個序列");
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
"拼對(序列1 [, 序列2 [...]]) -> [(序列1[0], 序列2[0] ...), (...)]\n\
\n\
傳回一個元素組的列表, 序列中第 i 個元素組含有所有參數序列的第 i 個元素.\n\
傳回的序列的長度為參數序列中最短的為準.";


static PyMethodDef builtin_methods[] = {
	{"__import__",	builtin___import__, 1, import_doc},
	{"__載入__",	builtin___import__, 1, import_doc},
	{"abs",		builtin_abs, 1, abs_doc},
	{"絕對值",		builtin_abs, 1, abs_doc},
	{"apply",	builtin_apply, 1, apply_doc},
	{"套用",	builtin_apply, 1, apply_doc},
	{"buffer",	builtin_buffer, 1, buffer_doc},
	{"暫存",	builtin_buffer, 1, buffer_doc},
	{"callable",	builtin_callable, 1, callable_doc},
	{"可呼叫",	builtin_callable, 1, callable_doc},
	{"chr",		builtin_chr, 1, chr_doc},
	{"字元",		builtin_chr, 1, chr_doc},
	{"cmp",		builtin_cmp, 1, cmp_doc},
	{"比較",		builtin_cmp, 1, cmp_doc},
	{"coerce",	builtin_coerce, 1, coerce_doc},
	{"同化",	builtin_coerce, 1, coerce_doc},
	{"compile",	builtin_compile, 1, compile_doc},
	{"編譯",	builtin_compile, 1, compile_doc},
#ifndef WITHOUT_COMPLEX
	{"complex",	builtin_complex, 1, complex_doc},
	{"複數",	builtin_complex, 1, complex_doc},
#endif
	{"delattr",	builtin_delattr, 1, delattr_doc},
	{"刪屬性",	builtin_delattr, 1, delattr_doc},
	{"dir",		builtin_dir, 1, dir_doc},
	{"總內容",	builtin_dir, 1, dir_doc},
	{"cdir",	builtin_dir_cp, 1, dir_cp_doc},
	{"內容",	builtin_dir_cp, 1, dir_cp_doc},
	{"divmod",	builtin_divmod, 1, divmod_doc},
	{"商餘數",	builtin_divmod, 1, divmod_doc},
	{"eval",	builtin_eval, 1, eval_doc},
	{"推算",	builtin_eval, 1, eval_doc},
	{"execfile",	builtin_execfile, 1, execfile_doc},
	{"執行文件",	builtin_execfile, 1, execfile_doc},
	{"filter",	builtin_filter, 1, filter_doc},
	{"過濾",	builtin_filter, 1, filter_doc},
	{"float",	builtin_float, 1, float_doc},
	{"浮點數",	builtin_float, 1, float_doc},
	{"getattr",	builtin_getattr, 1, getattr_doc},
	{"讀屬性",	builtin_getattr, 1, getattr_doc},
	{"globals",	builtin_globals, 1, globals_doc},
	{"共用變數",	builtin_globals, 1, globals_doc},
	{"hasattr",	builtin_hasattr, 1, hasattr_doc},
	{"有屬性",	builtin_hasattr, 1, hasattr_doc},
	{"hash",	builtin_hash, 1, hash_doc},
	{"拼揍值",	builtin_hash, 1, hash_doc},
	{"hex",		builtin_hex, 1, hex_doc},
	{"十六進",		builtin_hex, 1, hex_doc},
	{"id",		builtin_id, 1, id_doc},
	{"代號",		builtin_id, 1, id_doc},
	{"input",	builtin_input, 1, input_doc},
	{"推算讀入",	builtin_input, 1, input_doc},
	{"intern",	builtin_intern, 1, intern_doc},
	{"內存",	builtin_intern, 1, intern_doc},
	{"int",		builtin_int, 1, int_doc},
	{"整數",		builtin_int, 1, int_doc},
	{"isinstance",  builtin_isinstance, 1, isinstance_doc},
	{"是實體",  builtin_isinstance, 1, isinstance_doc},
	{"issubclass",  builtin_issubclass, 1, issubclass_doc},
	{"是子概念",  builtin_issubclass, 1, issubclass_doc},
	{"len",		builtin_len, 1, len_doc},
	{"長度",		builtin_len, 1, len_doc},
	{"list",	builtin_list, 1, list_doc},
	{"變序列",	builtin_list, 1, list_doc},
	{"locals",	builtin_locals, 1, locals_doc},
	{"私有變數",	builtin_locals, 1, locals_doc},
	{"long",	builtin_long, 1, long_doc},
	{"大整數",	builtin_long, 1, long_doc},
	{"map",		builtin_map, 1, map_doc},
	{"對應",		builtin_map, 1, map_doc},
	{"max",		builtin_max, 1, max_doc},
	{"最大",		builtin_max, 1, max_doc},
	{"min",		builtin_min, 1, min_doc},
	{"最小",		builtin_min, 1, min_doc},
	{"oct",		builtin_oct, 1, oct_doc},
	{"八進",		builtin_oct, 1, oct_doc},
	{"open",	builtin_open, 1, open_doc},
	{"打開",	builtin_open, 1, open_doc},
	{"ord",		builtin_ord, 1, ord_doc},
	{"字碼",		builtin_ord, 1, ord_doc},
	{"pow",		builtin_pow, 1, pow_doc},
	{"求冪",		builtin_pow, 1, pow_doc},
	{"range",	builtin_range, 1, range_doc},
	{"範圍",	builtin_range, 1, range_doc},
	{"raw_input",	builtin_raw_input, 1, raw_input_doc},
	{"讀入",	builtin_raw_input, 1, raw_input_doc},
	{"reduce",	builtin_reduce, 1, reduce_doc},
	{"化簡",	builtin_reduce, 1, reduce_doc},
	{"reload",	builtin_reload, 1, reload_doc},
	{"重載",	builtin_reload, 1, reload_doc},
	{"repr",	builtin_repr, 1, repr_doc},
	{"以文字表示",	builtin_repr, 1, repr_doc},
	{"round",	builtin_round, 1, round_doc},
	{"四舍",	builtin_round, 1, round_doc},
	{"setattr",	builtin_setattr, 1, setattr_doc},
	{"設屬性",	builtin_setattr, 1, setattr_doc},
	{"slice",       builtin_slice, 1, slice_doc},
	{"切片",       builtin_slice, 1, slice_doc},
	{"str",		builtin_str, 1, str_doc},
	{"變字串",		builtin_str, 1, str_doc},
	{"tuple",	builtin_tuple, 1, tuple_doc},
	{"變元素組",	builtin_tuple, 1, tuple_doc},
	{"type",	builtin_type, 1, type_doc},
	{"類型",	builtin_type, 1, type_doc},
	{"unicode",	builtin_unicode, 1, unicode_doc},
	{"統一碼",	builtin_unicode, 1, unicode_doc},
	{"unichr",	builtin_unichr, 1, unichr_doc},
	{"統一碼字",	builtin_unichr, 1, unichr_doc},
	{"vars",	builtin_vars, 1, vars_doc},
	{"變數表",	builtin_vars, 1, vars_doc},
	{"xrange",	builtin_xrange, 1, xrange_doc},
	{"簡範圍",	builtin_xrange, 1, xrange_doc},
 	{"zip",         builtin_zip, 1, zip_doc},
 	{"拼對",         builtin_zip, 1, zip_doc},
	{NULL,		NULL},
};

static char builtin_doc[] =
/*"Built-in functions, exceptions, and other objects.\n\
\n\
Noteworthy: None is the `nil' object; Ellipsis represents `...' in slices.";*/
"內建函數, 異常類以及其他的實體.\n\
\n\
注意: 無/None 代表 `什麼也不是' 實體; 省略號/Ellipsis 則代表切片中的 `...'.";

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
	if (PyDict_SetItemString(dict, "無", Py_None) < 0)
		return NULL;
	if (PyDict_SetItemString(dict, "Ellipsis", Py_Ellipsis) < 0)
		return NULL;
	if (PyDict_SetItemString(dict, "省略號", Py_Ellipsis) < 0)
		return NULL;
	if (PyDict_SetItemString(dict, "NotImplemented",
				 Py_NotImplemented) < 0)
		return NULL;
	if (PyDict_SetItemString(dict, "尚未完工",
				 Py_NotImplemented) < 0)
		return NULL;
	debug = PyInt_FromLong(Py_OptimizeFlag == 0);
	if (PyDict_SetItemString(dict, "__debug__", debug) < 0) {
		Py_XDECREF(debug);
		return NULL;
	}
	if (PyDict_SetItemString(dict, "__除錯__", debug) < 0) {
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
