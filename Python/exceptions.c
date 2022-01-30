/* This module provides the suite of standard class-based exceptions for
 * Python's builtin module.  This is a complete C implementation of what,
 * in Python 1.5.2, was contained in the exceptions.py module.  The problem
 * there was that if exceptions.py could not be imported for some reason,
 * the entire interpreter would abort.
 *
 * By moving the exceptions into C and statically linking, we can guarantee
 * that the standard exceptions will always be available.
 *
 * history:
 * 98-08-19    fl   created (for pyexe)
 * 00-02-08    fl   updated for 1.5.2
 * 26-May-2000 baw  vetted for Python 1.6
 *
 * written by Fredrik Lundh
 * modifications, additions, cleanups, and proofreading by Barry Warsaw
 *
 * Copyright (c) 1998-2000 by Secret Labs AB.  All rights reserved.
 */

#include "Python.h"
#include "osdefs.h"

/* Caution:  MS Visual C++ 6 errors if a single string literal exceeds
 * 2Kb.  So the module docstring has been broken roughly in half, using
 * compile-time literal concatenation.
 */
static char
module__doc__[] =
"Python's standard exception class hierarchy.\n\
\n\
Before Python 1.5, the standard exceptions were all simple string objects.\n\
In Python 1.5, the standard exceptions were converted to classes organized\n\
into a relatively flat hierarchy.  String-based standard exceptions were\n\
optional, or used as a fallback if some problem occurred while importing\n\
the exception module.  With Python 1.6, optional string-based standard\n\
exceptions were removed (along with the -X command line flag).\n\
\n\
The class exceptions were implemented in such a way as to be almost\n\
completely backward compatible.  Some tricky uses of IOError could\n\
potentially have broken, but by Python 1.6, all of these should have\n\
been fixed.  As of Python 1.6, the class-based standard exceptions are\n\
now implemented in C, and are guaranteed to exist in the Python\n\
interpreter.\n\
\n\
Here is a rundown of the class hierarchy.  The classes found here are\n\
inserted into both the exceptions module and the `built-in' module.  It is\n\
recommended that user defined class based exceptions be derived from the\n\
`Exception' class, although this is currently not enforced.\n"
	/* keep string pieces "small" */
"\n\
Exception\n\
 |\n\
 +-- SystemExit\n\
 +-- StandardError\n\
 |    |\n\
 |    +-- KeyboardInterrupt\n\
 |    +-- ImportError\n\
 |    +-- EnvironmentError\n\
 |    |    |\n\
 |    |    +-- IOError\n\
 |    |    +-- OSError\n\
 |    |         |\n\
 |    |         +-- WindowsError\n\
 |    |\n\
 |    +-- EOFError\n\
 |    +-- RuntimeError\n\
 |    |    |\n\
 |    |    +-- NotImplementedError\n\
 |    |\n\
 |    +-- NameError\n\
 |    |    |\n\
 |    |    +-- UnboundLocalError\n\
 |    |\n\
 |    +-- AttributeError\n\
 |    +-- SyntaxError\n\
 |    |    |\n\
 |    |    +-- IndentationError\n\
 |    |         |\n\
 |    |         +-- TabError\n\
 |    |\n\
 |    +-- TypeError\n\
 |    +-- AssertionError\n\
 |    +-- LookupError\n\
 |    |    |\n\
 |    |    +-- IndexError\n\
 |    |    +-- KeyError\n\
 |    |\n\
 |    +-- ArithmeticError\n\
 |    |    |\n\
 |    |    +-- OverflowError\n\
 |    |    +-- ZeroDivisionError\n\
 |    |    +-- FloatingPointError\n\
 |    |\n\
 |    +-- ValueError\n\
 |    |    |\n\
 |    |    +-- UnicodeError\n\
 |    |\n\
 |    +-- SystemError\n\
 |    +-- MemoryError\n\
 |\n\
 +---Warning\n\
      |\n\
      +-- UserWarning\n\
      +-- DeprecationWarning\n\
      +-- SyntaxWarning\n\
      +-- RuntimeWarning";


/* Helper function for populating a dictionary with method wrappers. */
static int
populate_methods(PyObject *klass, PyObject *dict, PyMethodDef *methods)
{
    if (!methods)
	return 0;

    while (methods->ml_name) {
	/* get a wrapper for the built-in function */
	PyObject *func = PyCFunction_New(methods, NULL);
	PyObject *meth;
	int status;

	if (!func)
	    return -1;

	/* turn the function into an unbound method */
	if (!(meth = PyMethod_New(func, NULL, klass))) {
	    Py_DECREF(func);
	    return -1;
	}

	/* add method to dictionary */
	status = PyDict_SetItemString(dict, methods->ml_name, meth);
	Py_DECREF(meth);
	Py_DECREF(func);

	/* stop now if an error occurred, otherwise do the next method */
	if (status)
	    return status;

	methods++;
    }
    return 0;
}



/* This function is used to create all subsequent exception classes. */
static int
make_class(PyObject **klass, PyObject *base,
	   char *name, PyMethodDef *methods,
	   char *docstr)
{
    PyObject *dict = PyDict_New();
    PyObject *str = NULL;
    int status = -1;

    if (!dict)
	return -1;

    /* If an error occurs from here on, goto finally instead of explicitly
     * returning NULL.
     */

    if (docstr) {
	if (!(str = PyString_FromStringAndEncode(docstr, Source_Encoding)))
	    goto finally;
	if (PyDict_SetItemString(dict, "__doc__", str))
	    goto finally;
	if (PyDict_SetItemString(dict, "__����__", str))
	    goto finally;
    }

    if (!(*klass = PyErr_NewException(name, base, dict)))
	goto finally;

    if (populate_methods(*klass, dict, methods)) {
	Py_DECREF(*klass);
	*klass = NULL;
	goto finally;
    }

    status = 0;

  finally:
    Py_XDECREF(dict);
    Py_XDECREF(str);
    return status;
}


/* Use this for *args signatures, otherwise just use PyArg_ParseTuple() */
static PyObject *
get_self(PyObject *args)
{
    PyObject *self = PyTuple_GetItem(args, 0);
    if (!self) {
	/* Watch out for being called to early in the bootstrapping process */
	if (PyExc_TypeError) {
	    PyErr_SetString(PyExc_TypeError,
	     /*"unbound method must be called with instance as first argument");*/
	     "�եΩ|���i�w��k�ɲĤ@�ӰѼƥ����O����");
	}
        return NULL;
    }
    return self;
}



/* Notes on bootstrapping the exception classes.
 *
 * First thing we create is the base class for all exceptions, called
 * appropriately enough: Exception.  Creation of this class makes no
 * assumptions about the existence of any other exception class -- except
 * for TypeError, which can conditionally exist.
 *
 * Next, StandardError is created (which is quite simple) followed by
 * TypeError, because the instantiation of other exceptions can potentially
 * throw a TypeError.  Once these exceptions are created, all the others
 * can be created in any order.  See the static exctable below for the
 * explicit bootstrap order.
 *
 * All classes after Exception can be created using PyErr_NewException().
 */

static char
/*Exception__doc__[] = "Common base class for all exceptions.";*/
Exception__doc__[] = "�����Ҧ����`�Ȫ����h����";


static PyObject *
Exception__init__(PyObject *self, PyObject *args)
{
    int status, status2;

    if (!(self = get_self(args)))
	return NULL;

    /* set args attribute */
    /* XXX size is only a hint */
    args = PySequence_GetSlice(args, 1, PySequence_Size(args));
    if (!args)
        return NULL;
    status = PyObject_SetAttrString(self, "args", args);
    status2 = PyObject_SetAttrString(self, "�Ѽ�", args);
    Py_DECREF(args);
    if (status < 0 || status2 < 0){
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *
Exception__str__(PyObject *self, PyObject *args)
{
    PyObject *out;

    if (!PyArg_ParseTuple(args, "O:__str__", &self))
        return NULL;

    args = PyObject_GetAttrString(self, "args");
    if (!args)
        return NULL;

    switch (PySequence_Size(args)) {
    case 0:
        out = PyString_FromStringAndEncode("", Source_Encoding);
        break;
    case 1:
    {
	PyObject *tmp = PySequence_GetItem(args, 0);
	if (tmp) {
	    out = PyObject_Str(tmp);
	    Py_DECREF(tmp);
	}
	else
	    out = NULL;
	break;
    }
    default:
        out = PyObject_Str(args);
        break;
    }

    Py_DECREF(args);
    return out;
}


static PyObject *
Exception__getitem__(PyObject *self, PyObject *args)
{
    PyObject *out;
    PyObject *index;

    if (!PyArg_ParseTuple(args, "OO:__getitem__", &self, &index))
        return NULL;

    args = PyObject_GetAttrString(self, "args");
    if (!args)
        return NULL;

    out = PyObject_GetItem(args, index);
    Py_DECREF(args);
    return out;
}


static PyMethodDef
Exception_methods[] = {
    /* methods for the Exception class */
    { "__getitem__", Exception__getitem__, METH_VARARGS},
    { "__���l��__", Exception__getitem__, METH_VARARGS},
    { "__str__",     Exception__str__, METH_VARARGS},
    { "__�ܦr��__",     Exception__str__, METH_VARARGS},
    { "__init__",    Exception__init__, METH_VARARGS},
    { "__��l__",    Exception__init__, METH_VARARGS},
    { NULL, NULL }
};


static int
make_Exception(char *modulename)
{
    PyObject *dict = PyDict_New();
    PyObject *str = NULL;
    PyObject *name = NULL;
    int status = -1;

    if (!dict)
	return -1;

    /* If an error occurs from here on, goto finally instead of explicitly
     * returning NULL.
     */

    if (!(str = PyString_FromStringAndEncode(modulename, Source_Encoding)))
	goto finally;
    if (PyDict_SetItemString(dict, "__module__", str))
	goto finally;
    if (PyDict_SetItemString(dict, "__�Ҳ�__", str))
	goto finally;
    Py_DECREF(str);
    if (!(str = PyString_FromStringAndEncode(Exception__doc__, Source_Encoding)))
	goto finally;
    if (PyDict_SetItemString(dict, "__doc__", str))
	goto finally;
    if (PyDict_SetItemString(dict, "__����__", str))
	goto finally;

    if (!(name = PyString_FromStringAndEncode("Exception", Source_Encoding)))
	goto finally;

    if (!(PyExc_Exception = PyClass_New(NULL, dict, name)))
	goto finally;

    /* Now populate the dictionary with the method suite */
    if (populate_methods(PyExc_Exception, dict, Exception_methods))
	/* Don't need to reclaim PyExc_Exception here because that'll
	 * happen during interpreter shutdown.
	 */
	goto finally;

    status = 0;

  finally:
    Py_XDECREF(dict);
    Py_XDECREF(str);
    Py_XDECREF(name);
    return status;
}



static char
/*StandardError__doc__[] = "Base class for all standard Python exceptions.";*/
StandardError__doc__[] = "�Ҧ������зǲ��`�Ȫ����h����";

static char
TypeError__doc__[] = "�Ѽ����O����";



static char
SystemExit__doc__[] = "�n�D���}���ʼҦ�";


static PyObject *
SystemExit__init__(PyObject *self, PyObject *args)
{
    PyObject *code;
    int status, status2;

    if (!(self = get_self(args)))
	return NULL;

    /* Set args attribute. */
    if (!(args = PySequence_GetSlice(args, 1, PySequence_Size(args))))
        return NULL;

    status = PyObject_SetAttrString(self, "args", args);
    status2 = PyObject_SetAttrString(self, "�Ѽ�", args);
    if (status < 0 || status2 < 0) {
	Py_DECREF(args);
        return NULL;
    }

    /* set code attribute */
    switch (PySequence_Size(args)) {
    case 0:
        Py_INCREF(Py_None);
        code = Py_None;
        break;
    case 1:
        code = PySequence_GetItem(args, 0);
        break;
    default:
        Py_INCREF(args);
        code = args;
        break;
    }

    status = PyObject_SetAttrString(self, "code", code);
    status2 = PyObject_SetAttrString(self, "�{���X", code);
    Py_DECREF(code);
    Py_DECREF(args);
    if (status < 0 || status2 < 0)
        return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}


static PyMethodDef SystemExit_methods[] = {
    { "__init__", SystemExit__init__, METH_VARARGS},
    { "__��l__", SystemExit__init__, METH_VARARGS},
    {NULL, NULL}
};



static char
KeyboardInterrupt__doc__[] = "�ϥΪ̭n�D�פ�{���B��";

static char
ImportError__doc__[] =
"�L�k���J�ҲթμҲզW�٤��s�b";



static char
EnvironmentError__doc__[] = "��J/��X�����`�����h����";


static PyObject *
EnvironmentError__init__(PyObject *self, PyObject *args)
{
    PyObject *item0 = NULL;
    PyObject *item1 = NULL;
    PyObject *item2 = NULL;
    PyObject *subslice = NULL;
    PyObject *rtnval = NULL;

    if (!(self = get_self(args)))
	return NULL;

    if (!(args = PySequence_GetSlice(args, 1, PySequence_Size(args))))
	return NULL;

    if (PyObject_SetAttrString(self, "args", args) ||
	PyObject_SetAttrString(self, "errno", Py_None) ||
	PyObject_SetAttrString(self, "strerror", Py_None) ||
	PyObject_SetAttrString(self, "filename", Py_None) ||
        PyObject_SetAttrString(self, "�Ѽ�", args) ||
	PyObject_SetAttrString(self, "���~�N��", Py_None) ||
	PyObject_SetAttrString(self, "���~�T��", Py_None) ||
	PyObject_SetAttrString(self, "�ɮ�", Py_None)) 
    {
	goto finally;
    }

    switch (PySequence_Size(args)) {
    case 3:
	/* Where a function has a single filename, such as open() or some
	 * of the os module functions, PyErr_SetFromErrnoWithFilename() is
	 * called, giving a third argument which is the filename.  But, so
	 * that old code using in-place unpacking doesn't break, e.g.:
	 *
	 * except IOError, (errno, strerror):
	 *
	 * we hack args so that it only contains two items.  This also
	 * means we need our own __str__() which prints out the filename
	 * when it was supplied.
	 */
	item0 = PySequence_GetItem(args, 0);
	item1 = PySequence_GetItem(args, 1);
	item2 = PySequence_GetItem(args, 2);
	if (!item0 || !item1 || !item2)
	    goto finally;

	if (PyObject_SetAttrString(self, "errno", item0) ||
	    PyObject_SetAttrString(self, "strerror", item1) ||
	    PyObject_SetAttrString(self, "filename", item2) ||
	    PyObject_SetAttrString(self, "���~�N��", item0) ||
	    PyObject_SetAttrString(self, "���~�T��", item1) ||
	    PyObject_SetAttrString(self, "�ɮ�", item2))
	{
	    goto finally;
	}

	subslice = PySequence_GetSlice(args, 0, 2);
	if (!subslice || PyObject_SetAttrString(self, "args", subslice) ||
		PyObject_SetAttrString(self, "�Ѽ�", subslice))
	    goto finally;
	break;

    case 2:
	/* Used when PyErr_SetFromErrno() is called and no filename
	 * argument is given.
	 */
	item0 = PySequence_GetItem(args, 0);
	item1 = PySequence_GetItem(args, 1);
	if (!item0 || !item1)
	    goto finally;

	if (PyObject_SetAttrString(self, "errno", item0) ||
	    PyObject_SetAttrString(self, "strerror", item1) ||
	    PyObject_SetAttrString(self, "���~�N��", item0) ||
	    PyObject_SetAttrString(self, "���~�T��", item1))
	{
	    goto finally;
	}
	break;
    }

    Py_INCREF(Py_None);
    rtnval = Py_None;

  finally:
    Py_DECREF(args);
    Py_XDECREF(item0);
    Py_XDECREF(item1);
    Py_XDECREF(item2);
    Py_XDECREF(subslice);
    return rtnval;
}


static PyObject *
EnvironmentError__str__(PyObject *self, PyObject *args)
{
    PyObject *originalself = self;
    PyObject *filename;
    PyObject *serrno;
    PyObject *strerror;
    PyObject *rtnval = NULL;

    if (!PyArg_ParseTuple(args, "O:__str__", &self))
	return NULL;

    filename = PyObject_GetAttrString(self, "filename");
    serrno = PyObject_GetAttrString(self, "errno");
    strerror = PyObject_GetAttrString(self, "strerror");
    if (!filename || !serrno || !strerror)
	goto finally;

    if (filename != Py_None) {
	/*PyObject *fmt = PyString_FromString("[Errno %s] %s: %s");*/
	PyObject *fmt = PyString_FromStringAndEncode("[���~�N�� %s] %s: %s", 
					Source_Encoding);
	PyObject *repr = PyObject_Repr(filename);
	PyObject *tuple = PyTuple_New(3);

	if (!fmt || !repr || !tuple) {
	    Py_XDECREF(fmt);
	    Py_XDECREF(repr);
	    Py_XDECREF(tuple);
	    goto finally;
	}

	PyTuple_SET_ITEM(tuple, 0, serrno);
	PyTuple_SET_ITEM(tuple, 1, strerror);
	PyTuple_SET_ITEM(tuple, 2, repr);

	rtnval = PyString_Format(fmt, tuple);

	Py_DECREF(fmt);
	Py_DECREF(tuple);
	/* already freed because tuple owned only reference */
	serrno = NULL;
	strerror = NULL;
    }
    else if (PyObject_IsTrue(serrno) && PyObject_IsTrue(strerror)) {
	/*PyObject *fmt = PyString_FromString("[Errno %s] %s");*/
	PyObject *fmt = PyString_FromStringAndEncode("[���~�N�� %s] %s", 
					Source_Encoding);
	PyObject *tuple = PyTuple_New(2);

	if (!fmt || !tuple) {
	    Py_XDECREF(fmt);
	    Py_XDECREF(tuple);
	    goto finally;
	}

	PyTuple_SET_ITEM(tuple, 0, serrno);
	PyTuple_SET_ITEM(tuple, 1, strerror);

	rtnval = PyString_Format(fmt, tuple);

	Py_DECREF(fmt);
	Py_DECREF(tuple);
	/* already freed because tuple owned only reference */
	serrno = NULL;
	strerror = NULL;
    }
    else
	/* The original Python code said:
	 *
	 *   return StandardError.__str__(self)
	 *
	 * but there is no StandardError__str__() function; we happen to
	 * know that's just a pass through to Exception__str__().
	 */
	rtnval = Exception__str__(originalself, args);

  finally:
    Py_XDECREF(filename);
    Py_XDECREF(serrno);
    Py_XDECREF(strerror);
    return rtnval;
}


static
PyMethodDef EnvironmentError_methods[] = {
    {"__init__", EnvironmentError__init__, METH_VARARGS},
    {"__��l__", EnvironmentError__init__, METH_VARARGS},
    {"__str__",  EnvironmentError__str__, METH_VARARGS},
    {"__�ܦr��__",  EnvironmentError__str__, METH_VARARGS},
    {NULL, NULL}
};




static char
IOError__doc__[] = "��J/��X���ާ@����";

static char
OSError__doc__[] = "�եξާ@�t�Ψ�ƥ���";

#ifdef MS_WINDOWS
static char
WindowsError__doc__[] = "�ե� MS-Windows ��ƥ���";
#endif /* MS_WINDOWS */

static char
EOFError__doc__[] = "�w�W�L�F�ɮק���";

static char
RuntimeError__doc__[] = "���椤�J��N�Ƥ��쪺���`";

static char
NotImplementedError__doc__[] =
"�o�ӥΪk�Φ�����Ʃ|�����X��";

static char
NameError__doc__[] = "�L�k���o�ӦW�r";

static char
UnboundLocalError__doc__[] =
"�o�Ӱϰ��ܼ��٨S����";

static char
AttributeError__doc__[] = "�S�o���ݩ�";



static char
SyntaxError__doc__[] = "�y�k�g���F";


static int
SyntaxError__classinit__(PyObject *klass)
{
    int retval = 0;
    PyObject *emptystring = PyString_FromStringAndEncode("", Source_Encoding);

    /* Additional class-creation time initializations */
    if (!emptystring ||
	PyObject_SetAttrString(klass, "msg", emptystring) ||
	PyObject_SetAttrString(klass, "filename", Py_None) ||
	PyObject_SetAttrString(klass, "lineno", Py_None) ||
	PyObject_SetAttrString(klass, "offset", Py_None) ||
	PyObject_SetAttrString(klass, "text", Py_None) ||
	PyObject_SetAttrString(klass, "�T��", emptystring) ||
	PyObject_SetAttrString(klass, "�ɮ�", Py_None) ||
	PyObject_SetAttrString(klass, "�渹", Py_None) ||
	PyObject_SetAttrString(klass, "������", Py_None) ||
	PyObject_SetAttrString(klass, "����", Py_None))
    {
	retval = -1;
    }
    Py_XDECREF(emptystring);
    return retval;
}


static PyObject *
SyntaxError__init__(PyObject *self, PyObject *args)
{
    PyObject *rtnval = NULL;
    int lenargs;

    if (!(self = get_self(args)))
	return NULL;

    if (!(args = PySequence_GetSlice(args, 1, PySequence_Size(args))))
	return NULL;

    if (PyObject_SetAttrString(self, "args", args))
	goto finally;

    if (PyObject_SetAttrString(self, "�Ѽ�", args))
	goto finally;

    lenargs = PySequence_Size(args);
    if (lenargs >= 1) {
	PyObject *item0 = PySequence_GetItem(args, 0);
	int status, status2;

	if (!item0)
	    goto finally;
	status = PyObject_SetAttrString(self, "msg", item0);
	status2 = PyObject_SetAttrString(self, "�T��", item0);
	Py_DECREF(item0);
	if (status || status2)
	    goto finally;
    }
    if (lenargs == 2) {
	PyObject *info = PySequence_GetItem(args, 1);
	PyObject *filename = NULL, *lineno = NULL;
	PyObject *offset = NULL, *text = NULL;
	int status = 1;

	if (!info)
	    goto finally;

	filename = PySequence_GetItem(info, 0);
	if (filename != NULL) {
	    lineno = PySequence_GetItem(info, 1);
	    if (lineno != NULL) {
		offset = PySequence_GetItem(info, 2);
		if (offset != NULL) {
		    text = PySequence_GetItem(info, 3);
		    if (text != NULL) {
			status =
			    PyObject_SetAttrString(self, "filename", filename)
			    || PyObject_SetAttrString(self, "lineno", lineno)
			    || PyObject_SetAttrString(self, "offset", offset)
			    || PyObject_SetAttrString(self, "text", text)
			    || PyObject_SetAttrString(self, "�ɮ�", filename)
			    || PyObject_SetAttrString(self, "�渹", lineno)
			    || PyObject_SetAttrString(self, "������", offset)
			    || PyObject_SetAttrString(self, "����", text);
			Py_DECREF(text);
		    }
		    Py_DECREF(offset);
		}
		Py_DECREF(lineno);
	    }
	    Py_DECREF(filename);
	}
	Py_DECREF(info);

	if (status)
	    goto finally;
    }
    Py_INCREF(Py_None);
    rtnval = Py_None;

  finally:
    Py_DECREF(args);
    return rtnval;
}


/* This is called "my_basename" instead of just "basename" to avoid name
   conflicts with glibc; basename is already prototyped if _GNU_SOURCE is
   defined, and Python does define that. */
static char *
my_basename(char *name)
{
	char *cp = name;
	char *result = name;

	if (name == NULL)
		return "???";
	while (*cp != '\0') {
		if (*cp == SEP)
			result = cp + 1;
		++cp;
	}
	return result;
}


static PyObject *
SyntaxError__str__(PyObject *self, PyObject *args)
{
    PyObject *msg;
    PyObject *str;
    PyObject *filename, *lineno, *result;

    if (!PyArg_ParseTuple(args, "O:__str__", &self))
	return NULL;

    if (!(msg = PyObject_GetAttrString(self, "msg")))
	return NULL;

    str = PyObject_Str(msg);
    Py_DECREF(msg);
    result = str;

    /* XXX -- do all the additional formatting with filename and
       lineno here */

    if (PyString_Check(str)) {
	int have_filename = 0;
	int have_lineno = 0;
	char *buffer = NULL;

	if ((filename = PyObject_GetAttrString(self, "filename")) != NULL)
	    have_filename = PyString_Check(filename);
	else
	    PyErr_Clear();

	if ((lineno = PyObject_GetAttrString(self, "lineno")) != NULL)
	    have_lineno = PyInt_Check(lineno);
	else
	    PyErr_Clear();

	if (have_filename || have_lineno) {
	    int bufsize = PyString_GET_SIZE(str) + 64;
	    if (have_filename)
		bufsize += PyString_GET_SIZE(filename);

	    buffer = PyMem_Malloc(bufsize);
	    if (buffer != NULL) {
		if (have_filename && have_lineno)
		    /*sprintf(buffer, "%s (%s, line %ld)",*/
		    sprintf(buffer, "%s (%s, �� %ld)",
			    PyString_AS_STRING(str),
			    my_basename(PyString_AS_STRING(filename)),
			    PyInt_AsLong(lineno));
		else if (have_filename)
		    sprintf(buffer, "%s (%s)",
			    PyString_AS_STRING(str),
			    my_basename(PyString_AS_STRING(filename)));
		else if (have_lineno)
		    /*sprintf(buffer, "%s (line %ld)",*/
		    sprintf(buffer, "%s (�� %ld)",
			    PyString_AS_STRING(str),
			    PyInt_AsLong(lineno));

		result = PyString_FromStringAndEncode(buffer, Source_Encoding);
		PyMem_FREE(buffer);

		if (result == NULL)
		    result = str;
		else
		    Py_DECREF(str);
	    }
	}
	Py_XDECREF(filename);
	Py_XDECREF(lineno);
    }
    return result;
}


static PyMethodDef SyntaxError_methods[] = {
    {"__init__", SyntaxError__init__, METH_VARARGS},
    {"__��l__", SyntaxError__init__, METH_VARARGS},
    {"__str__",  SyntaxError__str__, METH_VARARGS},
    {"__�ܦr��__",  SyntaxError__str__, METH_VARARGS},
    {NULL, NULL}
};



/* Exception doc strings */

static char
AssertionError__doc__[] = "����q�L����";

static char
LookupError__doc__[] = "�M�䲧�`�����h����";

static char
IndexError__doc__[] = "���ޭȦb�ǦC�d��H�~";

static char
KeyError__doc__[] = "�䤣��o�����";

static char
ArithmeticError__doc__[] = "��N���`�����h����";

static char
OverflowError__doc__[] = "���G�Ӥj�F�L�k���";

static char
ZeroDivisionError__doc__[] =
"���k�κ�l�ƪ��p�⤤���Ƭ��s";

static char
FloatingPointError__doc__[] = "�B�I�ƾާ@����";

static char
ValueError__doc__[] = "�ѼƭȪ����O����";

static char
UnicodeError__doc__[] = "�M Unicode ���������`";

static char
SystemError__doc__[] = "Python �����X��\n\
\n\
�Ч�o�����`�H�β��`�l�ܩM����,�w��/�ާ@�t�ε�\n\
��Ƴ��i�����������@�H��";

static char
MemoryError__doc__[] = "�O���餣���F";

static char
IndentationError__doc__[] = "�{�����Y�Ʈ榡�˿��F";

static char
TabError__doc__[] = "�Ů�M����V�X�ϥΪ�����";

/* Warning category docstrings */

static char
/*Warning__doc__[] = "Base class for warning categories.";*/
Warning__doc__[] = "ĵ�i�������h����";

static char
/*UserWarning__doc__[] = "Base class for warnings generated by user code.";*/
UserWarning__doc__[] = "�Τ�w�q��ĵ�i�������h����";

static char
DeprecationWarning__doc__[] =
/*"Base class for warnings about deprecated features.";*/
"�ϥΤF�ª��\\�઺ĵ�i�����h����";

static char
/*SyntaxWarning__doc__[] = "Base class for warnings about dubious syntax.";*/
SyntaxWarning__doc__[] = "�o�X�y�k���~ĵ�i�����h����";

static char
RuntimeWarning__doc__[] =
/*"Base class for warnings about dubious runtime behavior.";*/
"�o�X�B��ɲ��`ĵ�i�����h����";



/* module global functions */
static PyMethodDef functions[] = {
    /* Sentinel */
    {NULL, NULL}
};



/* Global C API defined exceptions */

PyObject *PyExc_Exception;
PyObject *PyExc_StandardError;
PyObject *PyExc_ArithmeticError;
PyObject *PyExc_LookupError;

PyObject *PyExc_AssertionError;
PyObject *PyExc_AttributeError;
PyObject *PyExc_EOFError;
PyObject *PyExc_FloatingPointError;
PyObject *PyExc_EnvironmentError;
PyObject *PyExc_IOError;
PyObject *PyExc_OSError;
PyObject *PyExc_ImportError;
PyObject *PyExc_IndexError;
PyObject *PyExc_KeyError;
PyObject *PyExc_KeyboardInterrupt;
PyObject *PyExc_MemoryError;
PyObject *PyExc_NameError;
PyObject *PyExc_OverflowError;
PyObject *PyExc_RuntimeError;
PyObject *PyExc_NotImplementedError;
PyObject *PyExc_SyntaxError;
PyObject *PyExc_IndentationError;
PyObject *PyExc_TabError;
PyObject *PyExc_SystemError;
PyObject *PyExc_SystemExit;
PyObject *PyExc_UnboundLocalError;
PyObject *PyExc_UnicodeError;
PyObject *PyExc_TypeError;
PyObject *PyExc_ValueError;
PyObject *PyExc_ZeroDivisionError;
#ifdef MS_WINDOWS
PyObject *PyExc_WindowsError;
#endif

/* Pre-computed MemoryError instance.  Best to create this as early as
 * possibly and not wait until a MemoryError is actually raised!
 */
PyObject *PyExc_MemoryErrorInst;

/* Predefined warning categories */
PyObject *PyExc_Warning;
PyObject *PyExc_UserWarning;
PyObject *PyExc_DeprecationWarning;
PyObject *PyExc_SyntaxWarning;
PyObject *PyExc_RuntimeWarning;



/* mapping between exception names and their PyObject ** */
static struct {
    char *name;
    char *cp_name;
    PyObject **exc;
    PyObject **base;			     /* NULL == PyExc_StandardError */
    char *docstr;
    PyMethodDef *methods;
    int (*classinit)(PyObject *);
} exctable[] = {
 /*
  * The first three classes MUST appear in exactly this order
  */
 {"Exception", "���`", &PyExc_Exception},
 {"StandardError", "�зǲ��`", &PyExc_StandardError, &PyExc_Exception,
  StandardError__doc__},
 {"TypeError", "�������`", &PyExc_TypeError, 0, TypeError__doc__},
 /*
  * The rest appear in depth-first order of the hierarchy
  */
 {"SystemExit", "�h�X�t��", &PyExc_SystemExit, &PyExc_Exception, SystemExit__doc__,
  SystemExit_methods},
 {"KeyboardInterrupt",  "��L���_", &PyExc_KeyboardInterrupt, 0, KeyboardInterrupt__doc__},
 {"ImportError", "���J���`",  &PyExc_ImportError,       0, ImportError__doc__},
 {"EnvironmentError", "���Ҳ��`",   &PyExc_EnvironmentError,  0, EnvironmentError__doc__,
  EnvironmentError_methods},
 {"IOError", "��J/�X���`", &PyExc_IOError, &PyExc_EnvironmentError, IOError__doc__},
 {"OSError", "�ާ@�t�β��`", &PyExc_OSError, &PyExc_EnvironmentError, OSError__doc__},
#ifdef MS_WINDOWS
 {"WindowsError", "�������`", &PyExc_WindowsError, &PyExc_OSError,
  WindowsError__doc__},
#endif /* MS_WINDOWS */
 {"EOFError", "�ɮײ׵����`", &PyExc_EOFError,     0, EOFError__doc__},
 {"RuntimeError", "���沧�`", &PyExc_RuntimeError, 0, RuntimeError__doc__},
 {"NotImplementedError", "�|���䴩", &PyExc_NotImplementedError,
  &PyExc_RuntimeError, NotImplementedError__doc__},
 {"NameError", "�W�ٲ��`",  &PyExc_NameError,    0, NameError__doc__},
 {"UnboundLocalError", "�|�����j", &PyExc_UnboundLocalError, &PyExc_NameError,
  UnboundLocalError__doc__},
 {"AttributeError",  "�ݩʲ��`",  &PyExc_AttributeError, 0, AttributeError__doc__},
 {"SyntaxError", "�y�k���`", &PyExc_SyntaxError,    0, SyntaxError__doc__,
  SyntaxError_methods, SyntaxError__classinit__},
 {"IndentationError", "�Y�Ʋ��`",  &PyExc_IndentationError, &PyExc_SyntaxError,
  IndentationError__doc__},
 {"TabError", "���沧�`",   &PyExc_TabError, &PyExc_IndentationError,
  TabError__doc__},
 {"AssertionError", "�_�����`", &PyExc_AssertionError, 0, AssertionError__doc__},
 {"LookupError", "�d�䲧�`", &PyExc_LookupError,    0, LookupError__doc__},
 {"IndexError", "���޲��`",   &PyExc_IndexError,     &PyExc_LookupError,
  IndexError__doc__},
 {"KeyError",  "��Ȳ��`",  &PyExc_KeyError,       &PyExc_LookupError,
  KeyError__doc__},
 {"ArithmeticError", "��N���`",  &PyExc_ArithmeticError, 0, ArithmeticError__doc__},
 {"OverflowError", "���Ȳ��`",  &PyExc_OverflowError,     &PyExc_ArithmeticError,
  OverflowError__doc__},
 {"ZeroDivisionError", "���s���`",  &PyExc_ZeroDivisionError,  &PyExc_ArithmeticError,
  ZeroDivisionError__doc__},
 {"FloatingPointError", "�B�I���`", &PyExc_FloatingPointError, &PyExc_ArithmeticError,
  FloatingPointError__doc__},
 {"ValueError", "�Ȳ��`",   &PyExc_ValueError,  0, ValueError__doc__},
 {"UnicodeError", "�Τ@�X���`",  &PyExc_UnicodeError, &PyExc_ValueError, UnicodeError__doc__},
 {"SystemError", "�t�β��`",  &PyExc_SystemError, 0, SystemError__doc__},
 {"MemoryError", "�O���鲧�`",  &PyExc_MemoryError, 0, MemoryError__doc__},
 /* Warning categories */
 {"Warning", "ĵ�i", &PyExc_Warning, &PyExc_Exception, Warning__doc__},
 {"UserWarning", "�Τ�ĵ�i", &PyExc_UserWarning, &PyExc_Warning, UserWarning__doc__},
 {"DeprecationWarning", "�w�L�ɪ��Ϊk", &PyExc_DeprecationWarning, &PyExc_Warning,
  DeprecationWarning__doc__},
 {"SyntaxWarning", "�y�kĵ�i", &PyExc_SyntaxWarning, &PyExc_Warning, SyntaxWarning__doc__},
 {"RuntimeWarning", "�B���ĵ�i", &PyExc_RuntimeWarning, &PyExc_Warning,
  RuntimeWarning__doc__},
 /* Sentinel */
 {NULL}
};



DL_EXPORT(void)
init_exceptions(void)
{
    char *modulename = "exceptions";
    int modnamesz = strlen(modulename);
    int i;

    PyObject *me = Py_InitModule(modulename, functions);
    PyObject *mydict = PyModule_GetDict(me);
    PyObject *bltinmod = PyImport_ImportModule("__builtin__");
    PyObject *bdict = PyModule_GetDict(bltinmod);
    PyObject *doc = PyString_FromStringAndEncode(module__doc__, Source_Encoding);
    PyObject *args;

    PyDict_SetItemString(mydict, "__doc__", doc);
    PyDict_SetItemString(mydict, "__����__", doc);
    Py_DECREF(doc);
    if (PyErr_Occurred())
	/*Py_FatalError("exceptions bootstrapping error.");*/
	Py_FatalError("�L�k�Ұʲ��`�B�z");

    /* This is the base class of all exceptions, so make it first. */
    if (make_Exception(modulename) ||
	PyDict_SetItemString(mydict, "Exception", PyExc_Exception) ||
	PyDict_SetItemString(mydict, "���`", PyExc_Exception) ||
	PyDict_SetItemString(bdict,  "Exception", PyExc_Exception) ||
	PyDict_SetItemString(bdict, "���`", PyExc_Exception))
    {
	/*Py_FatalError("Base class `Exception' could not be created.");*/
	Py_FatalError("�L�k�Ы� `���`'���h����");
    }

    /* Now we can programmatically create all the remaining exceptions.
     * Remember to start the loop at 1 to skip Exceptions.
     */
    for (i=1; exctable[i].cp_name; i++) {
	int status;
	char *cname = PyMem_NEW(char, modnamesz+strlen(exctable[i].cp_name)+2);
	PyObject *base;

	(void)strcpy(cname, modulename);
	(void)strcat(cname, ".");
	(void)strcat(cname, exctable[i].cp_name);

	if (exctable[i].base == 0)
	    base = PyExc_StandardError;
	else
	    base = *exctable[i].base;

	status = make_class(exctable[i].exc, base, cname,
			    exctable[i].methods,
			    exctable[i].docstr);

	PyMem_DEL(cname);

	if (status)
	    /*Py_FatalError("Standard exception classes could not be created.");*/
	    Py_FatalError("�L�k�Ыؼзǲ��`���O");

	if (exctable[i].classinit) {
	    status = (*exctable[i].classinit)(*exctable[i].exc);
	    if (status)
		/*Py_FatalError("An exception class could not be initialized.");*/
		Py_FatalError("�L�k��l�ƬY�Ӳ��`�B�z��");
	}

	/* Now insert the class into both this module and the __builtin__
	 * module.
	 */
	if (PyDict_SetItemString(mydict, exctable[i].name, *exctable[i].exc) ||
	    PyDict_SetItemString(bdict, exctable[i].name, *exctable[i].exc)  ||
	    PyDict_SetItemString(mydict, exctable[i].cp_name, *exctable[i].exc)||
	    PyDict_SetItemString(bdict, exctable[i].cp_name, *exctable[i].exc))
	{
	    /*Py_FatalError("Module dictionary insertion problem.");*/
	    Py_FatalError("�L�k�ⲧ�`�B�z�[��Ҳզr�夤");
	}
    }

    /* Now we need to pre-allocate a MemoryError instance */
    args = Py_BuildValue("()");
    if (!args ||
	!(PyExc_MemoryErrorInst = PyEval_CallObject(PyExc_MemoryError, args)))
    {
	/*Py_FatalError("Cannot pre-allocate MemoryError instance\n");*/
	Py_FatalError("�L�k�w���t�m �O���鲧�` \n");
    }
    Py_DECREF(args);

    /* We're done with __builtin__ */
    Py_DECREF(bltinmod);
}


DL_EXPORT(void)
fini_exceptions(void)
{
    int i;

    Py_XDECREF(PyExc_MemoryErrorInst);
    PyExc_MemoryErrorInst = NULL;

    for (i=0; exctable[i].name; i++) {
	if(*exctable[i].exc!=NULL) {
	/* clear the class's dictionary, freeing up circular references
	 * between the class and its methods.
	 */
	PyObject* cdict = PyObject_GetAttrString(*exctable[i].exc, "__dict__");
	PyDict_Clear(cdict);
	Py_DECREF(cdict);

	/* Now decref the exception class */
	Py_XDECREF(*exctable[i].exc);
	*exctable[i].exc = NULL;
    	}
    }
}
