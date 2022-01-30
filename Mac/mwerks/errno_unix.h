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
** There are various sources of unix-like error numbers: GUSI headers,
** MSL headers and Carbon-specific MSL headers. The later are triggered,
** apparently, by the _POSIX define.
*/
#ifndef USE_GUSI2
#define ENOTDIR		(-120)
#ifndef __MSL__
#define EACCES		(-54)
#endif
#ifndef _POSIX
#define EEXIST		(-48)
#define ENOENT		(-43)
#define ENFILE		(-42)
#define ENOSPC		(-34)
#define	EIO			(-36)
#endif
#define EBUSY		(-47)
#define EROFS		(-44)
#endif

#ifndef USE_GUSI2
#define ESRCH		3
#define EINTR		4
#define ENODEV		19
#endif
#ifndef _POSIX
#define EBADF		9
#define EINVAL		22
#define EMFILE		24
#endif

