# This script generates a Python interface for an Apple Macintosh Manager.
# It uses the "bgen" package to generate C code.
# The function specifications are generated by scanning the mamager's header file,
# using the "scantools" package (customized for this particular manager).

# NOTE: the scrap include file is so bad that the bgen output has to be
# massaged by hand.

import string

# Declarations that change for each manager
MACHEADERFILE = 'Scrap.h'		# The Apple header file
MODNAME = 'Scrap'				# The name of the module

# The following is *usually* unchanged but may still require tuning
MODPREFIX = MODNAME			# The prefix for module-wide routines
INPUTFILE = string.lower(MODPREFIX) + 'gen.py' # The file generated by the scanner
OUTPUTFILE = '@' + MODNAME + "module.c"	# The file generated by this program

from macsupport import *

# Create the type objects

includestuff = includestuff + """
#ifdef WITHOUT_FRAMEWORKS
#include <Scrap.h>
#else
#include <Carbon/Carbon.h>
#endif

/*
** Generate ScrapInfo records
*/
static PyObject *
SCRRec_New(itself)
	ScrapStuff *itself;
{

	return Py_BuildValue("lO&hhO&", itself->scrapSize,
		ResObj_New, itself->scrapHandle, itself->scrapCount, itself->scrapState,
		PyMac_BuildStr255, itself->scrapName);
}
"""

ScrapStuffPtr = OpaqueByValueType('ScrapStuffPtr', 'SCRRec')
ScrapFlavorType = OSTypeType('ScrapFlavorType')
putscrapbuffer = FixedInputBufferType('void *')

# Create the generator groups and link them
module = MacModule(MODNAME, MODPREFIX, includestuff, finalstuff, initstuff)

# Create the generator classes used to populate the lists
Function = OSErrFunctionGenerator

# Create and populate the lists
functions = []
execfile(INPUTFILE)

# add the populated lists to the generator groups
# (in a different wordl the scan program would generate this)
for f in functions: module.add(f)

# generate output (open the output file as late as possible)
SetOutputFileName(OUTPUTFILE)
module.generate()

