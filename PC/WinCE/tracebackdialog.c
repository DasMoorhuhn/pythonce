#include "Python.h"
#include "windows.h"
#include "tchar.h"


#if (_WIN32_WCE >= 300)
#include "winbase.h"
#include <aygshell.h>
#endif

// extern HMODULE PyWin_DLLhModule;
extern HINSTANCE hInstExe;

/* Obtains a string from a Python traceback.
   This is the exact same string as "traceback.print_exc" would return.

   Pass in a Python traceback object (probably obtained from PyErr_Fetch())
   Result is a string which must be free'd using PyMem_Free()
*/
char *PyTraceback_AsString(PyObject *exc_tb)
{
	char *result = NULL;
	char *errMsg = NULL;
	PyObject *modStringIO = NULL;
	PyObject *modTB = NULL;
	PyObject *obFuncStringIO = NULL;
	PyObject *obStringIO = NULL;
	PyObject *obFuncTB = NULL;
	PyObject *argsTB = NULL;
	PyObject *obResult = NULL;

	/* Import the modules we need - cStringIO and traceback */
	modStringIO = PyImport_ImportModule("cStringIO");
	if (modStringIO==NULL) 
		result = "cant import cStringIO\n";

	if (errMsg==NULL) {
		modTB = PyImport_ImportModule("traceback");
		if (modTB==NULL)
			errMsg = "cant import traceback\n";
	}
	/* Construct a cStringIO object */
	if (errMsg == NULL) {
		obFuncStringIO = PyObject_GetAttrString(modStringIO, "StringIO");
		if (obFuncStringIO==NULL)
			errMsg = "cant find cStringIO.StringIO\n";
	}
	if (errMsg == NULL) {
		obStringIO = PyObject_CallObject(obFuncStringIO, NULL);
		if (obStringIO==NULL) 
			errMsg = "cStringIO.StringIO() failed\n";
	}
	/* Get the traceback.print_exception function, and call it. */
	if (errMsg == NULL) {
		obFuncTB = PyObject_GetAttrString(modTB, "print_tb");
		if (obFuncTB==NULL) 
			errMsg = "cant find traceback.print_tb\n";
	}
	if (errMsg == NULL) {
		argsTB = Py_BuildValue("OOO", 
				exc_tb  ? exc_tb  : Py_None,
				Py_None, 
				obStringIO);
		if (argsTB==NULL) 
			errMsg = "cant make print_tb arguments\n";
	}
	if (errMsg == NULL) {
		obResult = PyObject_CallObject(obFuncTB, argsTB);
		if (obResult==NULL) 
			errMsg = "traceback.print_tb() failed\n";
	}
	/* Now call the getvalue() method in the StringIO instance */
	Py_XDECREF(obFuncStringIO);
	obFuncStringIO = NULL;
	if (errMsg == NULL) {
		obFuncStringIO = PyObject_GetAttrString(obStringIO, "getvalue");
		if (obFuncStringIO==NULL) 
			errMsg = "cant find getvalue function\n";
	}
	Py_XDECREF(obResult);
	obResult = NULL;
	if (errMsg == NULL) {
		obResult = PyObject_CallObject(obFuncStringIO, NULL);
		if (obResult==NULL) 
			errMsg = "getvalue() failed.\n";
	}
	/* And it should be a string all ready to go - duplicate it. */
	if (errMsg == NULL) {
		if (!PyString_Check(obResult))
			errMsg = "getvalue() did not return a string\n";
		else {
			char *tempResult = PyString_AsString(obResult);
			result = PyMem_Malloc(strlen(tempResult)+1);
			strcpy(result, tempResult);
		}
	}
	/* All finished - first see if we encountered an error */
	if (result==NULL && errMsg != NULL) {
		result = PyMem_Malloc(strlen(errMsg)+1);
		strcpy(result, errMsg);
	}

	Py_XDECREF(modStringIO);
	Py_XDECREF(modTB);
	Py_XDECREF(obFuncStringIO);
	Py_XDECREF(obStringIO);
	Py_XDECREF(obFuncTB);
	Py_XDECREF(argsTB);
	Py_XDECREF(obResult);
	return result;
}

/* -----------------------------------------------------------------------------
   Support for a modal window which displays a Python traceback
----------------------------------------------------------------------------- */

/* A private structure used to pass to the dialog box */
struct PyExcInfo {
	PyObject *exc_type;
	PyObject *exc_value;
	PyObject *exc_traceback;
};


/* A private function to translate "\n" to "\r\n" for use in an edit control
 * You must use "PyMem_Free()" on the returned string.
 */
static char *TranslateNewlines(char *szInput)
{
	/* Do it very simply - first count the "\n" characters, allocate a new string
	   and copy it in! 
	*/
	int cChars=0;
	char *szResult;
	char *szReturn;
	char *szLook = szInput;
	for (szLook=szInput;*szLook;szLook++) {
		cChars++;
		if (*szLook=='\n')
			cChars++;
	}
	szResult = szReturn = PyMem_Malloc(cChars + 1);
	for (szLook=szInput;*szLook;szLook++) {
		if (*szLook=='\n')
			*szResult++ = '\r';
		*szResult++ = *szLook;
	}
	*szResult = '\0';
	return szReturn;
}

#ifdef MS_WINCE
/* A private function that resizes the dialog if it is too small.
   Coded for CE, as it assumes the "Close" button has already been
   hidden by CE (otherwise we'd need to take the size of the button
   into account.
*/
static void FixupWindowSizeForWinCE(HWND hWnd, HWND hWndEdit)
{
	RECT rectWnd;
	RECT rectWork;
	BOOL bMove = FALSE;

#if (_WIN32_WCE >= 300)
	SHINITDLGINFO shidi;
	RECT rectEdit;

	shidi.dwMask = SHIDIM_FLAGS;
	shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
	shidi.hDlg = hWnd;
	//initialzes the dialog based on the dwFlags parameter
	SHInitDialog(&shidi);

	GetClientRect(hWnd,&rectWnd);
	GetWindowRect(hWnd,&rectWork);
	GetWindowRect(hWndEdit, &rectEdit);
	MoveWindow(hWndEdit, 5, 5, rectWnd.right-5, rectWnd.bottom-10, 1);

#else
	int cxWnd, cyWnd;
	int cxWork, cyWork;

	SystemParametersInfo(SPI_GETWORKAREA, 0, &rectWork, 0); 
	GetWindowRect(hWnd, &rectWnd);
	cxWnd = rectWnd.right - rectWnd.left;
	cyWnd = rectWnd.bottom - rectWnd.top;
	cxWork = rectWork.right - rectWork.left;
	cyWork = rectWork.bottom - rectWork.top;

	if (cxWnd > cxWork) {
		rectWnd.left = 5;
		cxWnd = cxWork - 10;
		bMove = TRUE;
	}
	if (cyWnd > cyWork) {
		rectWnd.top = 5;
		cyWnd = cyWork - 10;
		bMove = TRUE;
	}
	if (bMove) {
		/* Resize the edit control */
		RECT rectEdit;
		GetWindowRect(hWndEdit, &rectEdit);
		MapWindowPoints(NULL, hWnd, (POINT *)&rectEdit, 2);
		MoveWindow(hWndEdit, rectEdit.left, rectEdit.top, cxWnd-15, cyWnd-15, 0);
		/* And resize the dialog itself */
		MoveWindow(hWnd, rectWnd.left, rectWnd.top, cxWnd, cyWnd, 0);
	}
#endif
}
#endif /* MS_WINCE */

/* WndProc - The Window Procedure for the error dialog */
static LRESULT CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			/* Dialog initialising */
			{
			TCHAR *fullBuf; // May be Unicode.
			struct PyExcInfo *pExcInfo = (struct PyExcInfo *)lParam;

 			char *szTracebackOrig = PyTraceback_AsString(pExcInfo->exc_traceback);
			/* Translate '\n' to '\r\n' */
			char *szTraceback = TranslateNewlines(szTracebackOrig);
			char *szExcType = PyString_AsString(PyObject_Str(pExcInfo->exc_type));
			char *szExcValue = PyString_AsString(PyObject_Str(pExcInfo->exc_value));
			char *szPrefix = "Traceback (innermost last):\r\n";
			HWND hWndEdit = GetDlgItem(hWnd, 1001);

			PyMem_Free(szTracebackOrig); /* Finished with this string */

#ifdef MS_WINCE
			/* Ensure the window is visible on Windows CE */
			FixupWindowSizeForWinCE(hWnd, hWndEdit);
#endif

			/* Create a new string with the fully formatted value */
			/* 3 extra chars - ": " and terminating '\0' */
			fullBuf = PyMem_Malloc( sizeof(TCHAR) * (3 + strlen(szPrefix) + strlen(szTraceback) + strlen(szExcType) + strlen(szExcValue) ) );
			/* Use wsprintf, so we can use %hs, which always uses ascii even under Unicode */
			wsprintf(fullBuf, _T("%hs%hs%hs: %hs"), szPrefix, szTraceback, szExcType, szExcValue);

			/* Write it out */
			SendMessage( hWndEdit, EM_REPLACESEL, (WPARAM)(BOOL)0, (LPARAM)fullBuf );

			/* Set the selection to be the end 
			   For some reason, SendMessage doesnt work yet - so we post it!*/
			PostMessage( hWndEdit, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);

			/* Free our buffers */
			PyMem_Free(fullBuf);
			PyMem_Free(szTraceback);
			return TRUE;
			break;
			}
		case WM_COMMAND:
			{
			WORD wID = LOWORD(wParam);
			if (wID==IDCANCEL || wID==IDOK)
				EndDialog(hWnd, IDCANCEL);
			break;
			}
		default:
			return FALSE;
	}
	return FALSE;
}

int PyErr_DisplayInDialog( PyObject *exc_type, PyObject *exc_value, PyObject *exc_traceback )
{
	int rc;
	struct PyExcInfo exc_info;
	exc_info.exc_type = exc_type;
	exc_info.exc_value = exc_value;
	exc_info.exc_traceback = exc_traceback;

	rc = DialogBoxParam(hInstExe, MAKEINTRESOURCE(1001), 
		NULL, DialogProc, (LPARAM)&exc_info); 
	/* We worked if the result is the "Close" button ID. */
	return rc==IDCANCEL ? 0 : -1;
}
