
/* Module object implementation */

#include "Python.h"

typedef struct {
	PyObject_HEAD
	PyObject *md_dict;
} PyModuleObject;

PyObject *
PyModule_New(char *name)
{
	PyModuleObject *m;
	PyObject *nameobj;
	m = PyObject_NEW(PyModuleObject, &PyModule_Type);
	if (m == NULL)
		return NULL;
	nameobj = PyString_FromStringAndEncode(name, Source_Encoding);
	m->md_dict = PyDict_New();
	PyObject_GC_Init(m);
	if (m->md_dict == NULL || nameobj == NULL)
		goto fail;
	if (PyDict_SetItemString(m->md_dict, "__name__", nameobj) != 0)
		goto fail;
	if (PyDict_SetItemString(m->md_dict, "__�W��__", nameobj) != 0)
		goto fail;
	if (PyDict_SetItemString(m->md_dict, "__doc__", Py_None) != 0)
		goto fail;
	if (PyDict_SetItemString(m->md_dict, "__����__", Py_None) != 0)
		goto fail;
	Py_DECREF(nameobj);
	return (PyObject *)m;

 fail:
	Py_XDECREF(nameobj);
	Py_DECREF(m);
	return NULL;
}

PyObject *
PyModule_GetDict(PyObject *m)
{
	if (!PyModule_Check(m)) {
		PyErr_BadInternalCall();
		return NULL;
	}
	return ((PyModuleObject *)m) -> md_dict;
}

char *
PyModule_GetName(PyObject *m)
{
	PyObject *nameobj;
	if (!PyModule_Check(m)) {
		PyErr_BadArgument();
		return NULL;
	}
	nameobj = PyDict_GetItemString(((PyModuleObject *)m)->md_dict,
				       "__name__");
	if (nameobj == NULL || !PyString_Check(nameobj)) {
	    nameobj = PyDict_GetItemString(((PyModuleObject *)m)->md_dict,
				       "__�W��__");
	    if (nameobj == NULL || !PyString_Check(nameobj)) {
		/*PyErr_SetString(PyExc_SystemError, "nameless module");*/
		PyErr_SetString(PyExc_SystemError, "�ҲըS���W��");
		return NULL;
	    }
	}
	return PyString_AsString(nameobj);
}

char *
PyModule_GetFilename(PyObject *m)
{
	PyObject *fileobj;
	if (!PyModule_Check(m)) {
		PyErr_BadArgument();
		return NULL;
	}
	fileobj = PyDict_GetItemString(((PyModuleObject *)m)->md_dict,
				       "__file__");
	if (fileobj == NULL || !PyString_Check(fileobj)) {
		fileobj = PyDict_GetItemString(((PyModuleObject *)m)->md_dict,
				       "__�ɮ�__");
		if (fileobj == NULL || !PyString_Check(fileobj)) {
		   /*PyErr_SetString(PyExc_SystemError, "module filename missing");*/
		   PyErr_SetString(PyExc_SystemError, "�ҲըS���ɮצW");
		   return NULL;
		}
	}
	return PyString_AsString(fileobj);
}

void
_PyModule_Clear(PyObject *m)
{
	/* To make the execution order of destructors for global
	   objects a bit more predictable, we first zap all objects
	   whose name starts with a single underscore, before we clear
	   the entire dictionary.  We zap them by replacing them with
	   None, rather than deleting them from the dictionary, to
	   avoid rehashing the dictionary (to some extent). */

	int pos;
	PyObject *key, *value;
	PyObject *d;

	d = ((PyModuleObject *)m)->md_dict;

	/* First, clear only names starting with a single underscore */
	pos = 0;
	while (PyDict_Next(d, &pos, &key, &value)) {
		if (value != Py_None && PyString_Check(key)) {
			char *s = PyString_AsString(key);
			if (s[0] == '_' && s[1] != '_') {
				if (Py_VerboseFlag > 1)
				    /*PySys_WriteStderr("#   clear[1] %s\n", s);*/
				    PySys_WriteStderr("#   �M��/clear[1] %s\n", s);
				PyDict_SetItem(d, key, Py_None);
			}
		}
	}

	/* Next, clear all names except for __builtins__ */
	pos = 0;
	while (PyDict_Next(d, &pos, &key, &value)) {
		if (value != Py_None && PyString_Check(key)) {
			char *s = PyString_AsString(key);
			if (s[0] != '_' || strcmp(s, "__builtins__") != 0) {
				if (Py_VerboseFlag > 1)
				    /*PySys_WriteStderr("#   clear[2] %s\n", s);*/
				    PySys_WriteStderr("#   �M��/clear[2] %s\n", s);
				PyDict_SetItem(d, key, Py_None);
			}
		}
	}

	/* Note: we leave __builtins__ in place, so that destructors
	   of non-global objects defined in this module can still use
	   builtins, in particularly 'None'. */

}

/* Methods */

static void
module_dealloc(PyModuleObject *m)
{
	PyObject_GC_Fini(m);
	if (m->md_dict != NULL) {
		_PyModule_Clear((PyObject *)m);
		Py_DECREF(m->md_dict);
	}
	PyObject_DEL(PyObject_AS_GC(m));
}

static PyObject *
module_repr(PyModuleObject *m)
{
	char buf[400];
	char *name;
	char *filename;
	name = PyModule_GetName((PyObject *)m);
	if (name == NULL) {
		PyErr_Clear();
		name = "?";
	}
	filename = PyModule_GetFilename((PyObject *)m);
	if (filename == NULL) {
		PyErr_Clear();
		/*sprintf(buf, "<module '%.80s' (built-in)>", name);*/
		sprintf(buf, "<�Ҳ� '%.80s' (����)>", name);
	} else {
		/*sprintf(buf, "<module '%.80s' from '%.255s'>", name, filename);*/
		sprintf(buf, "<�Ҳ� '%.80s' �Ӧ� '%.255s'>", name, filename);
	}

	return PyString_FromStringAndEncode(buf, Source_Encoding);
}

static PyObject *
module_getattr(PyModuleObject *m, char *name)
{
	PyObject *res;
	char* modname;
	if (strcmp(name, "__dict__") == 0 ||
	    strcmp(name, "__�r��__") == 0 ) {
		Py_INCREF(m->md_dict);
		return m->md_dict;
	}
	res = PyDict_GetItemString(m->md_dict, name);
	if (res == NULL) {
		modname = PyModule_GetName((PyObject *)m);
		if (modname == NULL) {
			PyErr_Clear();
			modname = "?";
		}
		PyErr_Format(PyExc_AttributeError,
			     /*"'%.50s' module has no attribute '%.400s'",*/
			     "'%.50s' �ҲըS�� '%.400s' �o�ݩ�",
			     modname, name);
	}
	else
		Py_INCREF(res);
	return res;
}

static int
module_setattr(PyModuleObject *m, char *name, PyObject *v)
{
	char* modname;
	if (name[0] == '_' && (strcmp(name, "__dict__") == 0 ||
			       strcmp(name, "__�r��__") == 0 )) {
		PyErr_SetString(PyExc_TypeError,
				/*"read-only special attribute");*/
				"�o�O�Ӱ�Ū���S���ݩ�");
		return -1;
	}
	if (v == NULL) {
		int rv = PyDict_DelItemString(m->md_dict, name);
		if (rv < 0) {
			modname = PyModule_GetName((PyObject *)m);
			if (modname == NULL) {
				PyErr_Clear();
				modname = "?";
			}
			PyErr_Format(PyExc_AttributeError,
				     /*"'%.50s' module has no attribute '%.400s'",*/
				     "'%.50s' �ҲըS�� '%.400s' �ݩ�",
				     modname, name);
		}
		return rv;
	}
	else
		return PyDict_SetItemString(m->md_dict, name, v);
}

/* We only need a traverse function, no clear function: If the module
   is in a cycle, md_dict will be cleared as well, which will break
   the cycle. */
static int
module_traverse(PyModuleObject *m, visitproc visit, void *arg)
{
	if (m->md_dict != NULL)
		return visit(m->md_dict, arg);
	return 0;
}

PyTypeObject PyModule_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,			/*ob_size*/
	"�Ҳ�",		/*tp_name*/
	sizeof(PyModuleObject) + PyGC_HEAD_SIZE,	/*tp_size*/
	0,			/*tp_itemsize*/
	(destructor)module_dealloc, /*tp_dealloc*/
	0,			/*tp_print*/
	(getattrfunc)module_getattr, /*tp_getattr*/
	(setattrfunc)module_setattr, /*tp_setattr*/
	0,			/*tp_compare*/
	(reprfunc)module_repr, /*tp_repr*/
	0,			/*tp_as_number*/
	0,			/*tp_as_sequence*/
	0,		/*tp_as_mapping*/
	0,		/* tp_hash */
	0,		/* tp_call */
	0,		/* tp_str */
	0,		/* tp_getattro */
	0,		/* tp_setattro */
	0,		/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_GC, /*tp_flags*/
	0,		/* tp_doc */
	(traverseproc)module_traverse,	/* tp_traverse */
};
