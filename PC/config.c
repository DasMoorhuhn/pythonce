/* Module configuration */

/* This file contains the table of built-in modules.
   See init_builtin() in import.c. */

#include "Python.h"

extern void initarray(void);
#ifndef MS_WIN64
extern void initaudioop(void);
#endif
extern void initbinascii(void);
extern void initcmath(void);
extern void initerrno(void);
extern void initgc(void);
#ifndef MS_WIN64
extern void initimageop(void);
#endif
extern void initmath(void);
extern void initmd5(void);
extern void initnt(void);
extern void initoperator(void);
extern void initregex(void);
#ifndef MS_WIN64
extern void initrgbimg(void);
#endif
extern void initsignal(void);
extern void initsha(void);
extern void initstrop(void);
extern void initstruct(void);
extern void inittime(void);
extern void initthread(void);
extern void initcStringIO(void);
extern void initcPickle(void);
#ifdef WIN32
extern void initmsvcrt(void);
extern void init_locale(void);
#endif
extern void init_codecs(void);
extern void init_weakref(void);
extern void init_hotshot(void);
extern void initxxsubtype(void);
extern void initzipimport(void);
extern void init_random(void);
extern void inititertools(void);
extern void initcollections(void);
extern void init_heapq(void);
extern void init_bisect(void);
extern void init_symtable(void);
extern void initmmap(void);
extern void init_csv(void);
extern void init_sre(void);
extern void initparser(void);
extern void init_winreg(void);
extern void initdatetime(void);

extern void init_multibytecodec(void);
extern void init_codecs_cn(void);
extern void init_codecs_hk(void);
extern void init_codecs_iso2022(void);
extern void init_codecs_jp(void);
extern void init_codecs_kr(void);
extern void init_codecs_tw(void);
extern void init_subprocess(void);
#ifdef	MS_WINCE
extern void initce(void);
extern void initzlib(void);
#endif	/* MS_WINCE */

/* tools/freeze/makeconfig.py marker for additional "extern" */
/* -- ADDMODULE MARKER 1 -- */

extern void PyMarshal_Init(void);
extern void initimp(void);

struct _inittab _PyImport_Inittab[] = {

        {"array", initarray},
#ifdef MS_WINDOWS
#ifndef MS_WIN64
        {"audioop", initaudioop},
#endif
#endif
        {"binascii", initbinascii},
        {"cmath", initcmath},
        {"errno", initerrno},
        {"gc", initgc},
#ifndef MS_WIN64
        {"imageop", initimageop},
#endif
        {"math", initmath},
        {"md5", initmd5},
#ifndef MS_WINCE
        {"nt", initnt}, /* Use the NT os functions, not posix */
#endif /* not MS_WINCE */
        {"operator", initoperator},
        {"regex", initregex},
#ifndef MS_WIN64
        {"rgbimg", initrgbimg},
#endif
#ifndef MS_WINCE
        {"signal", initsignal},
#endif /* not MS_WINCE */
        {"sha", initsha},
        {"strop", initstrop},
        {"struct", initstruct},
        {"time", inittime},
#ifdef WITH_THREAD
        {"thread", initthread},
#endif
        {"cStringIO", initcStringIO},
        {"cPickle", initcPickle},
#ifdef WIN32
#ifndef	MS_WINCE
        {"msvcrt", initmsvcrt},
#endif	/* not MS_WINCE */
        {"_locale", init_locale},
#endif

#ifndef MS_WINCE
	/* XXX Should _subprocess go in a WIN32 block?  not WIN64? */
	{"_subprocess", init_subprocess},
#endif

        {"_codecs", init_codecs},
	{"_weakref", init_weakref},
	{"_hotshot", init_hotshot},
	{"_random", init_random},
        {"_bisect", init_bisect},
        {"_heapq", init_heapq},
	{"itertools", inititertools},
        {"collections", initcollections},
	{"_symtable", init_symtable},
	{"mmap", initmmap},
	{"_csv", init_csv},
	{"_sre", init_sre},
	{"parser", initparser},
	{"_winreg", init_winreg},
	{"datetime", initdatetime},

#ifdef	MS_WINCE
	{"ce", initce},
	/*
	 *	The following module is compiled in to ensure that basic things
	 *	work even when the world is really messed up
	 */
	{"zlib", initzlib},	/* Needed to support loading ZIP files from resources (interactive shell window) */
#endif	/* MS_WINCE */

	{"xxsubtype", initxxsubtype},
	{"zipimport", initzipimport},

	/* CJK codecs */
	{"_multibytecodec", init_multibytecodec},
	{"_codecs_cn", init_codecs_cn},
	{"_codecs_hk", init_codecs_hk},
	{"_codecs_iso2022", init_codecs_iso2022},
	{"_codecs_jp", init_codecs_jp},
	{"_codecs_kr", init_codecs_kr},
	{"_codecs_tw", init_codecs_tw},

/* tools/freeze/makeconfig.py marker for additional "_inittab" entries */
/* -- ADDMODULE MARKER 2 -- */

        /* This module "lives in" with marshal.c */
        {"marshal", PyMarshal_Init},

        /* This lives it with import.c */
        {"imp", initimp},

        /* These entries are here for sys.builtin_module_names */
        {"__main__", NULL},
        {"__builtin__", NULL},
        {"sys", NULL},
	{"exceptions", NULL},

        /* Sentinel */
        {0, 0}
};
