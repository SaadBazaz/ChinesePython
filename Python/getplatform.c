
#include "Python.h"

#ifndef PLATFORM
#define PLATFORM "unknown"
/*#define PLATFORM "����"*/
#endif

const char *
Py_GetPlatform(void)
{
	return PLATFORM;
}

