
/* Type object implementation */

#include "Python.h"

/* Type object implementation */

static PyObject *
type_getattr(PyTypeObject *t, char *name)
{
	if (strcmp(name, "__name__") == 0 ||
	 	strcmp(name, "__名稱__") == 0)
		return PyString_FromStringAndEncode(t->tp_name, Source_Encoding);
	if (strcmp(name, "__doc__") == 0  ||
		strcmp(name, "__說明__") == 0) {
		char *doc = t->tp_doc;
		if (doc != NULL)
			return PyString_FromStringAndEncode(doc, Source_Encoding);
		Py_INCREF(Py_None);
		return Py_None;
	}
	if (strcmp(name, "__members__") == 0 ||
		strcmp(name, "__成員__") == 0)
		return Py_BuildValue("[ssss]", "__doc__", "__name__",
			"__說明__","__名稱__");
	PyErr_SetString(PyExc_AttributeError, name);
	return NULL;
}

static PyObject *
type_repr(PyTypeObject *v)
{
	char buf[100];
	/* why this doesn't work ???
	sprintf(buf, "<類別 '%.80s'>", v->tp_name);
	*/
	sprintf(buf, "<類別 '%s'>", v->tp_name);
	return PyString_FromStringAndEncode(buf, Source_Encoding);
}

PyTypeObject PyType_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,			/* Number of items for varobject */
	"類型",			/* Name of this type */
	sizeof(PyTypeObject),	/* Basic object size */
	0,			/* Item size for varobject */
	0,			/*tp_dealloc*/
	0,			/*tp_print*/
	(getattrfunc)type_getattr, /*tp_getattr*/
	0,			/*tp_setattr*/
	0,			/*tp_compare*/
	(reprfunc)type_repr,	/*tp_repr*/
	0,			/*tp_as_number*/
	0,			/*tp_as_sequence*/
	0,			/*tp_as_mapping*/
	0,			/*tp_hash*/
	0,			/*tp_call*/
	0,			/*tp_str*/
	0,			/*tp_xxx1*/
	0,			/*tp_xxx2*/
	0,			/*tp_xxx3*/
	0,			/*tp_xxx4*/
	"定義某一類別實體的特定行為",
};
