
#include "Python.h"

#ifndef PLATFORM
#define PLATFORM "unknown"
/*#define PLATFORM "คฃธิ"*/
#endif

const char *
Py_GetPlatform(void)
{
	return PLATFORM;
}

