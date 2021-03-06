/* Cell object implementation */

#include "Python.h"

PyObject *
PyCell_New(PyObject *obj)
{
	PyCellObject *op;

	op = (PyCellObject *)PyObject_New(PyCellObject, &PyCell_Type);
	op->ob_ref = obj;
	Py_XINCREF(obj);

	PyObject_GC_Init(op);
	return (PyObject *)op;
}

PyObject *
PyCell_Get(PyObject *op)
{
	if (!PyCell_Check(op)) {
		PyErr_BadInternalCall();
		return NULL;
	}
	Py_XINCREF(((PyCellObject*)op)->ob_ref);
	return PyCell_GET(op);
}

int
PyCell_Set(PyObject *op, PyObject *obj)
{
	if (!PyCell_Check(op)) {
		PyErr_BadInternalCall();
		return -1;
	}
	Py_XDECREF(((PyCellObject*)op)->ob_ref);
	Py_XINCREF(obj);
	PyCell_SET(op, obj);
	return 0;
}

static void
cell_dealloc(PyCellObject *op)
{
	PyObject_GC_Fini(op);
	Py_XDECREF(op->ob_ref);
	PyObject_Del(op);
}

static int
cell_compare(PyCellObject *a, PyCellObject *b)
{
	if (a->ob_ref == NULL) {
		if (b->ob_ref == NULL)
			return 0;
		return -1;
	} else if (b->ob_ref == NULL)
		return 1;
	return PyObject_Compare(a->ob_ref, b->ob_ref);
}

static PyObject *
cell_repr(PyCellObject *op)
{
	char buf[256];

	if (op->ob_ref == NULL)
		/*sprintf(buf, "<cell at %p: empty>", op);*/
		sprintf(buf, "<胞 %p: 空的>", op);
	else
		/*sprintf(buf, "<cell at %p: %.80s object at %p>",*/
		sprintf(buf, "<胞 %p: %.80s 實體於 %p>",
			op, op->ob_ref->ob_type->tp_name, op->ob_ref);
	return PyString_FromStringAndEncode(buf, Source_Encoding);
}

static int
cell_traverse(PyCellObject *op, visitproc visit, void *arg)
{
	if (op->ob_ref)
		return visit(op->ob_ref, arg);
	return 0;
}

static int
cell_clear(PyCellObject *op)
{
	Py_XDECREF(op->ob_ref);
	op->ob_ref = NULL;
	return 0;
}

PyTypeObject PyCell_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"胞",
	sizeof(PyCellObject) + PyGC_HEAD_SIZE,
	0,
	(destructor)cell_dealloc,               /* tp_dealloc */
	0,                                      /* tp_print */
	0,	                                /* tp_getattr */
	0,					/* tp_setattr */
	(cmpfunc)cell_compare,			/* tp_compare */
	(reprfunc)cell_repr,			/* tp_repr */
	0,					/* tp_as_number */
	0,			                /* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,					/* tp_hash */
	0,					/* tp_call */
	0,					/* tp_str */
	0,					/* tp_getattro */
	0,					/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_GC,	/* tp_flags */
 	0,					/* tp_doc */
 	(traverseproc)cell_traverse,		/* tp_traverse */
 	(inquiry)cell_clear,			/* tp_clear */
};
