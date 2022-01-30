/***********************************************************
Copyright 1991-1997 by Stichting Mathematisch Centrum, Amsterdam,
The Netherlands.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Stichting Mathematisch
Centrum or CWI or Corporation for National Research Initiatives or
CNRI not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

While CWI is the initial source for this software, a modified version
is made available by the Corporation for National Research Initiatives
(CNRI) at the Internet address ftp://ftp.python.org.

STICHTING MATHEMATISCH CENTRUM AND CNRI DISCLAIM ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH
CENTRUM OR CNRI BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

/*
 *  macsetfiletype - Set the mac's idea of file type
 *
 */
 
#include "Python.h"
#include "macglue.h"
#include "macdefs.h"

int
PyMac_setfiletype(name, creator, type)
char *name;
long creator, type;
{
	FInfo info;
	FSSpec fss;
	unsigned char *pname;
	
	pname = (StringPtr) Pstring(name);
	if (FSMakeFSSpec(0, 0, pname, &fss) < 0 )
		return -1;
	if ( FSpGetFInfo(&fss, &info) < 0 )
		return -1;
	info.fdType = type;
	info.fdCreator = creator;
	return FSpSetFInfo(&fss, &info);
}

long
PyMac_getfiletype(name)
char *name;
{
	FInfo info;
	unsigned char *pname;
	FSSpec fss;
	
	pname = (StringPtr) Pstring(name);
	if (FSMakeFSSpec(0, 0, pname, &fss) < 0 )
		return -1;
	if ( FSpGetFInfo(&fss, &info) < 0 )
		return -1;
	return info.fdType;
}

