
/* Support for dynamic loading of extension modules */

#include "Python.h"
#include "osdefs.h"

/* ./configure sets HAVE_DYNAMIC_LOADING if dynamic loading of modules is
   supported on this platform. configure will then compile and link in one
   of the dynload_*.c files, as appropriate. We will call a function in
   those modules to get a function pointer to the module's init function.
*/
#ifdef HAVE_DYNAMIC_LOADING

#include "importdl.h"

extern dl_funcptr _PyImport_GetDynLoadFunc(const char *name,
					   const char *shortname,
					   const char *pathname, FILE *fp);



PyObject *
_PyImport_LoadDynamicModule(char *name, char *pathname, FILE *fp)
{
	PyObject *m, *d, *s;
	char *lastdot, *shortname, *packagecontext, *oldcontext;
	/* ice add:for chinese filename. not used anymore
	char tmp_pathname[MAXPATHLEN+1], *cp_shortname, *lastslash;
	 - ice end */
	dl_funcptr p;

	/* ice add begin for check
	printf("dynaloading name:%s pathname:%s\n",name,pathname);
	ice add end */

	if ((m = _PyImport_FindExtension(name, pathname)) != NULL) {
		Py_INCREF(m);
		return m;
	}

	lastdot = strrchr(name, '.');
	if (lastdot == NULL) {
		packagecontext = NULL;
		shortname = name;
	}
	else {
		packagecontext = name;
		shortname = lastdot+1;
	}

	/* ice begin try assign a new cp_shortname, not used anymore
	strcpy(tmp_pathname,pathname);
	lastslash = strrchr(tmp_pathname,'/');
	lastdot = strrchr(tmp_pathname,'.');
	if (lastslash == NULL) 
		cp_shortname = tmp_pathname;
	else 
		cp_shortname = lastslash + 1;
	if (lastdot != NULL)
		*(lastdot) = '\0';
	printf("my new shortname is %s\n",cp_shortname);
	shortname = cp_shortname;
	*/

	p = _PyImport_GetDynLoadFunc(name, shortname, pathname, fp);
	if (PyErr_Occurred())
		return NULL;
	if (p == NULL) {
		PyErr_Format(PyExc_ImportError,
		   /*"dynamic module does not define init function (init%.200s)",*/
		   "動態模組沒有定義初始函數 init (init%.200s)",
			     shortname);
		return NULL;
	}
        oldcontext = _Py_PackageContext;
	_Py_PackageContext = packagecontext;
	(*p)();
	_Py_PackageContext = oldcontext;
	if (PyErr_Occurred())
		return NULL;
	if (_PyImport_FixupExtension(name, pathname) == NULL)
		return NULL;

	m = PyDict_GetItemString(PyImport_GetModuleDict(), name);
	if (m == NULL) {
		PyErr_SetString(PyExc_SystemError,
				/*"dynamic module not initialized properly");*/
				"動態模組的初始化不完全");
		return NULL;
	}
	/* Remember the filename as the __file__ attribute */
	d = PyModule_GetDict(m);
	s = PyString_FromStringAndEncode(pathname, Source_Encoding);
	if (s == NULL || PyDict_SetItemString(d, "__file__", s) != 0 ||
		PyDict_SetItemString(d, "__檔案__", s) != 0 )
		PyErr_Clear(); /* Not important enough to report */
	Py_XDECREF(s);
	if (Py_VerboseFlag)
		PySys_WriteStderr(
			/*"import %s # dynamically loaded from %s\n",*/
			"載入 %s # 從 %s 中動態加載\n",
			name, pathname);
	Py_INCREF(m);
	return m;
}

#endif /* HAVE_DYNAMIC_LOADING */
