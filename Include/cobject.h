
/* C objects to be exported from one extension module to another.
 
   C objects are used for communication between extension modules.
   They provide a way for an extension module to export a C interface
   to other extension modules, so that extension modules can use the
   Python import mechanism to link to one another.

*/

#ifndef Py_COBJECT_H
#define Py_COBJECT_H
#ifdef __cplusplus
extern "C" {
#endif

extern DL_IMPORT(PyTypeObject) PyCObject_Type;

#define PyCObject_Check(op) ((op)->ob_type == &PyCObject_Type)

/* Create a PyCObject from a pointer to a C object and an optional
   destructor function.  If the second argument is non-null, then it
   will be called with the first argument if and when the PyCObject is
   destroyed.

*/
extern DL_IMPORT(PyObject *)
PyCObject_FromVoidPtr(void *cobj, void (*destruct)(void*));


/* Create a PyCObject from a pointer to a C object, a description object,
   and an optional destructor function.  If the third argument is non-null,
   then it will be called with the first and second arguments if and when 
   the PyCObject is destroyed.
*/
extern DL_IMPORT(PyObject *)
PyCObject_FromVoidPtrAndDesc(void *cobj, void *desc,
                             void (*destruct)(void*,void*));

/* Retrieve a pointer to a C object from a PyCObject. */
extern DL_IMPORT(void *)
PyCObject_AsVoidPtr(PyObject *);

/* Retrieve a pointer to a description object from a PyCObject. */
extern DL_IMPORT(void *)
PyCObject_GetDesc(PyObject *);

/* Import a pointer to a C object from a module using a PyCObject. */
extern DL_IMPORT(void *)
PyCObject_Import(char *module_name, char *cobject_name);

#ifdef __cplusplus
}
#endif
#endif /* !Py_COBJECT_H */
