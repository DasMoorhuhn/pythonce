/********************************************************************

 import_nt.c 

  Win32 specific import code.

*/

#include "Python.h"
#include "osdefs.h"
#include <windows.h>
#include "importdl.h"
#include "malloc.h" /* for alloca */

#include  <tchar.h>

/* a string loaded from the DLL at startup */
extern const TCHAR *PyWin_DLLVersionString;

FILE *PyWin_FindRegisteredModule(const char *moduleName,
				 struct filedescr **ppFileDesc,
				 char *pathBuf,
				 int pathLen)
{
	TCHAR *moduleKey;
	const TCHAR keyPrefix[] = _T("Software\\Python\\PythonCore\\");
	const TCHAR keySuffix[] = _T("\\Modules\\");
#ifdef _DEBUG
	/* In debugging builds, we _must_ have the debug version
	 * registered.
	 */
	const TCHAR debugString[] = _T("\\Debug");
#else
	const TCHAR debugString[] = _T("");
#endif
	struct filedescr *fdp = NULL;
	FILE *fp = NULL;
	HKEY keyBase = HKEY_CURRENT_USER;
	DWORD lptype;
	int modNameSize;
	long regStat;

	/* Calculate the size for the sprintf buffer.
	 * Get the size of the chars only, plus 1 NULL.
	 */
	size_t bufSize = (sizeof(keyPrefix)/sizeof(keyPrefix[0]))-1 +
	                 _tcslen(PyWin_DLLVersionString) +
	                 (sizeof(keySuffix)/sizeof(keySuffix[0])) +
	                 strlen(moduleName) +
	                 (sizeof(debugString)/sizeof(debugString[0])) - 1;
	/* alloca == no free required, but memory only local to fn,
	 * also no heap fragmentation!
	 */
#if defined(MS_WINCE) && defined(UNICODE)
	TCHAR *tPathBuf = (TCHAR *)LocalAlloc(0, pathLen * sizeof(TCHAR));
#else	/* not MS_WINCE || not UNICODE */
	TCHAR *tPathBuf = pathBuf;
#endif	/* not MS_WINCE || not UNICODE */

	moduleKey = alloca(bufSize * sizeof(TCHAR)); 
#ifndef	MS_WINCE
	PyOS_snprintf(moduleKey, bufSize,
		      "Software\\Python\\PythonCore\\%s\\Modules\\%s%s",
		      PyWin_DLLVersionString, moduleName, debugString);

#else	/* not MS_WINCE */
	wsprintf(moduleKey,
		_T("Software\\Python\\PythonCore\\%s\\Modules\\%hs%s"),
	        PyWin_DLLVersionString, moduleName, debugString);
#endif	/* not MS_WINCE */
	modNameSize = pathLen;
	regStat = RegQueryValueEx(keyBase, moduleKey, NULL, &lptype, (char *)tPathBuf, &modNameSize);
	if (regStat != ERROR_SUCCESS) {
		/* No user setting - lookup in machine settings */
		keyBase = HKEY_LOCAL_MACHINE;
		/* be anal - failure may have reset size param */
		modNameSize = pathLen;
		regStat = RegQueryValueEx(keyBase, moduleKey, NULL, &lptype,
					  (char *) tPathBuf, &modNameSize);

		if (regStat != ERROR_SUCCESS)
			goto done;
	}
#ifdef	UNICODE
	WideCharToMultiByte(CP_ACP,0,tPathBuf,modNameSize,pathBuf,modNameSize,NULL,NULL);
#endif
	/* use the file extension to locate the type entry. */
	for (fdp = _PyImport_Filetab; fdp->suffix != NULL; fdp++) {
		size_t extLen = strlen(fdp->suffix);
		assert(modNameSize >= 0); /* else cast to size_t is wrong */
#ifdef	MS_WINCE	// could be case sensitive on windows too
		if ((size_t)modNameSize > extLen &&
		    strncmp(pathBuf + ((size_t)modNameSize-extLen-1),
		             fdp->suffix,
		             extLen) == 0)

#else	/* not MS_WINCE */
		if ((size_t)modNameSize > extLen &&
		    strnicmp(pathBuf + ((size_t)modNameSize-extLen-1),
		             fdp->suffix,
		             extLen) == 0)
#endif	/* not MS_WINCE */
			break;
	}
	if (fdp->suffix == NULL)
		goto done;
	fp = fopen(pathBuf, fdp->mode);
	if (fp != NULL)
		*ppFileDesc = fdp;

 done:
#ifdef	UNICODE
	if (tPathBuf) LocalFree(tPathBuf);
#endif
	return fp;
}
