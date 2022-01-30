/* Abstract Object Interface (many thanks to Jim Fulton) */


#include "Python.h"
#include <ctype.h>
#include "structmember.h" /* we need the offsetof() macro from there */

#define NEW_STYLE_NUMBER(o) PyType_HasFeature((o)->ob_type, \
				Py_TPFLAGS_CHECKTYPES)

/* Shorthands to return certain errors */

static PyObject *
type_error(const char *msg)
{
	PyErr_SetString(PyExc_TypeError, msg);
	return NULL;
}

static PyObject *
null_error(void)
{
	if (!PyErr_Occurred())
		/*PyErr_SetString(PyExc_SystemError,
				"null argument to internal routine");*/
		PyErr_SetString(PyExc_SystemError,
				"�ǰe null �ѼƵ������{��");
	return NULL;
}

/* Operations on any object */

int
PyObject_Cmp(PyObject *o1, PyObject *o2, int *result)
{
	int r;

	if (o1 == NULL || o2 == NULL) {
		null_error();
		return -1;
	}
	r = PyObject_Compare(o1, o2);
	if (PyErr_Occurred())
		return -1;
	*result = r;
	return 0;
}

PyObject *
PyObject_Type(PyObject *o)
{
	PyObject *v;

	if (o == NULL)
		return null_error();
	v = (PyObject *)o->ob_type;
	Py_INCREF(v);
	return v;
}

int
PyObject_Size(PyObject *o)
{
	PySequenceMethods *m;

	if (o == NULL) {
		null_error();
		return -1;
	}

	m = o->ob_type->tp_as_sequence;
	if (m && m->sq_length)
		return m->sq_length(o);

	return PyMapping_Size(o);
}

#undef PyObject_Length
int
PyObject_Length(PyObject *o)
{
	return PyObject_Size(o);
}
#define PyObject_Length PyObject_Size

PyObject *
PyObject_GetItem(PyObject *o, PyObject *key)
{
	PyMappingMethods *m;

	if (o == NULL || key == NULL)
		return null_error();

	m = o->ob_type->tp_as_mapping;
	if (m && m->mp_subscript)
		return m->mp_subscript(o, key);

	if (o->ob_type->tp_as_sequence) {
		if (PyInt_Check(key))
			return PySequence_GetItem(o, PyInt_AsLong(key));
		else if (PyLong_Check(key)) {
			long key_value = PyLong_AsLong(key);
			if (key_value == -1 && PyErr_Occurred())
				return NULL;
			return PySequence_GetItem(o, key_value);
		}
		/*return type_error("sequence index must be integer");*/
		return type_error("�ǦC�����ޥ��������");
	}

	/*return type_error("unsubscriptable object");*/
	return type_error("���餣�������޶�");
}

int
PyObject_SetItem(PyObject *o, PyObject *key, PyObject *value)
{
	PyMappingMethods *m;

	if (o == NULL || key == NULL || value == NULL) {
		null_error();
		return -1;
	}
	m = o->ob_type->tp_as_mapping;
	if (m && m->mp_ass_subscript)
		return m->mp_ass_subscript(o, key, value);

	if (o->ob_type->tp_as_sequence) {
		if (PyInt_Check(key))
			return PySequence_SetItem(o, PyInt_AsLong(key), value);
		else if (PyLong_Check(key)) {
			long key_value = PyLong_AsLong(key);
			if (key_value == -1 && PyErr_Occurred())
				return -1;
			return PySequence_SetItem(o, key_value, value);
		}
		/*type_error("sequence index must be integer");*/
		type_error("�ǦC�����ޥ��������");
		return -1;
	}

	/*type_error("object does not support item assignment");*/
	type_error("����l�����i���");
	return -1;
}

int
PyObject_DelItem(PyObject *o, PyObject *key)
{
	PyMappingMethods *m;

	if (o == NULL || key == NULL) {
		null_error();
		return -1;
	}
	m = o->ob_type->tp_as_mapping;
	if (m && m->mp_ass_subscript)
		return m->mp_ass_subscript(o, key, (PyObject*)NULL);

	if (o->ob_type->tp_as_sequence) {
		if (PyInt_Check(key))
			return PySequence_DelItem(o, PyInt_AsLong(key));
		else if (PyLong_Check(key)) {
			long key_value = PyLong_AsLong(key);
			if (key_value == -1 && PyErr_Occurred())
				return -1;
			return PySequence_DelItem(o, key_value);
		}
		/*type_error("sequence index must be integer");*/
		type_error("�ǦC���ޥ��������");
		return -1;
	}

	/*type_error("object does not support item deletion");*/
	type_error("����l�����i�R��");
	return -1;
}

int PyObject_AsCharBuffer(PyObject *obj,
			  const char **buffer,
			  int *buffer_len)
{
	PyBufferProcs *pb;
	const char *pp;
	int len;

	if (obj == NULL || buffer == NULL || buffer_len == NULL) {
		null_error();
		return -1;
	}
	pb = obj->ob_type->tp_as_buffer;
	if ( pb == NULL ||
	     pb->bf_getcharbuffer == NULL ||
	     pb->bf_getsegcount == NULL ) {
		/*PyErr_SetString(PyExc_TypeError,
				"expected a character buffer object");*/
		PyErr_SetString(PyExc_TypeError,
				"���ӬO�@�Ӧr�żȦs����");
		goto onError;
	}
	if ( (*pb->bf_getsegcount)(obj,NULL) != 1 ) {
		/*PyErr_SetString(PyExc_TypeError,
				"expected a single-segment buffer object");*/
		PyErr_SetString(PyExc_TypeError,
				"���ӬO�@�ӳ�q�Ȧs����");
		goto onError;
	}
	len = (*pb->bf_getcharbuffer)(obj,0,&pp);
	if (len < 0)
		goto onError;
	*buffer = pp;
	*buffer_len = len;
	return 0;

 onError:
	return -1;
}

int PyObject_AsReadBuffer(PyObject *obj,
			  const void **buffer,
			  int *buffer_len)
{
	PyBufferProcs *pb;
	void *pp;
	int len;

	if (obj == NULL || buffer == NULL || buffer_len == NULL) {
		null_error();
		return -1;
	}
	pb = obj->ob_type->tp_as_buffer;
	if ( pb == NULL ||
	     pb->bf_getreadbuffer == NULL ||
	     pb->bf_getsegcount == NULL ) {
		/*PyErr_SetString(PyExc_TypeError,
				"expected a readable buffer object");*/
		PyErr_SetString(PyExc_TypeError,
				"�����@�iŪ�Ȧs����");
		goto onError;
	}
	if ( (*pb->bf_getsegcount)(obj,NULL) != 1 ) {
		/*PyErr_SetString(PyExc_TypeError,
				"expected a single-segment buffer object");*/
		PyErr_SetString(PyExc_TypeError,
				"���ӬO�@�ӳ�q�Ȧs����");
		goto onError;
	}
	len = (*pb->bf_getreadbuffer)(obj,0,&pp);
	if (len < 0)
		goto onError;
	*buffer = pp;
	*buffer_len = len;
	return 0;

 onError:
	return -1;
}

int PyObject_AsWriteBuffer(PyObject *obj,
			   void **buffer,
			   int *buffer_len)
{
	PyBufferProcs *pb;
	void*pp;
	int len;

	if (obj == NULL || buffer == NULL || buffer_len == NULL) {
		null_error();
		return -1;
	}
	pb = obj->ob_type->tp_as_buffer;
	if ( pb == NULL ||
	     pb->bf_getwritebuffer == NULL ||
	     pb->bf_getsegcount == NULL ) {
		/*PyErr_SetString(PyExc_TypeError,
				"expected a writeable buffer object");*/
		PyErr_SetString(PyExc_TypeError,
				"�����@�i�g���Ȧs����");
		goto onError;
	}
	if ( (*pb->bf_getsegcount)(obj,NULL) != 1 ) {
		/*PyErr_SetString(PyExc_TypeError,
				"expected a single-segment buffer object");*/
		PyErr_SetString(PyExc_TypeError,
				"���ӬO�@�ӳ�q�Ȧs����");
		goto onError;
	}
	len = (*pb->bf_getwritebuffer)(obj,0,&pp);
	if (len < 0)
		goto onError;
	*buffer = pp;
	*buffer_len = len;
	return 0;

 onError:
	return -1;
}

/* Operations on numbers */

int
PyNumber_Check(PyObject *o)
{
	return o && o->ob_type->tp_as_number;
}

/* Binary operators */

/* New style number protocol support */

#define NB_SLOT(x) offsetof(PyNumberMethods, x)
#define NB_BINOP(nb_methods, slot) \
		((binaryfunc*)(& ((char*)nb_methods)[slot] ))
#define NB_TERNOP(nb_methods, slot) \
		((ternaryfunc*)(& ((char*)nb_methods)[slot] ))

/*
  Calling scheme used for binary operations:

  v	w	Action
  -------------------------------------------------------------------
  new	new	v.op(v,w), w.op(v,w)
  new	old	v.op(v,w), coerce(v,w), v.op(v,w)
  old	new	w.op(v,w), coerce(v,w), v.op(v,w)
  old	old	coerce(v,w), v.op(v,w)

  Legend:
  -------
  * new == new style number
  * old == old style number
  * Action indicates the order in which operations are tried until either
    a valid result is produced or an error occurs.

 */

static PyObject *
binary_op1(PyObject *v, PyObject *w, const int op_slot)
{
	PyObject *x;
	binaryfunc *slot;
	if (v->ob_type->tp_as_number != NULL && NEW_STYLE_NUMBER(v)) {
		slot = NB_BINOP(v->ob_type->tp_as_number, op_slot);
		if (*slot) {
			x = (*slot)(v, w);
			if (x != Py_NotImplemented) {
				return x;
			}
			Py_DECREF(x); /* can't do it */
		}
		if (v->ob_type == w->ob_type) {
			goto binop_error;
		}
	}
	if (w->ob_type->tp_as_number != NULL && NEW_STYLE_NUMBER(w)) {
		slot = NB_BINOP(w->ob_type->tp_as_number, op_slot);
		if (*slot) {
			x = (*slot)(v, w);
			if (x != Py_NotImplemented) {
				return x;
			}
			Py_DECREF(x); /* can't do it */
		}
	}
	if (!NEW_STYLE_NUMBER(v) || !NEW_STYLE_NUMBER(w)) {
		int err = PyNumber_CoerceEx(&v, &w);
		if (err < 0) {
			return NULL;
		}
		if (err == 0) {
			PyNumberMethods *mv = v->ob_type->tp_as_number;
			if (mv) {
				slot = NB_BINOP(mv, op_slot);
				if (*slot) {
					PyObject *x = (*slot)(v, w);
					Py_DECREF(v);
					Py_DECREF(w);
					return x;
				}
			}
			/* CoerceEx incremented the reference counts */
			Py_DECREF(v);
			Py_DECREF(w);
		}
	}
binop_error:
	Py_INCREF(Py_NotImplemented);
	return Py_NotImplemented;
}
	    
static PyObject *
binary_op(PyObject *v, PyObject *w, const int op_slot, const char *op_name)
{
	PyObject *result = binary_op1(v, w, op_slot);
	if (result == Py_NotImplemented) {
		Py_DECREF(Py_NotImplemented);
		PyErr_Format(PyExc_TypeError, 
				/*"unsupported operand type(s) for %s", op_name);*/
				"%s ���䴩���������B�⤸", op_name);
		return NULL;
	}
	return result;
}


/*
  Calling scheme used for ternary operations:

  v	w	z	Action
  -------------------------------------------------------------------
  new	new	new	v.op(v,w,z), w.op(v,w,z), z.op(v,w,z)
  new	old	new	v.op(v,w,z), z.op(v,w,z), coerce(v,w,z), v.op(v,w,z)
  old	new	new	w.op(v,w,z), z.op(v,w,z), coerce(v,w,z), v.op(v,w,z)
  old	old	new	z.op(v,w,z), coerce(v,w,z), v.op(v,w,z)
  new	new	old	v.op(v,w,z), w.op(v,w,z), coerce(v,w,z), v.op(v,w,z)
  new	old	old	v.op(v,w,z), coerce(v,w,z), v.op(v,w,z)
  old	new	old	w.op(v,w,z), coerce(v,w,z), v.op(v,w,z)
  old	old	old	coerce(v,w,z), v.op(v,w,z)

  Legend:
  -------
  * new == new style number
  * old == old style number
  * Action indicates the order in which operations are tried until either
    a valid result is produced or an error occurs.
  * coerce(v,w,z) actually does: coerce(v,w), coerce(v,z), coerce(w,z) and
    only if z != Py_None; if z == Py_None, then it is treated as absent
    variable and only coerce(v,w) is tried.

 */

static PyObject *
ternary_op(PyObject *v,
	   PyObject *w,
	   PyObject *z,
	   const int op_slot,
	   const char *op_name)
{
	PyNumberMethods *mv, *mw, *mz;
	register PyObject *x = NULL;
	register ternaryfunc *slot;
	
	mv = v->ob_type->tp_as_number;
	if (mv != NULL && NEW_STYLE_NUMBER(v)) {
		/* try v.op(v,w,z) */
		slot = NB_TERNOP(mv, op_slot);
		if (*slot) {
			x = (*slot)(v, w, z);
			if (x != Py_NotImplemented)
				return x;
			/* Can't do it... fall through */
			Py_DECREF(x);
		}
		if (v->ob_type == w->ob_type &&
				(z == Py_None || z->ob_type == v->ob_type)) {
			goto ternary_error;
		}
	}
	mw = w->ob_type->tp_as_number;
	if (mw != NULL && NEW_STYLE_NUMBER(w)) {
		/* try w.op(v,w,z) */
		slot = NB_TERNOP(mw,op_slot);
		if (*slot) {
			x = (*slot)(v, w, z);
			if (x != Py_NotImplemented)
				return x;
			/* Can't do it... fall through */
			Py_DECREF(x);
		}
		if (NEW_STYLE_NUMBER(v) &&
				(z == Py_None || z->ob_type == v->ob_type)) {
			goto ternary_error;
		}
	}
	mz = z->ob_type->tp_as_number;
	if (mz != NULL && NEW_STYLE_NUMBER(z)) {
		/* try: z.op(v,w,z) */
		slot = NB_TERNOP(mz, op_slot);
		if (*slot) {
			x = (*slot)(v, w, z);
			if (x != Py_NotImplemented)
				return x;
			/* Can't do it... fall through */
			Py_DECREF(x);
		}
	}

	if (!NEW_STYLE_NUMBER(v) || !NEW_STYLE_NUMBER(w) ||
			(z != Py_None && !NEW_STYLE_NUMBER(z))) {
		/* we have an old style operand, coerce */
		PyObject *v1, *z1, *w2, *z2;
		int c;
		
		c = PyNumber_Coerce(&v, &w);
		if (c != 0)
			goto error3;

		/* Special case: if the third argument is None, it is
		   treated as absent argument and not coerced. */
		if (z == Py_None) {
			if (v->ob_type->tp_as_number) {
				slot = NB_TERNOP(v->ob_type->tp_as_number,
						 op_slot);
				if (*slot)
					x = (*slot)(v, w, z);
				else
					c = -1;
			}
			else
				c = -1;
			goto error2;
		}
		v1 = v;
		z1 = z;
		c = PyNumber_Coerce(&v1, &z1);
		if (c != 0)
			goto error2;
		w2 = w;
		z2 = z1;
		c = PyNumber_Coerce(&w2, &z2);
		if (c != 0)
			goto error1;

		if (v1->ob_type->tp_as_number != NULL) {
			slot = NB_TERNOP(v1->ob_type->tp_as_number,
					 op_slot);
			if (*slot)
				x = (*slot)(v1, w2, z2);
			else
				c = -1;
		}
		else
			c = -1;

		Py_DECREF(w2);
		Py_DECREF(z2);
	error1:
		Py_DECREF(v1);
		Py_DECREF(z1);
	error2:
		Py_DECREF(v);
		Py_DECREF(w);
	error3:
		if (c >= 0)
			return x;
	}
	
ternary_error:
	/*PyErr_Format(PyExc_TypeError, "unsupported operand type(s) for %s",*/
	PyErr_Format(PyExc_TypeError, "%s ���䴩�o���������B�⤸",
			op_name);
	return NULL;
}

#define BINARY_FUNC(func, op, op_name) \
    PyObject * \
    func(PyObject *v, PyObject *w) { \
	    return binary_op(v, w, NB_SLOT(op), op_name); \
    }

BINARY_FUNC(PyNumber_Or, nb_or, "|")
BINARY_FUNC(PyNumber_Xor, nb_xor, "^")
BINARY_FUNC(PyNumber_And, nb_and, "&")
BINARY_FUNC(PyNumber_Lshift, nb_lshift, "<<")
BINARY_FUNC(PyNumber_Rshift, nb_rshift, ">>")
BINARY_FUNC(PyNumber_Subtract, nb_subtract, "-")
BINARY_FUNC(PyNumber_Multiply, nb_multiply, "*")
BINARY_FUNC(PyNumber_Divide, nb_divide, "/")
BINARY_FUNC(PyNumber_Divmod, nb_divmod, "divmod()")

PyObject *
PyNumber_Add(PyObject *v, PyObject *w)
{
	PyObject *result = binary_op1(v, w, NB_SLOT(nb_add));
	if (result == Py_NotImplemented) {
		PySequenceMethods *m = v->ob_type->tp_as_sequence;
		Py_DECREF(Py_NotImplemented);
		if (m && m->sq_concat) {
			result = (*m->sq_concat)(v, w);
		}
                else {
                    PyErr_SetString(PyExc_TypeError,
                                    /*"unsupported operand types for +");*/
                                    " + ���䴩�o���������B�⤸");
                    result = NULL;
                }
	}
	return result;
}

PyObject *
PyNumber_Remainder(PyObject *v, PyObject *w)
{
	if (PyString_Check(v))
		return PyString_Format(v, w);
	else if (PyUnicode_Check(v))
		return PyUnicode_Format(v, w);
	return binary_op(v, w, NB_SLOT(nb_remainder), "%");
}

PyObject *
PyNumber_Power(PyObject *v, PyObject *w, PyObject *z)
{
	return ternary_op(v, w, z, NB_SLOT(nb_power), "** or pow()");
}

/* Binary in-place operators */

/* The in-place operators are defined to fall back to the 'normal',
   non in-place operations, if the in-place methods are not in place.

   - If the left hand object has the appropriate struct members, and
     they are filled, call the appropriate function and return the
     result.  No coercion is done on the arguments; the left-hand object
     is the one the operation is performed on, and it's up to the
     function to deal with the right-hand object.
     
   - Otherwise, in-place modification is not supported. Handle it exactly as
     a non in-place operation of the same kind.

   */

#define HASINPLACE(t) PyType_HasFeature((t)->ob_type, Py_TPFLAGS_HAVE_INPLACEOPS)

static PyObject *
binary_iop(PyObject *v, PyObject *w, const int iop_slot, const int op_slot,
		const char *op_name)
{
	PyNumberMethods *mv = v->ob_type->tp_as_number;
	if (mv != NULL && HASINPLACE(v)) {
		binaryfunc *slot = NB_BINOP(mv, iop_slot);
		if (*slot) {
			PyObject *x = (*slot)(v, w);
			if (x != Py_NotImplemented) {
				return x;
			}
			Py_DECREF(x);
		}
	}
	return binary_op(v, w, op_slot, op_name);
}

#define INPLACE_BINOP(func, iop, op, op_name) \
	PyObject * \
	func(PyObject *v, PyObject *w) { \
		return binary_iop(v, w, NB_SLOT(iop), NB_SLOT(op), op_name); \
	}

INPLACE_BINOP(PyNumber_InPlaceOr, nb_inplace_or, nb_or, "|=")
INPLACE_BINOP(PyNumber_InPlaceXor, nb_inplace_xor, nb_xor, "^=")
INPLACE_BINOP(PyNumber_InPlaceAnd, nb_inplace_and, nb_and, "&=")
INPLACE_BINOP(PyNumber_InPlaceLshift, nb_inplace_lshift, nb_lshift, "<<=")
INPLACE_BINOP(PyNumber_InPlaceRshift, nb_inplace_rshift, nb_rshift, ">>=")
INPLACE_BINOP(PyNumber_InPlaceSubtract, nb_inplace_subtract, nb_subtract, "-=")
INPLACE_BINOP(PyNumber_InPlaceDivide, nb_inplace_divide, nb_divide, "/=")

PyObject *
PyNumber_InPlaceAdd(PyObject *v, PyObject *w)
{
	binaryfunc f = NULL;

	if (v->ob_type->tp_as_sequence != NULL) {
		if (HASINPLACE(v))
			f = v->ob_type->tp_as_sequence->sq_inplace_concat;
		if (f == NULL)
			f = v->ob_type->tp_as_sequence->sq_concat;
		if (f != NULL)
			return (*f)(v, w);
	}
	return binary_iop(v, w, NB_SLOT(nb_inplace_add), NB_SLOT(nb_add), "+=");
}

PyObject *
PyNumber_InPlaceMultiply(PyObject *v, PyObject *w)
{
	PyObject * (*g)(PyObject *, int) = NULL;
	if (HASINPLACE(v) && v->ob_type->tp_as_sequence &&
		(g = v->ob_type->tp_as_sequence->sq_inplace_repeat)) {
		long n;
		if (PyInt_Check(w)) {
			n  = PyInt_AsLong(w);
		}
		else if (PyLong_Check(w)) {
			n = PyLong_AsLong(w);
			if (n == -1 && PyErr_Occurred())
				return NULL;
		}
		else {
			/*return type_error("can't multiply sequence to non-int");*/
			return type_error("�ǦC���୼�H�D���");
		}
		return (*g)(v, (int)n);
	}
	return binary_iop(v, w, NB_SLOT(nb_inplace_multiply),
				NB_SLOT(nb_multiply), "*=");
}



PyObject *
PyNumber_InPlaceRemainder(PyObject *v, PyObject *w)
{
	if (PyString_Check(v))
		return PyString_Format(v, w);
	else if (PyUnicode_Check(v))
		return PyUnicode_Format(v, w);
	else
		return binary_iop(v, w, NB_SLOT(nb_inplace_remainder),
					NB_SLOT(nb_remainder), "%=");
}


PyObject *
PyNumber_InPlacePower(PyObject *v, PyObject *w, PyObject *z)
{
	if (HASINPLACE(v) && v->ob_type->tp_as_number &&
	    v->ob_type->tp_as_number->nb_inplace_power != NULL) {
		return ternary_op(v, w, z, NB_SLOT(nb_inplace_power), "**=");
	}
	else {
		return ternary_op(v, w, z, NB_SLOT(nb_power), "**=");
	}
}


/* Unary operators and functions */

PyObject *
PyNumber_Negative(PyObject *o)
{
	PyNumberMethods *m;

	if (o == NULL)
		return null_error();
	m = o->ob_type->tp_as_number;
	if (m && m->nb_negative)
		return (*m->nb_negative)(o);

	/*return type_error("bad operand type for unary -");*/
	return type_error("- ���줣���T���B�⤸");
}

PyObject *
PyNumber_Positive(PyObject *o)
{
	PyNumberMethods *m;

	if (o == NULL)
		return null_error();
	m = o->ob_type->tp_as_number;
	if (m && m->nb_positive)
		return (*m->nb_positive)(o);

	/*return type_error("bad operand type for unary +");*/
	return type_error("+ ����@���T���B�⤸");
}

PyObject *
PyNumber_Invert(PyObject *o)
{
	PyNumberMethods *m;

	if (o == NULL)
		return null_error();
	m = o->ob_type->tp_as_number;
	if (m && m->nb_invert)
		return (*m->nb_invert)(o);

	/*return type_error("bad operand type for unary ~");*/
	return type_error("~ ���줣���T���B�⤸");
}

PyObject *
PyNumber_Absolute(PyObject *o)
{
	PyNumberMethods *m;

	if (o == NULL)
		return null_error();
	m = o->ob_type->tp_as_number;
	if (m && m->nb_absolute)
		return m->nb_absolute(o);

	/*return type_error("bad operand type for abs()");*/
	return type_error("abs()/�����() ���줣���T���B�⤸");
}

/* Add a check for embedded NULL-bytes in the argument. */
static PyObject *
int_from_string(const char *s, int len)
{
	char *end;
	PyObject *x;

	x = PyInt_FromString((char*)s, &end, 10);
	if (x == NULL)
		return NULL;
	if (end != s + len) {
		PyErr_SetString(PyExc_ValueError,
				/*"null byte in argument for int()");*/
				"int() ���� null �줸�@���Ѽ�");
		Py_DECREF(x);
		return NULL;
	}
	return x;
}

PyObject *
PyNumber_Int(PyObject *o)
{
	PyNumberMethods *m;
	const char *buffer;
	int buffer_len;

	if (o == NULL)
		return null_error();
	if (PyInt_Check(o)) {
		Py_INCREF(o);
		return o;
	}
	if (PyString_Check(o))
		return int_from_string(PyString_AS_STRING(o), 
				       PyString_GET_SIZE(o));
	if (PyUnicode_Check(o))
		return PyInt_FromUnicode(PyUnicode_AS_UNICODE(o),
					 PyUnicode_GET_SIZE(o),
					 10);
	m = o->ob_type->tp_as_number;
	if (m && m->nb_int)
		return m->nb_int(o);
	if (!PyObject_AsCharBuffer(o, &buffer, &buffer_len))
		return int_from_string((char*)buffer, buffer_len);

	/*return type_error("object can't be converted to int");*/
	return type_error("���餣�i�ഫ�����");
}

/* Add a check for embedded NULL-bytes in the argument. */
static PyObject *
long_from_string(const char *s, int len)
{
	char *end;
	PyObject *x;

	x = PyLong_FromString((char*)s, &end, 10);
	if (x == NULL)
		return NULL;
	if (end != s + len) {
		PyErr_SetString(PyExc_ValueError,
				/*"null byte in argument for long()");*/
				"long() ���� null �Ѽ�");
		Py_DECREF(x);
		return NULL;
	}
	return x;
}

PyObject *
PyNumber_Long(PyObject *o)
{
	PyNumberMethods *m;
	const char *buffer;
	int buffer_len;

	if (o == NULL)
		return null_error();
	if (PyLong_Check(o)) {
		Py_INCREF(o);
		return o;
	}
	if (PyString_Check(o))
		/* need to do extra error checking that PyLong_FromString() 
		 * doesn't do.  In particular long('9.5') must raise an
		 * exception, not truncate the float.
		 */
		return long_from_string(PyString_AS_STRING(o),
					PyString_GET_SIZE(o));
	if (PyUnicode_Check(o))
		/* The above check is done in PyLong_FromUnicode(). */
		return PyLong_FromUnicode(PyUnicode_AS_UNICODE(o),
					  PyUnicode_GET_SIZE(o),
					  10);
	m = o->ob_type->tp_as_number;
	if (m && m->nb_long)
		return m->nb_long(o);
	if (!PyObject_AsCharBuffer(o, &buffer, &buffer_len))
		return long_from_string(buffer, buffer_len);

	/*return type_error("object can't be converted to long");*/
	return type_error("����L�k�ഫ���j���");
}

PyObject *
PyNumber_Float(PyObject *o)
{
	PyNumberMethods *m;

	if (o == NULL)
		return null_error();
	if (PyFloat_Check(o)) {
		Py_INCREF(o);
		return o;
	}
	if (!PyString_Check(o)) {
		m = o->ob_type->tp_as_number;
		if (m && m->nb_float)
			return m->nb_float(o);
	}
	return PyFloat_FromString(o, NULL);
}

/* Operations on sequences */

int
PySequence_Check(PyObject *s)
{
	return s != NULL && s->ob_type->tp_as_sequence;
}

int
PySequence_Size(PyObject *s)
{
	PySequenceMethods *m;

	if (s == NULL) {
		null_error();
		return -1;
	}

	m = s->ob_type->tp_as_sequence;
	if (m && m->sq_length)
		return m->sq_length(s);

	/*type_error("len() of unsized object");*/
	type_error("����S�����ת��w�q");
	return -1;
}

#undef PySequence_Length
int
PySequence_Length(PyObject *s)
{
	return PySequence_Size(s);
}
#define PySequence_Length PySequence_Size

PyObject *
PySequence_Concat(PyObject *s, PyObject *o)
{
	PySequenceMethods *m;

	if (s == NULL || o == NULL)
		return null_error();

	m = s->ob_type->tp_as_sequence;
	if (m && m->sq_concat)
		return m->sq_concat(s, o);

	/*return type_error("object can't be concatenated");*/
	return type_error("���餣���ߦX");
}

PyObject *
PySequence_Repeat(PyObject *o, int count)
{
	PySequenceMethods *m;

	if (o == NULL)
		return null_error();

	m = o->ob_type->tp_as_sequence;
	if (m && m->sq_repeat)
		return m->sq_repeat(o, count);

	/*return type_error("object can't be repeated");*/
	return type_error("����L�k����");
}

PyObject *
PySequence_InPlaceConcat(PyObject *s, PyObject *o)
{
	PySequenceMethods *m;

	if (s == NULL || o == NULL)
		return null_error();

	m = s->ob_type->tp_as_sequence;
	if (m && HASINPLACE(s) && m->sq_inplace_concat)
		return m->sq_inplace_concat(s, o);
	if (m && m->sq_concat)
		return m->sq_concat(s, o);

	/*return type_error("object can't be concatenated");*/
	return type_error("���餣���ߦX");
}

PyObject *
PySequence_InPlaceRepeat(PyObject *o, int count)
{
	PySequenceMethods *m;

	if (o == NULL)
		return null_error();

	m = o->ob_type->tp_as_sequence;
	if (m && HASINPLACE(o) && m->sq_inplace_repeat)
		return m->sq_inplace_repeat(o, count);
	if (m && m->sq_repeat)
		return m->sq_repeat(o, count);

	/*return type_error("object can't be repeated");*/
	return type_error("����L�k����");
}

PyObject *
PySequence_GetItem(PyObject *s, int i)
{
	PySequenceMethods *m;

	if (s == NULL)
		return null_error();

	m = s->ob_type->tp_as_sequence;
	if (m && m->sq_item) {
		if (i < 0) {
			if (m->sq_length) {
				int l = (*m->sq_length)(s);
				if (l < 0)
					return NULL;
				i += l;
			}
		}
		return m->sq_item(s, i);
	}

	/*return type_error("unindexable object");*/
	return type_error("����S���l������");
}

static PyObject *
sliceobj_from_intint(int i, int j)
{
	PyObject *start, *end, *slice;
	start = PyInt_FromLong((long)i);
	if (!start)
		return NULL;
	end = PyInt_FromLong((long)j);
	if (!end) {
		Py_DECREF(start);
		return NULL;
	}
	slice = PySlice_New(start, end, NULL);
	Py_DECREF(start);
	Py_DECREF(end);
	return slice;
}

PyObject *
PySequence_GetSlice(PyObject *s, int i1, int i2)
{
	PySequenceMethods *m;
	PyMappingMethods *mp;

	if (!s) return null_error();

	m = s->ob_type->tp_as_sequence;
	if (m && m->sq_slice) {
		if (i1 < 0 || i2 < 0) {
			if (m->sq_length) {
				int l = (*m->sq_length)(s);
				if (l < 0)
					return NULL;
				if (i1 < 0)
					i1 += l;
				if (i2 < 0)
					i2 += l;
			}
		}
		return m->sq_slice(s, i1, i2);
	} else if ((mp = s->ob_type->tp_as_mapping) && mp->mp_subscript) {
		PyObject *res;
		PyObject *slice = sliceobj_from_intint(i1, i2);
		if (!slice)
			return NULL;
		res = mp->mp_subscript(s, slice);
		Py_DECREF(slice);
		return res;
	}

	/*return type_error("unsliceable object");*/
	return type_error("����S���w�q����");
}

int
PySequence_SetItem(PyObject *s, int i, PyObject *o)
{
	PySequenceMethods *m;

	if (s == NULL) {
		null_error();
		return -1;
	}

	m = s->ob_type->tp_as_sequence;
	if (m && m->sq_ass_item) {
		if (i < 0) {
			if (m->sq_length) {
				int l = (*m->sq_length)(s);
				if (l < 0)
					return -1;
				i += l;
			}
		}
		return m->sq_ass_item(s, i, o);
	}

	/*type_error("object doesn't support item assignment");*/
	type_error("����l�����i���");
	return -1;
}

int
PySequence_DelItem(PyObject *s, int i)
{
	PySequenceMethods *m;

	if (s == NULL) {
		null_error();
		return -1;
	}

	m = s->ob_type->tp_as_sequence;
	if (m && m->sq_ass_item) {
		if (i < 0) {
			if (m->sq_length) {
				int l = (*m->sq_length)(s);
				if (l < 0)
					return -1;
				i += l;
			}
		}
		return m->sq_ass_item(s, i, (PyObject *)NULL);
	}

	/*type_error("object doesn't support item deletion");*/
	type_error("����l�����i�R��");
	return -1;
}

int
PySequence_SetSlice(PyObject *s, int i1, int i2, PyObject *o)
{
	PySequenceMethods *m;
	PyMappingMethods *mp;

	if (s == NULL) {
		null_error();
		return -1;
	}

	m = s->ob_type->tp_as_sequence;
	if (m && m->sq_ass_slice) {
		if (i1 < 0 || i2 < 0) {
			if (m->sq_length) {
				int l = (*m->sq_length)(s);
				if (l < 0)
					return -1;
				if (i1 < 0)
					i1 += l;
				if (i2 < 0)
					i2 += l;
			}
		}
		return m->sq_ass_slice(s, i1, i2, o);
	} else if ((mp = s->ob_type->tp_as_mapping) && mp->mp_ass_subscript) {
		int res;
		PyObject *slice = sliceobj_from_intint(i1, i2);
		if (!slice)
			return -1;
		res = mp->mp_ass_subscript(s, slice, o);
		Py_DECREF(slice);
		return res;
	}

	/*type_error("object doesn't support slice assignment");*/
	type_error("����������i���");
	return -1;
}

int
PySequence_DelSlice(PyObject *s, int i1, int i2)
{
	PySequenceMethods *m;

	if (s == NULL) {
		null_error();
		return -1;
	}

	m = s->ob_type->tp_as_sequence;
	if (m && m->sq_ass_slice) {
		if (i1 < 0 || i2 < 0) {
			if (m->sq_length) {
				int l = (*m->sq_length)(s);
				if (l < 0)
					return -1;
				if (i1 < 0)
					i1 += l;
				if (i2 < 0)
					i2 += l;
			}
		}
		return m->sq_ass_slice(s, i1, i2, (PyObject *)NULL);
	}
	/*type_error("object doesn't support slice deletion");*/
	type_error("����������i�R��");
	return -1;
}

PyObject *
PySequence_Tuple(PyObject *v)
{
	PySequenceMethods *m;

	if (v == NULL)
		return null_error();

	if (PyTuple_Check(v)) {
		Py_INCREF(v);
		return v;
	}

	if (PyList_Check(v))
		return PyList_AsTuple(v);

	/* There used to be code for strings here, but tuplifying strings is
	   not a common activity, so I nuked it.  Down with code bloat! */

	/* Generic sequence object */
	m = v->ob_type->tp_as_sequence;
	if (m && m->sq_item) {
		int i;
		PyObject *t;
		int n = PySequence_Size(v);
		if (n < 0)
			return NULL;
		t = PyTuple_New(n);
		if (t == NULL)
			return NULL;
		for (i = 0; ; i++) {
			PyObject *item = (*m->sq_item)(v, i);
			if (item == NULL) {
				if (PyErr_ExceptionMatches(PyExc_IndexError))
					PyErr_Clear();
				else {
					Py_DECREF(t);
					t = NULL;
				}
				break;
			}
			if (i >= n) {
				if (n < 500)
					n += 10;
				else
					n += 100;
				if (_PyTuple_Resize(&t, n, 0) != 0)
					break;
			}
			PyTuple_SET_ITEM(t, i, item);
		}
		if (i < n && t != NULL)
			_PyTuple_Resize(&t, i, 0);
		return t;
	}

	/* None of the above */
	/*return type_error("tuple() argument must be a sequence");*/
	return type_error("tuple()/�ܤ�����() ���Ѽƥ����O�ǦC");
}

PyObject *
PySequence_List(PyObject *v)
{
	PySequenceMethods *m;

	if (v == NULL)
		return null_error();

	if (PyList_Check(v))
		return PyList_GetSlice(v, 0, PyList_GET_SIZE(v));

	m = v->ob_type->tp_as_sequence;
	if (m && m->sq_item) {
		int i;
		PyObject *l;
		int n = PySequence_Size(v);
		if (n < 0)
			return NULL;
		l = PyList_New(n);
		if (l == NULL)
			return NULL;
		for (i = 0; ; i++) {
			PyObject *item = (*m->sq_item)(v, i);
			if (item == NULL) {
				if (PyErr_ExceptionMatches(PyExc_IndexError))
					PyErr_Clear();
				else {
					Py_DECREF(l);
					l = NULL;
				}
				break;
			}
			if (i < n)
				PyList_SET_ITEM(l, i, item);
			else if (PyList_Append(l, item) < 0) {
				Py_DECREF(l);
				l = NULL;
				break;
			}
		}
		if (i < n && l != NULL) {
			if (PyList_SetSlice(l, i, n, (PyObject *)NULL) != 0) {
				Py_DECREF(l);
				l = NULL;
			}
		}
		return l;
	}
	/*return type_error("list() argument must be a sequence");*/
	return type_error("list()/�ܧǦC() ���Ѽƥ����O�ǦC");
}

PyObject *
PySequence_Fast(PyObject *v, const char *m)
{
	if (v == NULL)
		return null_error();

	if (PyList_Check(v) || PyTuple_Check(v)) {
		Py_INCREF(v);
		return v;
	}

	v = PySequence_Tuple(v);
	if (v == NULL && PyErr_ExceptionMatches(PyExc_TypeError))
		return type_error(m);

	return v;
}

int
PySequence_Count(PyObject *s, PyObject *o)
{
	int l, i, n, cmp, err;
	PyObject *item;

	if (s == NULL || o == NULL) {
		null_error();
		return -1;
	}
	
	l = PySequence_Size(s);
	if (l < 0)
		return -1;

	n = 0;
	for (i = 0; i < l; i++) {
		item = PySequence_GetItem(s, i);
		if (item == NULL)
			return -1;
		err = PyObject_Cmp(item, o, &cmp);
		Py_DECREF(item);
		if (err < 0)
			return err;
		if (cmp == 0)
			n++;
	}
	return n;
}

int
PySequence_Contains(PyObject *w, PyObject *v) /* v in w */
{
	int i, cmp;
	PyObject *x;
	PySequenceMethods *sq;

	if(PyType_HasFeature(w->ob_type, Py_TPFLAGS_HAVE_SEQUENCE_IN)) {
		sq = w->ob_type->tp_as_sequence;
	        if(sq != NULL && sq->sq_contains != NULL)
			return (*sq->sq_contains)(w, v);
	}
	
	/* If there is no better way to check whether an item is is contained,
	   do it the hard way */
	sq = w->ob_type->tp_as_sequence;
	if (sq == NULL || sq->sq_item == NULL) {
		PyErr_SetString(PyExc_TypeError,
			/*"'in' or 'not in' needs sequence right argument");*/
			"'in' or 'not in'/'�Ӻ�' �� '���Ӻ�' �k���ݭn�@�ӧǦC");
		return -1;
	}

	for (i = 0; ; i++) {
		x = (*sq->sq_item)(w, i);
		if (x == NULL) {
			if (PyErr_ExceptionMatches(PyExc_IndexError)) {
				PyErr_Clear();
				break;
			}
			return -1;
		}
		cmp = PyObject_RichCompareBool(v, x, Py_EQ);
		Py_XDECREF(x);
		if (cmp > 0)
			return 1;
		if (cmp < 0)
			return -1;
	}

	return 0;
}

/* Backwards compatibility */
#undef PySequence_In
int
PySequence_In(PyObject *w, PyObject *v)
{
	return PySequence_Contains(w, v);
}

int
PySequence_Index(PyObject *s, PyObject *o)
{
	int l, i, cmp, err;
	PyObject *item;

	if (s == NULL || o == NULL) {
		null_error();
		return -1;
	}
	
	l = PySequence_Size(s);
	if (l < 0)
		return -1;

	for (i = 0; i < l; i++) {
		item = PySequence_GetItem(s, i);
		if (item == NULL)
			return -1;
		err = PyObject_Cmp(item, o, &cmp);
		Py_DECREF(item);
		if (err < 0)
			return err;
		if (cmp == 0)
			return i;
	}

	/*PyErr_SetString(PyExc_ValueError, "sequence.index(x): x not in list");*/
	PyErr_SetString(PyExc_ValueError, "sequence.index(x): x ���s�b��ǦC��");
	return -1;
}

/* Operations on mappings */

int
PyMapping_Check(PyObject *o)
{
	return o && o->ob_type->tp_as_mapping;
}

int
PyMapping_Size(PyObject *o)
{
	PyMappingMethods *m;

	if (o == NULL) {
		null_error();
		return -1;
	}

	m = o->ob_type->tp_as_mapping;
	if (m && m->mp_length)
		return m->mp_length(o);

	/*type_error("len() of unsized object");*/
	type_error("����S�����ת��w�q");
	return -1;
}

#undef PyMapping_Length
int
PyMapping_Length(PyObject *o)
{
	return PyMapping_Size(o);
}
#define PyMapping_Length PyMapping_Size

PyObject *
PyMapping_GetItemString(PyObject *o, char *key)
{
	PyObject *okey, *r;

	if (key == NULL)
		return null_error();

	okey = PyString_FromStringAndEncode(key, Source_Encoding);
	if (okey == NULL)
		return NULL;
	r = PyObject_GetItem(o, okey);
	Py_DECREF(okey);
	return r;
}

int
PyMapping_SetItemString(PyObject *o, char *key, PyObject *value)
{
	PyObject *okey;
	int r;

	if (key == NULL) {
		null_error();
		return -1;
	}

	okey = PyString_FromStringAndEncode(key, Source_Encoding);
	if (okey == NULL)
		return -1;
	r = PyObject_SetItem(o, okey, value);
	Py_DECREF(okey);
	return r;
}

int
PyMapping_HasKeyString(PyObject *o, char *key)
{
	PyObject *v;

	v = PyMapping_GetItemString(o, key);
	if (v) {
		Py_DECREF(v);
		return 1;
	}
	PyErr_Clear();
	return 0;
}

int
PyMapping_HasKey(PyObject *o, PyObject *key)
{
	PyObject *v;

	v = PyObject_GetItem(o, key);
	if (v) {
		Py_DECREF(v);
		return 1;
	}
	PyErr_Clear();
	return 0;
}

/* Operations on callable objects */

/* XXX PyCallable_Check() is in object.c */

PyObject *
PyObject_CallObject(PyObject *o, PyObject *a)
{
	PyObject *r;
	PyObject *args = a;

	if (args == NULL) {
		args = PyTuple_New(0);
		if (args == NULL)
			return NULL;
	}

	r = PyEval_CallObject(o, args);

	if (args != a) {
		Py_DECREF(args);
	}

	return r;
}

PyObject *
PyObject_CallFunction(PyObject *callable, char *format, ...)
{
	va_list va;
	PyObject *args, *retval;
	va_start(va, format);

	if (callable == NULL) {
		va_end(va);
		return null_error();
	}

	if (format)
		args = Py_VaBuildValue(format, va);
	else
		args = PyTuple_New(0);

	va_end(va);
	
	if (args == NULL)
		return NULL;

	if (!PyTuple_Check(args)) {
		PyObject *a;

		a = PyTuple_New(1);
		if (a == NULL)
			return NULL;
		if (PyTuple_SetItem(a, 0, args) < 0)
			return NULL;
		args = a;
	}
	retval = PyObject_CallObject(callable, args);

	Py_DECREF(args);

	return retval;
}

PyObject *
PyObject_CallMethod(PyObject *o, char *name, char *format, ...)
{
	va_list va;
	PyObject *args, *func = 0, *retval;
	va_start(va, format);

	if (o == NULL || name == NULL) {
		va_end(va);
		return null_error();
	}

	func = PyObject_GetAttrString(o, name);
	if (func == NULL) {
		va_end(va);
		PyErr_SetString(PyExc_AttributeError, name);
		return 0;
	}

	if (!PyCallable_Check(func)) {
		va_end(va);
		/*return type_error("call of non-callable attribute");*/
		return type_error("�ݩʤ��O�i�եΪ�");
	}

	if (format && *format)
		args = Py_VaBuildValue(format, va);
	else
		args = PyTuple_New(0);

	va_end(va);

	if (!args)
		return NULL;

	if (!PyTuple_Check(args)) {
		PyObject *a;

		a = PyTuple_New(1);
		if (a == NULL)
			return NULL;
		if (PyTuple_SetItem(a, 0, args) < 0)
			return NULL;
		args = a;
	}

	retval = PyObject_CallObject(func, args);

	Py_DECREF(args);
	Py_DECREF(func);

	return retval;
}


/* isinstance(), issubclass() */

static int
abstract_issubclass(PyObject *derived, PyObject *cls, int first)
{
	static PyObject *__bases__ = NULL;
	PyObject *bases;
	int i, n;
	int r = 0;

	if (__bases__ == NULL) {
		__bases__ = PyString_FromStringAndEncode("__bases__", 
				Source_Encoding);
		if (__bases__ == NULL)
			return -1;
	}

	if (first) {
		bases = PyObject_GetAttr(cls, __bases__);
		if (bases == NULL || !PyTuple_Check(bases)) {
			Py_XDECREF(bases);
			PyErr_SetString(PyExc_TypeError,
					/*"issubclass() arg 2 must be a class");*/
					"issubclass()/�O�l����() ���ĤG�ӰѼƶ����@�ӷ���");
			return -1;
		}
		Py_DECREF(bases);
	}

	if (derived == cls)
		return 1;

	bases = PyObject_GetAttr(derived, __bases__);
	if (bases == NULL || !PyTuple_Check(bases)) {
	        Py_XDECREF(bases);
		PyErr_SetString(PyExc_TypeError,
				/*"issubclass() arg 1 must be a class");*/
				"issubclass()/�O�l����() ���Ĥ@�ӰѼƶ����@�ӷ���");
		return -1;
	}

	n = PyTuple_GET_SIZE(bases);
	for (i = 0; i < n; i++) {
		r = abstract_issubclass(PyTuple_GET_ITEM(bases, i), cls, 0);
		if (r != 0)
			break;
	}

	Py_DECREF(bases);

	return r;
}

int
PyObject_IsInstance(PyObject *inst, PyObject *cls)
{
	PyObject *icls;
	static PyObject *__class__ = NULL;
	int retval = 0;

        if (PyClass_Check(cls)) {
		if (PyInstance_Check(inst)) {
			PyObject *inclass =
				(PyObject*)((PyInstanceObject*)inst)->in_class;
			retval = PyClass_IsSubclass(inclass, cls);
		}
	}
	else if (PyType_Check(cls)) {
		retval = ((PyObject *)(inst->ob_type) == cls);
	}
	else if (!PyInstance_Check(inst)) {
		if (__class__ == NULL) {
			__class__ = PyString_FromStringAndEncode("__class__", 
					Source_Encoding);
			if (__class__ == NULL)
				return -1;
		}
		icls = PyObject_GetAttr(inst, __class__);
		if (icls != NULL) {
			retval = abstract_issubclass(icls, cls, 1);
			Py_DECREF(icls);
			if (retval < 0 &&
			    !PyErr_ExceptionMatches(PyExc_TypeError))
				return -1;
		}
		else
			retval = -1;
	}
	else
		retval = -1;

	if (retval < 0) {
		PyErr_SetString(PyExc_TypeError,
				/*"isinstance() arg 2 must be a class or type");*/
				"isinstance()/�O����() ���ĤG�ӰѼƶ�������������");
	}
	return retval;
}

int
PyObject_IsSubclass(PyObject *derived, PyObject *cls)
{
	int retval;

	if (!PyClass_Check(derived) || !PyClass_Check(cls)) {
		retval = abstract_issubclass(derived, cls, 1);
	}
	else {
		/* shortcut */
	  	if (!(retval = (derived == cls)))
			retval = PyClass_IsSubclass(derived, cls);
	}

	return retval;
}
