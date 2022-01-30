
/* Error handling */

#include "Python.h"

#ifdef macintosh
extern char *PyMac_StrError(int);
#undef strerror
#define strerror PyMac_StrError
#endif /* macintosh */

#ifndef __STDC__
#ifndef MS_WINDOWS
extern char *strerror(int);
#endif
#endif

#ifdef MS_WIN32
#include "windows.h"
#include "winbase.h"
#endif

#include <ctype.h>

void
PyErr_Restore(PyObject *type, PyObject *value, PyObject *traceback)
{
	PyThreadState *tstate = PyThreadState_GET();
	PyObject *oldtype, *oldvalue, *oldtraceback;

	if (traceback != NULL && !PyTraceBack_Check(traceback)) {
		/* XXX Should never happen -- fatal error instead? */
		Py_DECREF(traceback);
		traceback = NULL;
	}

	/* Save these in locals to safeguard against recursive
	   invocation through Py_XDECREF */
	oldtype = tstate->curexc_type;
	oldvalue = tstate->curexc_value;
	oldtraceback = tstate->curexc_traceback;

	tstate->curexc_type = type;
	tstate->curexc_value = value;
	tstate->curexc_traceback = traceback;

	Py_XDECREF(oldtype);
	Py_XDECREF(oldvalue);
	Py_XDECREF(oldtraceback);
}

void
PyErr_SetObject(PyObject *exception, PyObject *value)
{
	Py_XINCREF(exception);
	Py_XINCREF(value);
	PyErr_Restore(exception, value, (PyObject *)NULL);
}

void
PyErr_SetNone(PyObject *exception)
{
	PyErr_SetObject(exception, (PyObject *)NULL);
}

void
PyErr_SetString(PyObject *exception, const char *string)
{
	PyObject *value = PyString_FromStringAndEncode(string, Source_Encoding);
	PyErr_SetObject(exception, value);
	Py_XDECREF(value);
}


PyObject *
PyErr_Occurred(void)
{
	PyThreadState *tstate = PyThreadState_Get();

	return tstate->curexc_type;
}


int
PyErr_GivenExceptionMatches(PyObject *err, PyObject *exc)
{
	if (err == NULL || exc == NULL) {
		/* maybe caused by "import exceptions" that failed early on */
		return 0;
	}
	if (PyTuple_Check(exc)) {
		int i, n;
		n = PyTuple_Size(exc);
		for (i = 0; i < n; i++) {
			/* Test recursively */
		     if (PyErr_GivenExceptionMatches(
			     err, PyTuple_GET_ITEM(exc, i)))
		     {
			     return 1;
		     }
		}
		return 0;
	}
	/* err might be an instance, so check its class. */
	if (PyInstance_Check(err))
		err = (PyObject*)((PyInstanceObject*)err)->in_class;

	if (PyClass_Check(err) && PyClass_Check(exc))
		return PyClass_IsSubclass(err, exc);

	return err == exc;
}


int
PyErr_ExceptionMatches(PyObject *exc)
{
	return PyErr_GivenExceptionMatches(PyErr_Occurred(), exc);
}


/* Used in many places to normalize a raised exception, including in
   eval_code2(), do_raise(), and PyErr_Print()
*/
void
PyErr_NormalizeException(PyObject **exc, PyObject **val, PyObject **tb)
{
	PyObject *type = *exc;
	PyObject *value = *val;
	PyObject *inclass = NULL;

	if (type == NULL) {
		/* This is a bug.  Should never happen.  Don't dump core. */
		PyErr_SetString(PyExc_SystemError,
			/*"PyErr_NormalizeException() called without exception");*/
			"PyErr_NormalizeException() 被調用但沒有給出異常型態");
	}

	/* If PyErr_SetNone() was used, the value will have been actually
	   set to NULL.
	*/
	if (!value) {
		value = Py_None;
		Py_INCREF(value);
	}

	if (PyInstance_Check(value))
		inclass = (PyObject*)((PyInstanceObject*)value)->in_class;

	/* Normalize the exception so that if the type is a class, the
	   value will be an instance.
	*/
	if (PyClass_Check(type)) {
		/* if the value was not an instance, or is not an instance
		   whose class is (or is derived from) type, then use the
		   value as an argument to instantiation of the type
		   class.
		*/
		if (!inclass || !PyClass_IsSubclass(inclass, type)) {
			PyObject *args, *res;

			if (value == Py_None)
				args = Py_BuildValue("()");
			else if (PyTuple_Check(value)) {
				Py_INCREF(value);
				args = value;
			}
			else
				args = Py_BuildValue("(O)", value);

			if (args == NULL)
				goto finally;
			res = PyEval_CallObject(type, args);
			Py_DECREF(args);
			if (res == NULL)
				goto finally;
			Py_DECREF(value);
			value = res;
		}
		/* if the class of the instance doesn't exactly match the
		   class of the type, believe the instance
		*/
		else if (inclass != type) {
 			Py_DECREF(type);
			type = inclass;
			Py_INCREF(type);
		}
	}
	*exc = type;
	*val = value;
	return;
finally:
	Py_DECREF(type);
	Py_DECREF(value);
	Py_XDECREF(*tb);
	PyErr_Fetch(exc, val, tb);
	/* normalize recursively */
	PyErr_NormalizeException(exc, val, tb);
}


void
PyErr_Fetch(PyObject **p_type, PyObject **p_value, PyObject **p_traceback)
{
	PyThreadState *tstate = PyThreadState_Get();

	*p_type = tstate->curexc_type;
	*p_value = tstate->curexc_value;
	*p_traceback = tstate->curexc_traceback;

	tstate->curexc_type = NULL;
	tstate->curexc_value = NULL;
	tstate->curexc_traceback = NULL;
}

void
PyErr_Clear(void)
{
	PyErr_Restore(NULL, NULL, NULL);
}

/* Convenience functions to set a type error exception and return 0 */

int
PyErr_BadArgument(void)
{
	PyErr_SetString(PyExc_TypeError,
			/*"bad argument type for built-in operation");*/
			"調用內建函數時用了錯誤的參數");
	return 0;
}

PyObject *
PyErr_NoMemory(void)
{
	if (PyErr_ExceptionMatches(PyExc_MemoryError))
		/* already current */
		return NULL;

	/* raise the pre-allocated instance if it still exists */
	if (PyExc_MemoryErrorInst)
		PyErr_SetObject(PyExc_MemoryError, PyExc_MemoryErrorInst);
	else
		/* this will probably fail since there's no memory and hee,
		   hee, we have to instantiate this class
		*/
		PyErr_SetNone(PyExc_MemoryError);

	return NULL;
}

PyObject *
PyErr_SetFromErrnoWithFilename(PyObject *exc, char *filename)
{
	PyObject *v;
	char *s;
	int i = errno;
#ifdef MS_WIN32
	char *s_buf = NULL;
#endif
#ifdef EINTR
	if (i == EINTR && PyErr_CheckSignals())
		return NULL;
#endif
	if (i == 0)
		/*s = "Error"; *//* Sometimes errno didn't get set */
		s = "錯誤"; 
	else
#ifndef MS_WIN32
		s = strerror(i);
#else
	{
		/* Note that the Win32 errors do not lineup with the
		   errno error.  So if the error is in the MSVC error
		   table, we use it, otherwise we assume it really _is_ 
		   a Win32 error code
		*/
		if (i > 0 && i < _sys_nerr) {
			s = _sys_errlist[i];
		}
		else {
			int len = FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,	/* no message source */
				i,
				MAKELANGID(LANG_NEUTRAL,
					   SUBLANG_DEFAULT),
				           /* Default language */
				(LPTSTR) &s_buf,
				0,	/* size not used */
				NULL);	/* no args */
			s = s_buf;
			/* remove trailing cr/lf and dots */
			while (len > 0 && (s[len-1] <= ' ' || s[len-1] == '.'))
				s[--len] = '\0';
		}
	}
#endif
	if (filename != NULL)
		v = Py_BuildValue("(iss)", i, s, filename);
	else
		v = Py_BuildValue("(is)", i, s);
	if (v != NULL) {
		PyErr_SetObject(exc, v);
		Py_DECREF(v);
	}
#ifdef MS_WIN32
	LocalFree(s_buf);
#endif
	return NULL;
}


PyObject *
PyErr_SetFromErrno(PyObject *exc)
{
	return PyErr_SetFromErrnoWithFilename(exc, NULL);
}

#ifdef MS_WINDOWS 
/* Windows specific error code handling */
PyObject *PyErr_SetFromWindowsErrWithFilename(
	int ierr,
	const char *filename)
{
	int len;
	char *s;
	PyObject *v;
	DWORD err = (DWORD)ierr;
	if (err==0) err = GetLastError();
	len = FormatMessage(
		/* Error API error */
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,	/* no message source */
		err,
		MAKELANGID(LANG_NEUTRAL,
		SUBLANG_DEFAULT), /* Default language */
		(LPTSTR) &s,
		0,	/* size not used */
		NULL);	/* no args */
	/* remove trailing cr/lf and dots */
	while (len > 0 && (s[len-1] <= ' ' || s[len-1] == '.'))
		s[--len] = '\0';
	if (filename != NULL)
		v = Py_BuildValue("(iss)", err, s, filename);
	else
		v = Py_BuildValue("(is)", err, s);
	if (v != NULL) {
		PyErr_SetObject(PyExc_WindowsError, v);
		Py_DECREF(v);
	}
	LocalFree(s);
	return NULL;
}

PyObject *PyErr_SetFromWindowsErr(int ierr)
{
	return PyErr_SetFromWindowsErrWithFilename(ierr, NULL);
}
#endif /* MS_WINDOWS */

void
_PyErr_BadInternalCall(char *filename, int lineno)
{
	PyErr_Format(PyExc_SystemError,
		     /*"%s:%d: bad argument to internal function",*/
		     "%s:%d: 內部函數的調用參數不正確",
		     filename, lineno);
}

/* Remove the preprocessor macro for PyErr_BadInternalCall() so that we can
   export the entry point for existing object code: */
#undef PyErr_BadInternalCall
void
PyErr_BadInternalCall(void)
{
	PyErr_Format(PyExc_SystemError,
		     /*"bad argument to internal function");*/
		     "內部函數的調用參數不正確");
}
#define PyErr_BadInternalCall() _PyErr_BadInternalCall(__FILE__, __LINE__)



PyObject *
PyErr_Format(PyObject *exception, const char *format, ...)
{
	va_list vargs;
	int n, i;
	const char* f;
	char* s;
	PyObject* string;

	/* step 1: figure out how large a buffer we need */

#ifdef HAVE_STDARG_PROTOTYPES
	va_start(vargs, format);
#else
	va_start(vargs);
#endif

	n = 0;
	for (f = format; *f; f++) {
		if (*f == '%') {
			const char* p = f;
			while (*++f && *f != '%' && !isalpha(Py_CHARMASK(*f)))
				;
			switch (*f) {
			case 'c':
				(void) va_arg(vargs, int);
				/* fall through... */
			case '%':
				n++;
				break;
			case 'd': case 'i': case 'x':
				(void) va_arg(vargs, int);
				/* 20 bytes should be enough to hold a 64-bit
				   integer */
				n = n + 20;
				break;
			case 's':
				s = va_arg(vargs, char*);
				n = n + strlen(s);
				break;
			default:
				/* if we stumble upon an unknown
				   formatting code, copy the rest of
				   the format string to the output
				   string. (we cannot just skip the
				   code, since there's no way to know
				   what's in the argument list) */ 
				n = n + strlen(p);
				goto expand;
			}
		} else
			n = n + 1;
	}
	
 expand:
	
	string = PyString_FromStringAndSizeAndEncode(NULL, n, Source_Encoding);
	if (!string)
		return NULL;
	
#ifdef HAVE_STDARG_PROTOTYPES
	va_start(vargs, format);
#else
	va_start(vargs);
#endif

	/* step 2: fill the buffer */

	s = PyString_AsString(string);

	for (f = format; *f; f++) {
		if (*f == '%') {
			const char* p = f++;
			/* parse the width.precision part (we're only
			   interested in the precision value, if any) */
			n = 0;
			while (isdigit(Py_CHARMASK(*f)))
				n = (n*10) + *f++ - '0';
			if (*f == '.') {
				f++;
				n = 0;
				while (isdigit(Py_CHARMASK(*f)))
					n = (n*10) + *f++ - '0';
			}
			while (*f && *f != '%' && !isalpha(Py_CHARMASK(*f)))
				f++;
			switch (*f) {
			case 'c':
				*s++ = va_arg(vargs, int);
				break;
			case 'd': 
				sprintf(s, "%d", va_arg(vargs, int));
				s = s + strlen(s);
				break;
			case 'i':
				sprintf(s, "%i", va_arg(vargs, int));
				s = s + strlen(s);
				break;
			case 'x':
				sprintf(s, "%x", va_arg(vargs, int));
				s = s + strlen(s);
				break;
			case 's':
				p = va_arg(vargs, char*);
				i = strlen(p);
				if (n > 0 && i > n)
					i = n;
				memcpy(s, p, i);
				s = s + i;
				break;
			case '%':
				*s++ = '%';
				break;
			default:
				strcpy(s, p);
				s = s + strlen(s);
				goto end;
			}
		} else
			*s++ = *f;
	}
	
 end:
	
	/* printf("I am error %s\n",PyString_AsString(string)); */
	_PyString_Resize(&string, s - PyString_AsString(string));
	
	PyErr_SetObject(exception, string);
	Py_XDECREF(string);
	va_end(vargs);
	return NULL;
}


PyObject *
PyErr_NewException(char *name, PyObject *base, PyObject *dict)
{
	char *dot;
	PyObject *modulename = NULL;
	PyObject *classname = NULL;
	PyObject *mydict = NULL;
	PyObject *bases = NULL;
	PyObject *result = NULL;
	dot = strrchr(name, '.');
	if (dot == NULL) {
		PyErr_SetString(PyExc_SystemError,
			/*"PyErr_NewException: name must be module.class");*/
			"PyErr_NewException: 名稱必須是 模組.類別");
		return NULL;
	}
	if (base == NULL)
		base = PyExc_Exception;
	if (!PyClass_Check(base)) {
		/* Must be using string-based standard exceptions (-X) */
		return PyString_FromStringAndEncode(name, Source_Encoding);
	}
	if (dict == NULL) {
		dict = mydict = PyDict_New();
		if (dict == NULL)
			goto failure;
	}
	if (PyDict_GetItemString(dict, "__module__") == NULL) {
		modulename = PyString_FromStringAndSizeAndEncode(name, 
					(int)(dot-name), Source_Encoding);
		if (modulename == NULL)
			goto failure;
		if (PyDict_SetItemString(dict, "__module__", modulename) != 0)
			goto failure;
		if (PyDict_SetItemString(dict, "__模組__", modulename) != 0)
			goto failure;
	}
	classname = PyString_FromStringAndEncode(dot+1, Source_Encoding);
	if (classname == NULL)
		goto failure;
	bases = Py_BuildValue("(O)", base);
	if (bases == NULL)
		goto failure;
	result = PyClass_New(bases, dict, classname);
  failure:
	Py_XDECREF(bases);
	Py_XDECREF(mydict);
	Py_XDECREF(classname);
	Py_XDECREF(modulename);
	return result;
}

/* Call when an exception has occurred but there is no way for Python
   to handle it.  Examples: exception in __del__ or during GC. */
void
PyErr_WriteUnraisable(PyObject *obj)
{
	PyObject *f, *t, *v, *tb;
	PyErr_Fetch(&t, &v, &tb);
	f = PySys_GetObject("stderr");
	if (f != NULL) {
		PyFile_WriteString("錯誤:", f);
		if (t) {
			PyFile_WriteObject(t, f, Py_PRINT_RAW);
			if (v && v != Py_None) {
				PyFile_WriteString(": ", f);
				PyFile_WriteObject(v, f, 0);
			}
		}
		PyFile_WriteString("在", f);
		PyFile_WriteObject(obj, f, 0);
		PyFile_WriteString("被忽略\n", f);
		PyErr_Clear(); /* Just in case */
	}
	Py_XDECREF(t);
	Py_XDECREF(v);
	Py_XDECREF(tb);
}


/* Function to issue a warning message; may raise an exception. */
int
PyErr_Warn(PyObject *category, char *message)
{
	PyObject *mod, *dict, *func = NULL;

	mod = PyImport_ImportModule("warnings");
	if (mod != NULL) {
		dict = PyModule_GetDict(mod);
		func = PyDict_GetItemString(dict, "warn");
		Py_DECREF(mod);
	}
	if (func == NULL) {
		/*PySys_WriteStderr("warning: %s\n", message);*/
		PySys_WriteStderr("警告: %s\n", message);
		return 0;
	}
	else {
		PyObject *args, *res;

		if (category == NULL)
			category = PyExc_RuntimeWarning;
		args = Py_BuildValue("(sO)", message, category);
		if (args == NULL)
			return -1;
		res = PyEval_CallObject(func, args);
		Py_DECREF(args);
		if (res == NULL)
			return -1;
		Py_DECREF(res);
		return 0;
	}
}


/* Warning with explicit origin */
int
PyErr_WarnExplicit(PyObject *category, char *message,
		   char *filename, int lineno,
		   char *module, PyObject *registry)
{
	PyObject *mod, *dict, *func = NULL;

	mod = PyImport_ImportModule("warnings");
	if (mod != NULL) {
		dict = PyModule_GetDict(mod);
		func = PyDict_GetItemString(dict, "warn_explicit");
		Py_DECREF(mod);
	}
	if (func == NULL) {
		/*PySys_WriteStderr("warning: %s\n", message);*/
		PySys_WriteStderr("警告: %s\n", message);
		return 0;
	}
	else {
		PyObject *args, *res;

		if (category == NULL)
			category = PyExc_RuntimeWarning;
		if (registry == NULL)
			registry = Py_None;
		args = Py_BuildValue("(sOsizO)", message, category,
				     filename, lineno, module, registry);
		if (args == NULL)
			return -1;
		res = PyEval_CallObject(func, args);
		Py_DECREF(args);
		if (res == NULL)
			return -1;
		Py_DECREF(res);
		return 0;
	}
}


/* XXX There's a comment missing here */

void
PyErr_SyntaxLocation(char *filename, int lineno)
{
	PyObject *exc, *v, *tb, *tmp;

	/* add attributes for the line number and filename for the error */
	PyErr_Fetch(&exc, &v, &tb);
	PyErr_NormalizeException(&exc, &v, &tb);
	/* XXX check that it is, indeed, a syntax error */
	tmp = PyInt_FromLong(lineno);
	if (tmp == NULL)
		PyErr_Clear();
	else {
		if (PyObject_SetAttrString(v, "lineno", tmp))
			PyErr_Clear();
		if (PyObject_SetAttrString(v, "行號", tmp))
			PyErr_Clear();
		Py_DECREF(tmp);
	}
	if (filename != NULL) {
		tmp = PyString_FromStringAndEncode(filename, Source_Encoding);
		if (tmp == NULL)
			PyErr_Clear();
		else {
			if (PyObject_SetAttrString(v, "filename", tmp))
				PyErr_Clear();
			if (PyObject_SetAttrString(v, "檔案", tmp))
				PyErr_Clear();
			Py_DECREF(tmp);
		}

		tmp = PyErr_ProgramText(filename, lineno);
		if (tmp) {
			PyObject_SetAttrString(v, "text", tmp);
			PyObject_SetAttrString(v, "內文", tmp);
			Py_DECREF(tmp);
		}
	}
	PyErr_Restore(exc, v, tb);
}

/* com_fetch_program_text will attempt to load the line of text that
   the exception refers to.  If it fails, it will return NULL but will
   not set an exception. 

   XXX The functionality of this function is quite similar to the
   functionality in tb_displayline() in traceback.c.
*/

PyObject *
PyErr_ProgramText(char *filename, int lineno)
{
	FILE *fp;
	int i;
	char linebuf[1000];

	if (filename == NULL || lineno <= 0)
		return NULL;
	fp = fopen(filename, "r");
	if (fp == NULL)
		return NULL;
	for (i = 0; i < lineno; i++) {
		char *pLastChar = &linebuf[sizeof(linebuf) - 2];
		do {
			*pLastChar = '\0';
			if (fgets(linebuf, sizeof linebuf, fp) == NULL)
				break;
			/* fgets read *something*; if it didn't get as
			   far as pLastChar, it must have found a newline
			   or hit the end of the file;	if pLastChar is \n,
			   it obviously found a newline; else we haven't
			   yet seen a newline, so must continue */
		} while (*pLastChar != '\0' && *pLastChar != '\n');
	}
	fclose(fp);
	if (i == lineno) {
		char *p = linebuf;
		while (*p == ' ' || *p == '\t' || *p == '\014')
			p++;
		return PyString_FromStringAndEncode(p, Source_Encoding);
	}
	return NULL;
}
