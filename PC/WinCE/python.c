/*
 *	Windows/CE Python application startup
 *
 *	David Kashtan, Validus Medical Systems
 */
#include <windows.h>
#include "Python.h"

#ifndef	PYTHON_DLL_NAME
#error PYTHON_DLL_NAME not defined
#endif	/* not PYTHON_DLL_NAME */

/*
 *	A simple macro to stringify its argument
 */
#define	_NAME1(xxx) #xxx
#define	_NAME(xxx) _NAME1(xxx)

/*
 *	Dynamically loaded Python DLL symbols and macros so we can use them as though
 *	they were not dynamically loaded.
 */
#define	PyErr_Fetch (*PyErr_FetchP)
static void (*PyErr_FetchP)(PyObject **, PyObject **, PyObject **);
#define	Py_fopen (*Py_fopenP)
static FILE *Py_fopen(const char *, const char *);
#define	Py_fclose (*Py_fcloseP)
static void (*Py_fcloseP)(FILE *);
#define PyRun_SimpleFile (*PyRun_SimpleFileP)
static int (*PyRun_SimpleFileP)(FILE *, const char *);
#define	PyErr_SetString (*PyErr_SetStringP)
static void (*PyErr_SetString)(PyObject *, const char *);
#define PyExc_ValueError (*PyExc_ValueErrorP)
static PyObject **PyExc_ValueErrorP;
#define	PyObject_CallObject (*PyObject_CallObjectP)
static PyObject *(*PyObject_CallObjectP)(PyObject *, PyObject *);
#define PyObject_GetAttrString (*PyObject_GetAttrStringP)
static PyObject *(*PyObject_GetAttrStringP)(PyObject *, char *);
#define PyImport_ImportModule (*PyImport_ImportModuleP)
static PyObject *(*PyImport_ImportModuleP)(char *);
#define PySys_SetArgv (*PySys_SetArgvP)
static void (*PySys_SetArgvP)(int, char **);
#define PySys_SetObject (*PySys_SetObjectP)
static int (*PySys_SetObjectP)(char *, PyObject *);
#define PyInt_FromLong (*PyInt_FromLongP)
static PyObject *(*PyInt_FromLongP)(long);
#define Py_Initialize (*Py_InitializeP)
static void (*Py_InitializeP)(void);
#define Py_SetProgramName (*Py_SetProgramNameP)
static void (*Py_SetProgramNameP)(char *);
#define Py_InteractiveFlag (*Py_InteractiveFlagP)
static int (*Py_InteractiveFlagP);
#define PyString_AsString (*PyString_AsStringP)
static char *(*PyString_AsStringP)(PyObject *);
#define PyString_AsStringAndSize (*PyString_AsStringAndSizeP)
static int (*PyString_AsStringAndSizeP)(PyObject *, char **,int *);
#define PyString_FromStringAndSize (*PyString_FromStringAndSizeP)
static PyObject *(*PyString_FromStringAndSizeP)(const char *, int);
#define PyType_IsSubtype (*PyType_IsSubtypeP)
static int (*PyType_IsSubtypeP)(PyTypeObject *, PyTypeObject *);
#define PyString_Type (*PyString_TypeP)
static PyTypeObject *PyString_TypeP;
#define Py_BuildValue (*Py_BuildValueP)
static PyObject *(*Py_BuildValueP)(char *, ...);
#define _Py_NoneStruct (*_Py_NoneStructP)
static PyObject *_Py_NoneStructP;
#define PyObject_Str (*PyObject_StrP)
static PyObject *(*PyObject_StrP)(PyObject *);
#define PyOS_snprintf (*PyOS_snprintfP)
static int (*PyOS_snprintfP)(char *, size_t, const char *, ...);
#define PyRun_String (*PyRun_StringP)
static PyObject *(*PyRun_StringP)(char *, int, PyObject *, PyObject *);
#define PyErr_Occurred (*PyErr_OccurredP)
static int (*PyErr_OccurredP)(void);
#define	PyModule_GetDict (*PyModule_GetDictP)
static PyObject *(*PyModule_GetDictP)(PyObject *);
#define	PyDict_SetItemString (*PyDict_SetItemStringP)
static int (*PyDict_SetItemStringP)(PyObject *, const char *, PyObject *);
#define	PyEval_GetBuiltins (*PyEval_GetBuiltinsP)
static PyObject *(*PyEval_GetBuiltinsP)(void);
#define	Py_InitModule4 (*Py_InitModule4P)
static PyObject *(*Py_InitModule4P)(char *, PyMethodDef *, char *, PyObject *, int);
#define	PyArg_ParseTuple (*PyArg_ParseTupleP)
static int (*PyArg_ParseTupleP)(PyObject *, char *, ...);
#define PyEval_SaveThread (*PyEval_SaveThreadP)
static PyThreadState *(*PyEval_SaveThreadP)(void);
#define	PyEval_RestoreThread (*PyEval_RestoreThreadP)
static void (*PyEval_RestoreThreadP)(PyThreadState *);
#define PySys_GetObject (*PySys_GetObjectP)
static PyObject *(*PySys_GetObjectP)(char *);
#define	_WinCE_Declare_Interrupt	(*_WinCE_Declare_InterruptP)
static void (*_WinCE_Declare_InterruptP)(void);
#define	_SYMBOL(Name) {TEXT(#Name), (void **)&Name##P}
static struct {TCHAR *Name; void **SymbolP;} Python_DLL_Symbols[] = {
	_SYMBOL(PyErr_Fetch), _SYMBOL(PyExc_ValueError), _SYMBOL(PyRun_SimpleFile), _SYMBOL(Py_fopen), _SYMBOL(Py_fclose),
	_SYMBOL(PyObject_CallObject), _SYMBOL(PyObject_GetAttrString), _SYMBOL(PyImport_ImportModule),
	_SYMBOL(PySys_SetArgv), _SYMBOL(PySys_SetObject), _SYMBOL(PyInt_FromLong), _SYMBOL(Py_Initialize),
	_SYMBOL(Py_SetProgramName), _SYMBOL(Py_InteractiveFlag),
	_SYMBOL(PyString_AsString), _SYMBOL(PyString_AsStringAndSize), _SYMBOL(PyString_FromStringAndSize),
	_SYMBOL(PyObject_Str), _SYMBOL(PyType_IsSubtype), _SYMBOL(PyString_Type), _SYMBOL(Py_BuildValue),
	_SYMBOL(_Py_NoneStruct), _SYMBOL(PyOS_snprintf), _SYMBOL(PyRun_String), _SYMBOL(PyErr_Occurred),
	_SYMBOL(PyModule_GetDict), _SYMBOL(PyDict_SetItemString),
	_SYMBOL(PyEval_GetBuiltins), _SYMBOL(Py_InitModule4), _SYMBOL(PyArg_ParseTuple),
	_SYMBOL(PyEval_SaveThread), _SYMBOL(PyEval_RestoreThread), _SYMBOL(PySys_GetObject),
	_SYMBOL(_WinCE_Declare_Interrupt)};
#undef	_SYMBOL

/*
 *	A local strlen(), so we don't need the "C" library
 */
#define	strlen	local_strlen
static int strlen(char *String)
{
	char *cp;

	cp = String;
	while(*cp) cp++;
	return(cp - String);
}

/*
 *	A local strcmp(), so we don't need the "C" library
 */
#define strcmp local_strcmp
static int strcmp(char *cp, char *cp1)
{
	while(*cp)
		if (*cp++ != *cp1++) return(1);
	if (*cp1) return(1);
	return(0);
}

/*
 *	Return a string with the Python traceback in it
 */
static char *PyTraceback_AsString(PyObject *Traceback)
{
	char *Result_String = NULL;
	char *Error_Message = NULL;
	PyObject *StringIO_Module = NULL;
	PyObject *Traceback_Module = NULL;
	PyObject *StringIO_Function = NULL;
	PyObject *StringIO_Object = NULL;
	PyObject *Traceback_Function = NULL;
	PyObject *Traceback_Arguments = NULL;
	PyObject *Result = NULL;

	/*
	 *	Import the modules we need
	 */
	StringIO_Module = PyImport_ImportModule("cStringIO");
	if (!StringIO_Module) Result_String = "Cant import cStringIO\n";
	Traceback_Module = PyImport_ImportModule("traceback");
	if (!Traceback_Module) Error_Message = "Cant import traceback\n";
	/*
	 *	Construct a cStringIO object
	 */
	if (!Error_Message) {
		StringIO_Function = PyObject_GetAttrString(StringIO_Module, "StringIO");
		if (!StringIO_Function) Error_Message = "Cant find cStringIO.StringIO\n";
	}
	if (!Error_Message) {
		StringIO_Object = PyObject_CallObject(StringIO_Function, NULL);
		if (!StringIO_Object) Error_Message = "cStringIO.StringIO() failed\n";
	}
	Py_XDECREF(StringIO_Function);
	StringIO_Function = NULL;
	/*
	 *	Get the traceback.print_exception function
	 */
	if (!Error_Message) {
		Traceback_Function = PyObject_GetAttrString(Traceback_Module, "print_tb");
		if (!Traceback_Function) Error_Message = "Cant find traceback.print_tb\n";
	}
	/*
	 *	Build the argument list
	 */
	if (!Error_Message) {
		Traceback_Arguments = Py_BuildValue("OOO",
						    Traceback  ? Traceback  : Py_None,
						    Py_None, 
						    StringIO_Object);
		if (!Traceback_Arguments) Error_Message = "Cant make print_tb arguments\n";
	}
	/*
	 *	Call it
	 */
	if (!Error_Message) {
		Result = PyObject_CallObject(Traceback_Function, Traceback_Arguments);
		if (!Result) Error_Message = "traceback.print_tb() failed\n";
	}
	Py_XDECREF(Result);
	Result = NULL;
	/*
	 *	 Call the getvalue() method in the StringIO instance
	 */
	if (!Error_Message) {
		StringIO_Function = PyObject_GetAttrString(StringIO_Object, "getvalue");
		if (!StringIO_Function) Error_Message = "Cant find getvalue function\n";
	}
	if (!Error_Message) {
		Result = PyObject_CallObject(StringIO_Function, NULL);
		if (!Result) Error_Message = "getvalue() failed.\n";
	}
	/*
	 *	Get the string
	 */
	if (!Error_Message) {
		if (PyString_Check(Result)) {
			char *Temp;

			Temp = PyString_AsString(Result);
			Result_String = (char *)LocalAlloc(0, strlen(Temp) + 1);
			strcpy(Result_String, Temp);
		} else {
			Error_Message = "getvalue() did not return a string\n";
		}
	}
	/*
	 *	Check for errors
	 */
	if (!Result_String && Error_Message) {
		/*
		 *	Make the error the result string
		 */
		Result_String = (char *)LocalAlloc(0, strlen(Error_Message) + 1);
		strcpy(Result_String, Error_Message);
	}
	/*
	 *	Dereference everything
	 */
	Py_XDECREF(StringIO_Module);
	Py_XDECREF(Traceback_Module);
	Py_XDECREF(StringIO_Function);
	Py_XDECREF(StringIO_Object);
	Py_XDECREF(Traceback_Function);
	Py_XDECREF(Traceback_Arguments);
	Py_XDECREF(Result);
	return(Result_String);
}

/*
 *	Translate "\n" to "\r\n" for use in an edit control
 */
static char *Translate_Newlines(char *String)
{
	char *cp, *cp1;
	int Length;
	char *Result;
	
	/*
	 *	Calculate the new string length (count "\n" as "\r\n")
	 */
	Length = 0;
	cp = String;
	while(*cp) {
		Length++;
		if (*cp++ == '\n') Length++;
	}
	/*
	 *	Allocate the result buffer
	 */
	Result = (char *)LocalAlloc(0, Length + 1);
	cp = String;
	cp1 = Result;
	while((*cp1++ = *cp++) != '\0') {
		if (cp1[-1] == '\n') {
			cp1[-1] = '\r';
			*cp1++ = '\n';
		}
	}
	return(Result);
}

/*
 *	Resize a dialog if it is too small.
 *
 *	Coded for Windows/CE, as it assumes the "Close" button has already been
 *	hidden by CE (otherwise we'd need to take the size of the button
 *	into account.
 */
#include <aygshell.h>
static void Fixup_Window_Size_For_WinCE(HWND hWnd, HWND hWndEdit)
{
	RECT rectWnd;
	RECT rectWork;
	SHINITDLGINFO Info;
	RECT rectEdit;
	HANDLE Handle;
	int (*SHInitDialogP)(SHINITDLGINFO *);

	/*
	 *	Dynamically load the aygshell DLL
	 */
	Handle = LoadLibrary(TEXT("aygshell.dll"));
	if (!Handle) {
		MessageBox(0, TEXT("Couldn't open aygshell.dll"), TEXT("ERROR"), MB_OK);
		return;
	}
	/*
	 *	Find the SHInitDialog function
	 */
	SHInitDialogP = (int (*)(SHINITDLGINFO *))GetProcAddress(Handle, TEXT("SHInitDialog"));
	if (!SHInitDialogP) {
		MessageBox(0, TEXT("Couldn't find \"SHInitDialog\" function in aygshell.dll"), TEXT("ERROR"), MB_OK);
		return;
	}
	/*
	 *	Call it
	 */
	Info.dwMask = SHIDIM_FLAGS;
	Info.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
	Info.hDlg = hWnd;
	(*SHInitDialogP)(&Info);
	FreeLibrary(Handle);
	/*
	 *	Fix the window to fit the screen
	 */
	GetClientRect(hWnd,&rectWnd);
	GetWindowRect(hWnd,&rectWork);
	GetWindowRect(hWndEdit, &rectEdit);
	MoveWindow(hWndEdit, 5, 5, rectWnd.right-5, rectWnd.bottom-10, 1);
}

/*
 *	A private structure used to pass information to the dialog box
 */
struct _Info {
	PyObject *Type;
	PyObject *Value;
	PyObject *Traceback;
	};

/*
 *	An ID to use for the last chance error window
 */
#define	LASTCHANCE_ERROR_WINDOW_EDIT_CONTROL_ID	1000

/*
 *	The Window Procedure for the modal dialog that display's last-chance Python errors
 */
static LRESULT CALLBACK Dialog_Procedure(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	/*
	 *	Dispatch on windows message
	 */
	switch (Message) {
	  case WM_INITDIALOG: {
		HWND hWndEdit;
		TCHAR *Full_Buffer;
		struct _Info *Info = (struct _Info *)lParam;
		char *Traceback_String_Original;
		char *Traceback_String;
		char *Type_String;
		char *Value_String;
		char *Prefix_String;

		/*
		 *	Dialog initializing: Get the traceback string
		 */
		Traceback_String_Original = PyTraceback_AsString(Info->Traceback);
		/*
		 *	Translate '\n' to '\r\n'
		 */
		Traceback_String = Translate_Newlines(Traceback_String_Original);
		/*
		 *	Get the Type, Value and Prefix strings
		 */
		Type_String = PyString_AsString(PyObject_Str(Info->Type));
		Value_String = PyString_AsString(PyObject_Str(Info->Value));
		Prefix_String = "Traceback (innermost last):\r\n";
		/*
		 *	Get the edit window
		 */
		hWndEdit = GetDlgItem(hWnd, LASTCHANCE_ERROR_WINDOW_EDIT_CONTROL_ID);
		/*
		 *	We are finished with the original traceback string
		 */
		LocalFree(Traceback_String_Original);
		/*
		 *	Ensure the window is visible on Windows CE
		 */
		Fixup_Window_Size_For_WinCE(hWnd, hWndEdit);
		/*
		 *	Create a new string with the fully formatted value
		 *	*3 extra chars - ": " and terminating '\0')
		 */
		Full_Buffer = (TCHAR *)LocalAlloc(0, sizeof(TCHAR) * (3 +
								      strlen(Prefix_String) +
								      strlen(Traceback_String) +
								      strlen(Type_String) +
								      strlen(Value_String)));
		/*
		 *	Use wsprintf, so we can use %hs, which always uses ascii even under Unicode
		 */
		wsprintf(Full_Buffer, TEXT("%hs%hs%hs: %hs"), Prefix_String, Traceback_String, Type_String, Value_String);
		LocalFree(Traceback_String);
		/*
		 *	Write it out
		 */
		SendMessage(hWndEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)Full_Buffer);
		/*
		 *	Set the selection to be the end (SendMessage does not work)
		 */
		PostMessage(hWndEdit, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
		/*
		 *	Free our buffers
		 */
		LocalFree(Full_Buffer);
		return TRUE;
	  }
	  /*
	   *	User has clicked something
	   */
	  case WM_COMMAND: {
		WORD ID = LOWORD(wParam);

		/*
		 *	End the dialog on OK or CANCEL
		 */
		if ((ID == IDCANCEL) || (ID == IDOK)) EndDialog(hWnd, IDCANCEL);
		return(FALSE);
	  }
	}
	return(FALSE);
}

/*
 *	Convert a command line to an argv list
 */
static void Parse_Command_Line(TCHAR *lpCmdLine, int *ArgcP, char **Argv)
{
	int Length;
	char *cp;
	char Quote_Character = '\0';
	char Path[(MAX_PATH + 1)* sizeof(TCHAR)];

	/*
	 *	Get the module name so we can set argv[0]
	 */
	if (GetModuleFileName(NULL, (TCHAR *)Path, MAX_PATH + 1)) {
#ifdef	UNICODE
		WideCharToMultiByte(CP_ACP, 0, (TCHAR *)Path, -1, Path, MAX_PATH + 1, NULL, NULL);
#endif	/* UNICODE */
		Argv[0] = (char *)LocalAlloc(0, strlen(Path) + 1);
		if (Argv[0]) strcpy(Argv[0], Path);
	} else {
		Argv[0] = 0;
	}
	if (!Argv[0]) Argv[0] = "";
#ifdef	UNICODE
	/*
	 *	Convert to Ascii
	 */
	Length = WideCharToMultiByte(CP_ACP, 0, lpCmdLine, -1, NULL, 0, NULL, NULL);
	cp = (char *)LocalAlloc(0, Length + 1);
	if (!cp) return;
	WideCharToMultiByte(CP_ACP, 0, lpCmdLine, -1, cp, Length + 1, NULL, NULL);
#else	/* not UNICODE */
	cp = lpCmdLine;
#endif	/* not UNICODE */
	/*
	 *	Scan the command line
	 */
	Length = 1;
	while (*cp) {
		/*
		 *	Skip any white space
		 */
		while(*cp) {
			if ((*cp != ' ') && (*cp != '\t')) break;
			cp++;
		}
		/*
		 *	Deal with quote characters
		 */
		if ((*cp == '"') || (*cp== '\'')) {	
			Quote_Character = *cp;
			cp++;
		}
		if (!*cp) break;
		/*
		 *	Mark the beginning of the argument
		 */
		Argv[Length++] = cp;
		/*
		 *	Find the end of the argument
		 */
		while(*cp) {
			if (Quote_Character) {
				/*
				 *	Look for the quote character
				 */
				if (*cp == Quote_Character) {
					Quote_Character = '\0';
					*cp++ = 0;
					break;
				}
			} else {
				/*
				 *	Look for white space
				 */
				if ((*cp == ' ') || (*cp == '\t')) {
					*cp++ = 0;
					break;
				}
			}
			cp++;
		}
	}
	/*
	 *	Return the argument count
	 */
	*ArgcP = Length;
}

/*
 *	Dynamically load the Python DLL
 */
static int Dynamically_Load_Python(void)
{
	HANDLE Handle;
	int i;

	/*
	 *	Load the python library
	 */
	Handle = LoadLibrary(TEXT("\\Program Files\\Python\\Lib\\") TEXT(_NAME(PYTHON_DLL_NAME)));
	if (!Handle) {
		Handle = LoadLibrary(TEXT(_NAME(PYTHON_DLL_NAME)));
		if (!Handle) {
			MessageBox(0, TEXT("Couldn't open "TEXT(_NAME(PYTHON_DLL_NAME))), TEXT("ERROR"), MB_OK);
			return(0);
		}
	}
	/*
	 *	Get all the Python symbols
	 */
	for(i = 0; i < (sizeof(Python_DLL_Symbols)/sizeof(Python_DLL_Symbols[0])); i++) {
		*Python_DLL_Symbols[i].SymbolP = (void *)GetProcAddress(Handle, Python_DLL_Symbols[i].Name);
		if (!*Python_DLL_Symbols[i].SymbolP) {
			TCHAR Message[100];

			wsprintf(Message, TEXT("Couldn't find symbol \"%s\" in \"%s\""), Python_DLL_Symbols[i].Name, TEXT(_NAME(PYTHON_DLL_NAME)));
			MessageBox(0, Message, TEXT("ERROR"), MB_OK);
			return(0);
		}
	}
	return(1);
}

/*
 *	Create the last chance error window (without requiring any Dialog Template resource)
 */
static void Create_Last_Chance_Error_Window(HINSTANCE Module_Instance, void *Info)
{
	char *p;
	DLGTEMPLATE *Dialog_Template;
	DLGITEMTEMPLATE *Dialog_Item_Template;
	static WCHAR Python_Error[] = L"Python Error";
	static WCHAR MS_Sans_Serif[] = L"MS Sans Serif";
	static WCHAR Close[] = L"Close";
#define	_ROUNDUP(x) (((x) + (sizeof(DWORD) - 1)) & ~(sizeof(DWORD) - 1))
	char Buffer[_ROUNDUP(sizeof(DLGTEMPLATE) + (2 * sizeof(WORD)) +
			     sizeof(Python_Error) + sizeof(WORD) + sizeof(MS_Sans_Serif)) +
		    _ROUNDUP(sizeof(DLGITEMTEMPLATE) + (4 * sizeof(WORD))) + 
		    _ROUNDUP(sizeof(DLGITEMTEMPLATE) + (2 * sizeof(WORD)) + sizeof(Close) + sizeof(WORD))];

	/*
	 *	Construct the dialog template header
	 */
	ZeroMemory(Buffer, sizeof(Buffer));
	Dialog_Template = (DLGTEMPLATE *)Buffer;
	Dialog_Template->style = (DS_SETFONT | DS_MODALFRAME | DS_CENTER  | DS_SETFOREGROUND |
				  WS_POPUP   | WS_VISIBLE    | WS_CAPTION | WS_SYSMENU);
	Dialog_Template->dwExtendedStyle = 0;
	Dialog_Template->cdit = 2;
	Dialog_Template->x = 0;
	Dialog_Template->y = 0;
	Dialog_Template->cx = 257;
	Dialog_Template->cy = 158;
	p = Buffer + sizeof(DLGTEMPLATE);
	((WORD *)p)[0] = 0;	/* No Menu */
	((WORD *)p)[1] = 0;	/* No Class */
	p += 2 * sizeof(WORD);
	CopyMemory(p, Python_Error, sizeof(Python_Error));
	p += sizeof(Python_Error);
	((WORD *)p)[0] = 8;	/* Font Size */
	p += sizeof(WORD);
	CopyMemory(p, MS_Sans_Serif, sizeof(MS_Sans_Serif));
	/*
	 *	Construct dialog item template #1 (the Edit Control)
	 */
	p = Buffer + _ROUNDUP(sizeof(DLGTEMPLATE) + (2 * sizeof(WORD)) +
			      sizeof(Python_Error) + sizeof(WORD) + sizeof(MS_Sans_Serif));
	Dialog_Item_Template = (DLGITEMTEMPLATE *)p;
	Dialog_Item_Template->style = (ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL |
				       WS_VSCROLL   | WS_HSCROLL     | WS_TABSTOP |
				       WS_BORDER    | WS_CHILD       | WS_VISIBLE);
	Dialog_Item_Template->dwExtendedStyle = 0;
	Dialog_Item_Template->x = 5;
	Dialog_Item_Template->y = 5;
	Dialog_Item_Template->cx = 245;
	Dialog_Item_Template->cy = 120;
	Dialog_Item_Template->id = LASTCHANCE_ERROR_WINDOW_EDIT_CONTROL_ID;
	p += sizeof(DLGITEMTEMPLATE);
	((WORD *)p)[0] = 0xffff;	/* System Class */
	((WORD *)p)[1] = 0x81;		/* Edit Control */
	((WORD *)p)[2] = 0;		/* No Title */
	((WORD *)p)[3] = 0;		/* No Creation Data */
	/*
	 *	Construct dialog item template #2 (the Push Button)
	 */
	p = Buffer + _ROUNDUP(sizeof(DLGTEMPLATE) + (2 * sizeof(WORD)) +
			      sizeof(Python_Error) + sizeof(WORD) + sizeof(MS_Sans_Serif)) +
		     _ROUNDUP(sizeof(DLGITEMTEMPLATE) + (4 * sizeof(WORD)));
	Dialog_Item_Template = (DLGITEMTEMPLATE *)p;
	Dialog_Item_Template->style = (BS_DEFPUSHBUTTON | WS_TABSTOP | WS_CHILD | WS_VISIBLE);
	Dialog_Item_Template->dwExtendedStyle = 0;
	Dialog_Item_Template->x = 200;
	Dialog_Item_Template->y = 135;
	Dialog_Item_Template->cx = 50;
	Dialog_Item_Template->cy = 14;
	Dialog_Item_Template->id = LASTCHANCE_ERROR_WINDOW_EDIT_CONTROL_ID + 1;
	p += sizeof(DLGITEMTEMPLATE);
	((WORD *)p)[0] = 0xffff;	/* System Class */
	((WORD *)p)[1] = 0x80;		/* Button */
	p += 2 * sizeof(WORD);
	CopyMemory(p, Close, sizeof(Close)); /* Title */
	p += sizeof(Close);
	((WORD *)p)[3] = 0;		/* No Creation Data */
	/*
	 *	Create the dialog
	 */
	DialogBoxIndirectParam(Module_Instance,
			       (LPCDLGTEMPLATE)Buffer,
			       NULL,
			       Dialog_Procedure,
			       (LPARAM)Info);
#undef	_ROUNDUP
}

/*
 *	Attempt to execute the PCCESHELL program stored in a resource
 */
static PyObject *Execute_PCCESHELL_Resource(HINSTANCE hInst)
{
	HRSRC Resource;
	PyObject *Module;

	/*
	 *	Assume failure
	 */
	Module = 0;
	/*
	 *	See if we can find the resource
	 */
	Resource = FindResource(hInst, TEXT("PCCESHELL"), TEXT("PYTHONSHELL"));
	if (Resource) {
		int Size;

		/*
		 *	Get its size
		 */
		Size = SizeofResource(NULL, Resource);
		if (Size != 0) {
			HGLOBAL Resource_Data;

			/*
			 *	Load it and get a pointer to its data
			 */
			Resource_Data = LoadResource(NULL, Resource);
			if (Resource_Data) {
				char *Data;

				Data = (char *)LockResource(Resource_Data);
				if (Data) {
					/*
					 *	Create a module to hold the program
					 */
					Module = PyImport_ImportModule("_pcceshell_support");
					if (Module) {
						PyObject *Dictionary;

						Dictionary = PyModule_GetDict(Module);
						if (Dictionary) {
							char *cp, *cp1;
							char *Program;

							/*
							 *	Put __builtins__ into the module
							 */
							PyDict_SetItemString(Dictionary, "__builtins__", PyEval_GetBuiltins());
							/*
							 *	Get a version of the string that has \r\n changed to \n
							 */
							Program = LocalAlloc(0, Size + 1);
							cp = Data;
							cp1 = Program;
							while(--Size >= 0) {
								if ((*cp == '\r') && (cp[1] == '\n')) {
									cp++;
									continue;
								}
								*cp1++ = *cp++;
							}
							/*
							 *	Null terminate it
							 */
							*cp1 = '\0';
							/*
							 *	We are done with the resource
							 */
							CloseHandle(Resource_Data);
							Resource_Data = 0;
							CloseHandle(Resource);
							Resource = 0;
							/*
							 *	Run the program
							 */
							PyRun_String(Program, Py_file_input, Dictionary, Dictionary);
							if (PyErr_Occurred()) Module = 0;
							/*
							 *	Done with the program data
							 */
							LocalFree(Program);
						}
					}
				}
				if (Resource_Data) CloseHandle(Resource_Data);
			}
		}
		if (Resource) CloseHandle(Resource);
	}
	return(Module);
}

/*
 *	Parsed options
 */
static int New_PCCESHELL = 0;	/* Non-zero if we are forcing a new shell */

/*
 *	Windows/CE Python startup
 */
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpCmdLine, int nCmdShow)
{	
	PyObject *Object;
	int Argc;
	char *Local_Argv[8];
	char **Argv = Local_Argv;
	char *Python_Program_Name = 0;
	char *Shell_Module_Name = 0;
	int Error = 0;
	int No_PCCESHELL = 0;
	PyObject *Module;
	FILE *fp;
	extern void Initialize_PCCESHELL_Support(void);

	/*
	 *	Make us look busy
	 */
	SetCursor(LoadCursor(0, IDC_WAIT));
	/*
	 *	Dynamically load the Python DLL
	 */
	if (!Dynamically_Load_Python()) return(FALSE);
	/*
	 *	Set up python
	 */
	Py_InteractiveFlag = 1;
	Py_SetProgramName("Python CE");
	Py_Initialize();
	/*
	 *	Let Python know about our instance
	 */
	Object = PyInt_FromLong((long)hInst);
	PySys_SetObject("hinst", Object);
	Py_DECREF(Object);
	/*
	 *	Parse the command line
	 */
	Parse_Command_Line(lpCmdLine, &Argc, Local_Argv);
	if (Argc == 1) Argc = 0;
	while(Argc > 1) {
		/*
		 *	Check for option (starts with "/")
		 */
		if (Argv[1][0] == '/') {
			/*
			 *	Dispatch on the first character of the option
			 */
			switch(Argv[1][1]) {
			  /*
			   *	Check for "/nopcceshell" or "/new" argument
			   */
			  case 'n':
			    if (strcmp(Argv[1], "/nopcceshell") == 0) {
				    No_PCCESHELL = 1;
				    Argc--;
				    Argv++;
				    continue;
			    }
			    if (strcmp(Argv[1], "/new") == 0) {
				    New_PCCESHELL = 1;
				    Argc--;
				    Argv++;
				    continue;
			    }
			    break;
		          /*
			   *	Check for "/shell" argument
			   */
			  case 's':
			    if (strcmp(Argv[1], "/shell") == 0) {
				    if (Argc > 2) {
					    Shell_Module_Name = Argv[2];
					    Argc--;
					    Argv++;
				    }
				    Argc--;
				    Argv++;
				    continue;
			    }
			    break;
			}
		}
		/*
		 *	Unrecognized: Use it as the name of a program to run
		 */
		Python_Program_Name = Argv[1];
		Argc--;
		Argv++;
		break;
	}
	/*
	 *	Give python the command line
	 */
	PySys_SetArgv(Argc, Argv);
	/*
	 *	Initialize the shell so we can get output
	 */
	if (!No_PCCESHELL) Initialize_PCCESHELL_Support();
	/*
	 *	See what to run
	 */
	if (Python_Program_Name) {
		/*
		 *	Open the program file
		 */
		fp = Py_fopen(Python_Program_Name, "r");
		if (!fp) {
			char *String;

			/*
			 *	Couldn't open the program file
			 */
			String = (char *)LocalAlloc(0, _MAX_PATH + 80);
			if (String) {
				PyOS_snprintf(String, _MAX_PATH + 80, "Can not open the file '%s'", Python_Program_Name);
				PyErr_SetString(PyExc_ValueError, String);
				LocalFree(String);
			}
			Error = 1;
		} else {
			/*
			 *	Run it
			 */
			Error = PyRun_SimpleFile(fp, Python_Program_Name);
			Py_fclose(fp);
		}
	} else {
		static char Python_RC[] = "pythonrc.py";

		/*
		 *	See if we have a ".pythonrc.py" file and run it
		 */
		fp = Py_fopen(Python_RC, "r");
		if (fp) {
			Error = PyRun_SimpleFile(fp, Python_RC);
			Py_fclose(fp);
		}
		if (!Error) {
			/*
			 *	No errors so far: Mark us not busy for the import
			 */
			SetCursor(LoadCursor(0, 0));
			/*
			 *	Import the initial module
			 */
			if (Shell_Module_Name) {
				Module = PyImport_ImportModule(Shell_Module_Name);
			} else {
				Module = Execute_PCCESHELL_Resource(hInst);
			}
			if (Module) {
				/*
				 *	If it was the default shell module, call its "main" function
				 */
				if (!Shell_Module_Name) {
					PyObject *Function;

					/*
					 *	Find "main" in the shell
					 */
					Function = PyObject_GetAttrString(Module, "main");
					if (Function) {
						PyObject *Result;

						/*
						 *	Call it
						 */
						Result = PyObject_CallObject(Function, NULL);
						if (!Result) Error = 1;
						Py_XDECREF(Result);
					} else {
						/*
						 *	"main" not found
						 */
						Error = 1;
					}
					/*
					 *	Dereference the "main" function
					 */
					Py_XDECREF(Function);
				}
				/*
				 *	Dereference the Module
				 */
				Py_DECREF(Module);
			} else {
				/*
				 *	Error
				 */
				Error = 1;
			}
		}
	}
	/*
	 *	Show any errors
	 */
	if (Error) {
		struct _Info Info;

		/*
		 *	Get the error information
		 */
		PyErr_Fetch(&Info.Type, &Info.Value, &Info.Traceback);
		/*
		 *	Put up the window
		 */
		SetCursor(LoadCursor(0, 0));
		Create_Last_Chance_Error_Window(hInst, &Info);
		/*
		 *	Dereference everything and we are done
		 */
		Py_XDECREF(Info.Type);
		Py_XDECREF(Info.Value);
		Py_XDECREF(Info.Traceback);
		return(FALSE);
	}
	return(TRUE);
}

	/********************************************************************************/
	/*										*/
	/*				pcceshell support				*/
	/*										*/
	/********************************************************************************/
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"

/*
 *	Globals
 */
static PyObject *PCCESHELL_Module = NULL;
static HWND PCCESHELL_Main_Window = NULL;
static HWND PCCESHELL_Edit_Window = NULL;
static HMENU PCCESHELL_File_Menu = NULL;
static HANDLE PCCESHELL_Events[2] = {0};
#define	PCCESHELL_CLOSE_EVENT		0
#define	PCCESHELL_INPUT_READY_EVENT	1
static int PCCESHELL_Is_Busy = 0;
static int PCCESHELL_Is_Interactive = 1;
static int PCCESHELL_Output_Flush_Timer = 0;
static int PCCESHELL_Output_Flush_Timer_Queued = 0;
static CRITICAL_SECTION PCCESHELL_Write_Queue_Critical_Section = {0};

/*
 *	The maximum amount of text we will allow in the edit window
 */
#define	PCCESHELL_MAX_EDIT_WINDOW_TEXT	29000

/*
 *	The amount of time to wait before flushing output to the edit window
 */
#define	PCCESHELL_FLUSH_OUTPUT_TIMEOUT	200	/* 200 msec */

/*
 *	The Window Title for the PCCESHELL
 */
static TCHAR PCCESHELL_Title[] = TEXT("Python CE");

/*
 *	Class name for the PCCESHELL main window class
 */
static TCHAR PCCESHELL_Main_Window_Class_Name[] = TEXT("PYTON_CE");

/*
 *	Under Windows/CE 3.0 we don't have ExitProcess() so use ExitThread() instead
 */
#if _WIN32_WCE <= 300
#define	ExitProcess(Code) ExitThread(Code)
#endif	/* _WIN32_WCE <= 300 */

/*
 *	Window procedure for the About dialog
 */
static LRESULT CALLBACK PCCESHELL_Aboutbox_Dialog_Window_Procedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
	  /*
	   *	Deal with commands
	   */
	  case WM_COMMAND:
		  if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL)) {
			  EndDialog(hWnd, 0);
			  return(FALSE);
		  }
		  break;
	  /*
	   *	Initialize the About dialog box
	   */
	  case WM_INITDIALOG: {
		  SHINITDLGINFO Init_Dialog_Info;

		  Init_Dialog_Info.dwMask = SHIDIM_FLAGS;
		  Init_Dialog_Info.hDlg = hWnd;
		  Init_Dialog_Info.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_SIPDOWN;
		  SHInitDialog(&Init_Dialog_Info);
		  return(FALSE);
	  }
	}
	return(DefWindowProc(hWnd, uMsg, wParam, lParam));
}

/*
 *	Window procedure for the main window
 */
static LRESULT CALLBACK PCCESHELL_Main_Window_Procedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static SHACTIVATEINFO Activate_Info = {0};
	switch(uMsg) {
	  /*
	   *	Destroy: Kill the application
	   */
	  case WM_DESTROY:
		  SetEvent(PCCESHELL_Events[PCCESHELL_CLOSE_EVENT]);
		  PostQuitMessage(0);
		  return(FALSE);
	  /*
	   *	Resize the edit window to track the main window size change
	   */
	  case WM_SIZE:
	  	  if (PCCESHELL_Edit_Window) {
			  RECT Rect;

			  /*
			   *	Re-size to the parent
			   */
			  GetClientRect(hWnd, &Rect);
			  SetWindowPos(PCCESHELL_Edit_Window,
				       HWND_TOP,
				       Rect.left,
				       Rect.top,
				       Rect.right - Rect.left,
				       Rect.bottom - Rect.top,
				       0);
			  ShowWindow(PCCESHELL_Edit_Window, SW_SHOWNORMAL);
			  /*
			   *	Make sure we are scrolled to the bottom
			   */
			  SendMessage(PCCESHELL_Edit_Window, EM_LINESCROLL, 0,
				      SendMessage(PCCESHELL_Edit_Window, EM_GETLINECOUNT, 0, 0));
			  return(FALSE);
		  }
		  break;
	  /*
	   *	Set focus to the edit window
	   */
	  case WM_SETFOCUS:
		  if (PCCESHELL_Edit_Window)  SetFocus(PCCESHELL_Edit_Window);
		  break;
	  /*
	   *	Deal with menu commands
	   */
	  case WM_COMMAND:
		  /*
		   *	If "Interrupt": Interrupt Python if it is executing something
		   */
		  if (LOWORD(wParam) == IDM_INTERRUPT) {
			  if (PCCESHELL_Is_Busy) _WinCE_Declare_Interrupt();
			  return(FALSE);
		  }
		  /*
		   *	If "Exit": destroy the window
		   */
		  if (LOWORD(wParam) == IDM_FILEEXIT) {
			  DestroyWindow(hWnd);
			  return(FALSE);
		  }
		  /*
		   *	If "About": Do the about dialog box
		   */
		  if (LOWORD(wParam) == IDM_HELPABOUT) {
			  DialogBox(GetModuleHandle(NULL),
				    MAKEINTRESOURCE(IDD_ABOUT),
				    hWnd,
				    PCCESHELL_Aboutbox_Dialog_Window_Procedure);
			  return(FALSE);
		  }
		  return(FALSE);
	  /*
	   *	Change settings
	   */
	  case WM_SETTINGCHANGE:
		  if (wParam == SPI_SETSIPINFO) {
			  SIPINFO SIP_Info;

			  /*
			   *	Handle the setting change like we are supposed to
			   */
			  SHHandleWMSettingChange(hWnd, wParam, lParam, &Activate_Info);
			  /*
			   *	Now look at the desktop real-estate and re-size accordingly
			   */
			  ZeroMemory(&SIP_Info, sizeof(SIP_Info));
			  SIP_Info.cbSize = sizeof(SIP_Info);
			  if (SipGetInfo(&SIP_Info)) {
				  SetWindowPos(hWnd, 0,
					       SIP_Info.rcVisibleDesktop.left,
					       SIP_Info.rcVisibleDesktop.top,
					       SIP_Info.rcVisibleDesktop.right - SIP_Info.rcVisibleDesktop.left,
					       SIP_Info.rcVisibleDesktop.bottom - SIP_Info.rcVisibleDesktop.top -
							((SIP_Info.fdwFlags & SIPF_ON) ? 0: 26),
					       0);
			  }
			  return(FALSE);
		  }
		  break;
	  /*
	   *	Activate window
	   */
	  case WM_ACTIVATE:
		  if (wParam == SPI_SETSIPINFO) {
			  SHHandleWMActivate(hWnd, wParam, lParam, &Activate_Info, 0);
			  return(FALSE);
		  }
		  break;
	}
	return(DefWindowProc(hWnd, uMsg, wParam, lParam));
}

/*
 *	Window procedure for the edit window
 */
static void *PCCESHELL_Old_Edit_Window_Procedure = 0;
static char *PCCESHELL_Get_Prompt(int Index, int *SizeP, int Use_Cache);
static void PCCESHELL_Write_Text(char *Text, int Force_Output);
static void PCCESHELL_Find_Input_Block(int *Block_StartP, int *Block_EndP);
static void CALLBACK PCCESHELL_Flush_Output(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);
static LRESULT CALLBACK PCCESHELL_Edit_Window_Procedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
	  /*
	   *	Deal with incoming characters
	   */
	  case WM_CHAR: {
		int Block_Start, Block_End;
		int n;

		/*
		 *	Only do this if we are interactive and the input character is <Return>
		 */
		if (!PCCESHELL_Is_Interactive || (wParam != '\r')) break;
		HideCaret(hWnd);
		/*
		 *	Find the block of input text
		 */
		PCCESHELL_Find_Input_Block(&Block_Start, &Block_End);
		/*
		 *	Get the first line in the block
		 */
		if (Block_Start >= 0) {
			TCHAR Buffer[512];

			n = Edit_GetLine(PCCESHELL_Edit_Window, Block_Start, Buffer, sizeof(Buffer)/sizeof(Buffer[0]));
			if ((n == 4) && ((Block_Start + 1) == Block_End)) {
				/*
				 *	Empty command: Write a new prompt
				 */
				PCCESHELL_Write_Text("\n", 0);
				PCCESHELL_Write_Text(PCCESHELL_Get_Prompt(0, 0, 1), 0);
			} else {
				/*
				 *	Notify the shell that we have data for it
				 */
				SetEvent(PCCESHELL_Events[PCCESHELL_INPUT_READY_EVENT]);
			}
		} else {
			/*
			 *	Not in a block: Write a new prompt
			 */
			PCCESHELL_Write_Text("\n", 0);
			PCCESHELL_Write_Text(PCCESHELL_Get_Prompt(0, 0, 1), 0);
		}
		ShowCaret(hWnd);
		return(FALSE);
	  }
	  /*
	   *	Deal with requests for a timer
	   */
	  case WM_USER:
		  if (!PCCESHELL_Output_Flush_Timer) {
			  PCCESHELL_Output_Flush_Timer = SetTimer(NULL, 0, PCCESHELL_FLUSH_OUTPUT_TIMEOUT, PCCESHELL_Flush_Output);
			  if (!PCCESHELL_Output_Flush_Timer) {
				  EnterCriticalSection(&PCCESHELL_Write_Queue_Critical_Section);
				  PCCESHELL_Output_Flush_Timer_Queued = 0;
				  LeaveCriticalSection(&PCCESHELL_Write_Queue_Critical_Section);
			  }
		  }
		  return(FALSE);
	}
	/*
	 *	If we have an old Edit window procedure, call it -- otherwise call DefWindowProc
	 */
	if (PCCESHELL_Old_Edit_Window_Procedure)
		return(CallWindowProc((WNDPROC)PCCESHELL_Old_Edit_Window_Procedure, hWnd, uMsg, wParam, lParam));
	else
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
}

/*
 *	Check if the Python Shell is already running
 */
static BOOL CALLBACK PCCESHELL_Check_If_Running(HWND hWnd, LPARAM lParam)
{
	TCHAR *cp, *cp1;
	int n;
	TCHAR Title[(sizeof(PCCESHELL_Title)/sizeof(PCCESHELL_Title[0])) + 1];

	/*
	 *	Get the window title
	 */
	n = GetWindowText(hWnd, Title, sizeof(Title)/sizeof(Title[0]));
	/*
	 *	If the string sizes don't match, continue enumerating
	 */
	if (n != ((sizeof(PCCESHELL_Title)/sizeof(PCCESHELL_Title[0])) - 1)) return(TRUE);
	/*
	 *	Look for a string match (on a mismatch continue enumerating)
	 */
	cp = Title;
	cp1 = PCCESHELL_Title;
	while(--n >= 0)
		if (*cp++ != *cp1++) return(TRUE);
	/*
	 *	Match: Put the window in the foreground and exit
	 */
	SetForegroundWindow(hWnd);
	ExitProcess(0);
	return(FALSE);
}

/*
 *	The shell window thread
 */
static DWORD WINAPI PCCESHELL_Window_Thread(LPVOID Main_Thread_ID)
{
	MSG Message;
	HINSTANCE Program_Instance;
	WNDCLASS Window_Class;
	SIPINFO SIP_Info;
	SHMENUBARINFO Menu_Bar_Info;
	RECT Rect;
	HWND Menu_Bar;

	/*
	 *	Initialize the write queue critical section
	 */
	InitializeCriticalSection(&PCCESHELL_Write_Queue_Critical_Section);
	/*
	 *	Get the main program instance
	 */
	Program_Instance = GetModuleHandle(NULL);
	/*
	 *	Initialize common controls
	 */
	InitCommonControls();
	/*
	 *	Register the window class
	 */
	ZeroMemory(&Window_Class, sizeof(Window_Class));
	Window_Class.hInstance = Program_Instance;
	Window_Class.style = CS_HREDRAW | CS_VREDRAW;
	Window_Class.hbrBackground = GetStockObject(WHITE_BRUSH);
	Window_Class.lpszClassName = PCCESHELL_Main_Window_Class_Name;
	Window_Class.lpfnWndProc = PCCESHELL_Main_Window_Procedure;
	RegisterClass(&Window_Class);
	/*
	 *	Create the main window using information from the Software Input Panel
	 */
	ZeroMemory(&SIP_Info, sizeof(SIP_Info));
	SIP_Info.cbSize = sizeof(SIP_Info);
	if (!SipGetInfo(&SIP_Info)) {
		MessageBox(0, TEXT("SipGetInfo failed"), TEXT("ERROR"), MB_OK);
		ExitProcess(0);
		return(0);
	}
	PCCESHELL_Main_Window = CreateWindow(PCCESHELL_Main_Window_Class_Name,
					     PCCESHELL_Title,
					     WS_VISIBLE | WS_BORDER,
					     SIP_Info.rcVisibleDesktop.left,
					     SIP_Info.rcVisibleDesktop.top,
					     SIP_Info.rcVisibleDesktop.right - SIP_Info.rcVisibleDesktop.left,
					     SIP_Info.rcVisibleDesktop.bottom - SIP_Info.rcVisibleDesktop.top -
						((SIP_Info.fdwFlags & SIPF_ON) ? 0: 26),
					     0,
					     0,
					     Program_Instance,
					     0);
	if (!PCCESHELL_Main_Window) {
		MessageBox(0, TEXT("CreateWindow failed"), TEXT("ERROR"), MB_OK);
		ExitProcess(0);
		return(0);
	}
	/*
	 *	Setup the menu bar
	 */
	ZeroMemory(&Menu_Bar_Info, sizeof(Menu_Bar_Info));
	Menu_Bar_Info.cbSize = sizeof(Menu_Bar_Info);
	Menu_Bar_Info.hwndParent = PCCESHELL_Main_Window;
	Menu_Bar_Info.nToolBarId = IDR_MENUBAR1;
	Menu_Bar_Info.hInstRes = Program_Instance;
	if (!SHCreateMenuBar(&Menu_Bar_Info)) {
		Menu_Bar_Info.dwFlags = SHCMBF_EMPTYBAR;
		Menu_Bar_Info.nToolBarId = 0;
		if (!SHCreateMenuBar(&Menu_Bar_Info)) {
			MessageBox(0, TEXT("SHCreateMenuBar failed"), TEXT("ERROR"), MB_OK);
			ExitProcess(0);
			return(0);
		}
	}
	/*
	 *	Get the handle to the File menu and disable the Interrupt menu item
	 */
	Menu_Bar = SHFindMenuBar(PCCESHELL_Main_Window);
	if (Menu_Bar) {
		PCCESHELL_File_Menu = (HMENU)SendMessage (Menu_Bar, SHCMBM_GETSUBMENU, 0, IDM_FILE);
		if (PCCESHELL_File_Menu) EnableMenuItem(PCCESHELL_File_Menu, IDM_INTERRUPT, MF_BYCOMMAND | MF_GRAYED);
	}
	/*
	 *	Create the Edit window and set its window procedure
	 */
	GetClientRect(PCCESHELL_Main_Window, &Rect);
	PCCESHELL_Edit_Window = CreateWindow(TEXT("EDIT"),
					     NULL,
					     WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
						ES_LEFT |ES_MULTILINE | ES_WANTRETURN | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
					     Rect.left,
					     Rect.top,
					     Rect.right - Rect.left,
					     Rect.bottom - Rect.top,
					     PCCESHELL_Main_Window,
					     NULL,
					     Program_Instance,
					     NULL);
	if (!PCCESHELL_Edit_Window) {
		MessageBox(0, TEXT("CreateWindow (Edit) failed"), TEXT("ERROR"), MB_OK);
		ExitProcess(0);
		return(0);
	}
	*(long *)&PCCESHELL_Old_Edit_Window_Procedure =
			SetWindowLong(PCCESHELL_Edit_Window, GWL_WNDPROC, (long)PCCESHELL_Edit_Window_Procedure);
	/*
	 *	Show the window
	 */
	ShowWindow(PCCESHELL_Main_Window, SW_SHOW);
	UpdateWindow(PCCESHELL_Main_Window);
	EnableWindow(PCCESHELL_Edit_Window, TRUE);
	SetFocus(PCCESHELL_Edit_Window);
	/*
	 *	Create the events we will need
	 */
	PCCESHELL_Events[PCCESHELL_CLOSE_EVENT] = CreateEvent(NULL, 0, 0, NULL);
	PCCESHELL_Events[PCCESHELL_INPUT_READY_EVENT] = CreateEvent(NULL, 0, 0, NULL);
	/*
	 *	Resume the main thread
	 */
	ResumeThread((HANDLE)Main_Thread_ID);
	/*
	 *	Run the message pump
	 */
	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return(0);
}

/*
 *	Initialize the shell window
 */
static void PCCESHELL_Initialize(void)
{
	DWORD My_Thread_ID, Thread_ID;
	HANDLE Thread;
	char *Prompt;
	int i;
	static char *Names[] = {"stdout", "stderr", "__stdout__", "__stderr"};

	/*
	 *	See if we are already running (unless inhibited)
	 */
	if (!New_PCCESHELL) EnumWindows(PCCESHELL_Check_If_Running, (LPARAM)0);
	/*
	 *	Make sure we have prompts set
	 */
	Prompt = PCCESHELL_Get_Prompt(0, &i, 0);
	PySys_SetObject("ps1", PyString_FromStringAndSize(Prompt, i));
	Prompt = PCCESHELL_Get_Prompt(1, &i, 0);
	PySys_SetObject("ps2", PyString_FromStringAndSize(Prompt, i));
	/*
	 *	Create the window thread
	 */
	My_Thread_ID = GetCurrentThreadId();
	Thread = CreateThread(0, 2 * 1024, PCCESHELL_Window_Thread, (LPVOID)My_Thread_ID, 0, &Thread_ID);
	if (!Thread) {
		MessageBox(0, TEXT("CreateThread failed"), TEXT("ERROR"), MB_OK);
		ExitProcess(0);
	}
	/*
	 *	Make it slightly higher priority so it will work well when other threads are CPU bound
	 */
	CeSetThreadPriority(Thread, CeGetThreadPriority(Thread) - 2);
	/*
	 *	Wait for it to initialize
	 */
	SuspendThread((HANDLE)My_Thread_ID);
	/*
	 *	Point stdout & stderr at our "write" function
	 */
	for(i = 0; i < (sizeof(Names)/sizeof(Names[0])); i++) PySys_SetObject(Names[i], PCCESHELL_Module);
}

/*
 *	Terminate the shell
 */
static PyObject *PCCESHELL_Terminate(PyObject *Self, PyObject *Args)
{
	PostMessage(PCCESHELL_Main_Window, WM_QUIT, 0, 0);
	return(PyInt_FromLong((long)0));
}
	
/*
 *	Put text into the Edit Window
 */
static void PCCESHELL_Put_Text_In_Edit_Window(char *Text)
{
	char *cp;
	TCHAR *Wide_Text, *cp1;
	int End_Position, Index, Length;
	int Text_Length;
	int End_Of_Lines;

	/*
	 *	Put us at the end of the window text
	 */
	End_Position = GetWindowTextLength(PCCESHELL_Edit_Window);
	SendMessage(PCCESHELL_Edit_Window, EM_SETSEL, End_Position, End_Position);
	/*
	 *	Calculate the length of the line with "\n" converted to "\r\n"
	 */
	cp = Text;
	End_Of_Lines = 0;
	while(*cp) if (*cp++ == '\n') End_Of_Lines++;
	Text_Length = cp - Text + End_Of_Lines;
	/*
	 *	Find the line Index at which the control will no longer be full
	 */
        Index = 0;
	Length = 0;
	while((End_Position + Text_Length - Length) > PCCESHELL_MAX_EDIT_WINDOW_TEXT) {
                Index = Index + 1;
                Length = SendMessage(PCCESHELL_Edit_Window, EM_LINEINDEX, Index, 0);
	}
	if (Index > 0) {
		/*
		 *	It is full, trim it
		 */
		SendMessage(PCCESHELL_Edit_Window, WM_SETREDRAW, (WPARAM)FALSE, 0);
		SendMessage(PCCESHELL_Edit_Window, EM_SETSEL, 0, Length);
		SendMessage(PCCESHELL_Edit_Window, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)TEXT(""));
		End_Position = GetWindowTextLength(PCCESHELL_Edit_Window);
		/*
		 *	Put us back at the end of the window
		 */
                SendMessage(PCCESHELL_Edit_Window, EM_SETSEL, End_Position, End_Position);
		SendMessage(PCCESHELL_Edit_Window, WM_SETREDRAW, (WPARAM)TRUE, 0);
	}
	/*
	 *	Convert the text to "\n\r" TCHAR
	 */
	Wide_Text = (TCHAR *)LocalAlloc(0, (Text_Length + 1) * sizeof(TCHAR));
	if (Wide_Text) {
		cp1 = Wide_Text;
		cp = Text;
		while(*cp) {
			if (*cp == '\n') *cp1++ = TEXT('\r');
			*cp1++ = (TCHAR)*cp++;
		}
		*cp1 = TEXT('\0');
		/*
		 *	Insert it
		 */
		SendMessage(PCCESHELL_Edit_Window, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)Wide_Text);
		LocalFree(Wide_Text);
	}
}

/*
 *	Write to the Edit Window (with queueing)
 */
static void PCCESHELL_Write_Text(char *Text, int Force_Output)
{
	char *cp;
	static char *Queued_Text = 0;
	static int Queued_Text_Length = 0;

	EnterCriticalSection(&PCCESHELL_Write_Queue_Critical_Section);
	/*
	 *	If we are busy, queue the data
	 */
	if (PCCESHELL_Is_Busy && !Force_Output) {
		/*
		 *	Make sure we have a buffer for queued text
		 */
		if (!Queued_Text) Queued_Text = (char *)LocalAlloc(0, PCCESHELL_MAX_EDIT_WINDOW_TEXT);
		if (Queued_Text) {
			int Input_Length;
			int Length;

			/*
			 *	Calculate the text length
			 */
			cp = Text;
			while(*cp) cp++;
			Input_Length = cp - Text;
			/*
			 *	Limit us to the length of the queued text buffer
			 */
			if (Input_Length >= PCCESHELL_MAX_EDIT_WINDOW_TEXT) {
				Text += Input_Length - PCCESHELL_MAX_EDIT_WINDOW_TEXT + 1;
				Input_Length = PCCESHELL_MAX_EDIT_WINDOW_TEXT - 1;
			}
			/*
			 *	Trim lines from queued text to make room for our text
			 */
			cp = Queued_Text;
			Length = Queued_Text_Length;
			while ((Queued_Text_Length + Input_Length) >= PCCESHELL_MAX_EDIT_WINDOW_TEXT) {
				while(Length > 0) {
					Length--;
					Queued_Text_Length--;
					if (*cp++ == '\n') break;
				}
			}
			if ((cp != Queued_Text) && (Queued_Text_Length > 0)) CopyMemory(Queued_Text, cp, Queued_Text_Length);
			/*
			 *	Append to the queued text
			 */
			CopyMemory(Queued_Text + Queued_Text_Length, Text, Input_Length);
			Queued_Text_Length += Input_Length;
			/*
			 *	If we don't have a flush timer queued, notify the Edit Window that we want one
			 */
			if (!PCCESHELL_Output_Flush_Timer_Queued) {
				PCCESHELL_Output_Flush_Timer_Queued = 1;
				PostMessage(PCCESHELL_Edit_Window, WM_USER, 0, 0);
			}
			LeaveCriticalSection(&PCCESHELL_Write_Queue_Critical_Section);
			return;
		}
	} else {
		/*
		 *	See if we have Queued Text to output
		 */
		if (Queued_Text_Length > 0) {
			/*
			 *	Yes: Grab the data
			 */
			cp = Queued_Text;
			cp[Queued_Text_Length] = '\0';
			Queued_Text_Length = 0;
			/*
			 *	Reset the queued text and put it into the edit window
			 */
			PCCESHELL_Put_Text_In_Edit_Window(cp);
		}
	}
	/*
	 *	Put the text into the edit window
	 */
	if (Text) PCCESHELL_Put_Text_In_Edit_Window(Text);
	LeaveCriticalSection(&PCCESHELL_Write_Queue_Critical_Section);
}

static PyObject *PCCESHELL_Write(PyObject *Self, PyObject *Args)
{
	char *Text;
	if (!PyArg_ParseTuple(Args, "s:Write", &Text)) return(NULL);
	PCCESHELL_Write_Text(Text, 0);
	return(PyInt_FromLong((long)0));
}

/*
 *	Flush output (here mostly because some applications expect a "flush" method for sys.stdout/sys.stderr)
 */
static PyObject *PCCESHELL_Flush(PyObject *Self, PyObject *Args)
{
	PCCESHELL_Write_Text(0, 1);
	Py_INCREF(Py_None);
	return(Py_None);
}

/*
 *	Timerproc to flush output to the edit window
 */
static void CALLBACK PCCESHELL_Flush_Output(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	/*
	 *	Mark that the timer fired
	 */
	EnterCriticalSection(&PCCESHELL_Write_Queue_Critical_Section);
	if (PCCESHELL_Output_Flush_Timer) {
		KillTimer(NULL, PCCESHELL_Output_Flush_Timer);
		PCCESHELL_Output_Flush_Timer = 0;
	}
	PCCESHELL_Output_Flush_Timer_Queued = 0;
	LeaveCriticalSection(&PCCESHELL_Write_Queue_Critical_Section);
	/*
	 *	Flush output
	 */
	if (PCCESHELL_Is_Busy) PCCESHELL_Write_Text(0, 1);
}

/*
 *	Mark whether or not we are busy
 */
static PyObject *PCCESHELL_Busy(PyObject *Self, PyObject *Args)
{
	/*
	 *	Get the arguments
	 */
	if (!PyArg_ParseTuple(Args, "i:Busy", &PCCESHELL_Is_Busy)) return(NULL);
	/*
	 *	Do the appropriate action
	 */
	if (PCCESHELL_Is_Busy) {
		/*
		 *	Busy: Set the busy cursor and enable the Interrupt menu item
		 */
		SetCursor(LoadCursor(0, IDC_WAIT));
		if (PCCESHELL_File_Menu) EnableMenuItem(PCCESHELL_File_Menu, IDM_INTERRUPT, MF_BYCOMMAND | MF_ENABLED);
	} else {
		/*
		 *	Not Busy: Take down the busy cursor and disable the Interrupt menu item
		 */
		SetCursor(LoadCursor(0, 0));
		if (PCCESHELL_File_Menu) EnableMenuItem(PCCESHELL_File_Menu, IDM_INTERRUPT, MF_BYCOMMAND | MF_GRAYED);
	}
	return(PyInt_FromLong((long)0));
}

/*
 *	Wait for input or exit
 */
static PyObject *PCCESHELL_Wait(PyObject *Self, PyObject *Args)
{
	int Result;

	/*
	 *	Wait for any event
	 */
	Py_BEGIN_ALLOW_THREADS
	Result = WaitForMultipleObjects(sizeof(PCCESHELL_Events)/sizeof(PCCESHELL_Events[0]),
					PCCESHELL_Events,
					FALSE,
					INFINITE);
	Py_END_ALLOW_THREADS
	if (Result == WAIT_FAILED) {
		Py_INCREF(Py_None);
		return(Py_None);
	}
	/*
	 *	Return the event index (0 is the exit event)
	 */
	return(PyInt_FromLong((long)(Result - WAIT_OBJECT_0)));
}

/*
 *	Get the named prompt
 */
static char *PCCESHELL_Get_Prompt(int Index, int *SizeP, int Use_Cache)
{
	PyObject *Prompt;
	char *Result = 0;
	int Result_Size;
	static char *Prompts[2] = {"ps1", "ps2"};
	static char Default_PS1[] = ">>> ";
	static char Default_PS2[] = "... ";
	static char *Default_Prompts[2] = {Default_PS1, Default_PS2};
	static int Default_Prompt_Lengths[2] = {sizeof(Default_PS1)-1, sizeof(Default_PS2)-1};
	static char *Prompt_Cache[2] = {0};
	static int Prompt_Cache_Lengths[2] = {0};

	if (Use_Cache) {
		/*
		 *	Get the prompt out of the cache
		 */
		if (Prompt_Cache[Index]) {
			/*
			 *	Return it
			 */
			if (SizeP) *SizeP = Prompt_Cache_Lengths[Index];
			return(Prompt_Cache[Index]);
		}
	} else {
		/*
		 *	Get the named prompt from the sys module
		 */
		Prompt = PySys_GetObject(Prompts[Index]);
		if (Prompt) {
			if (PyString_AsStringAndSize(Prompt, &Result, &Result_Size) == -1) {
				/*
				 *	Error: We have no string
				 */
				Result = 0;
			} else {
				char *cp;

				/*
				 *	Success: Make sure the prompt is in the cache
				 */
				cp = Prompt_Cache[Index];
				if (!cp || (Prompt_Cache_Lengths[Index] != Result_Size) || (strcmp(cp, Result) != 0)) {
					/*
					 *	No: Free whatever is currently in the cache
					 */
					if (cp) {
						LocalFree(Prompt_Cache[Index]);
						Prompt_Cache[Index] = 0;
						Prompt_Cache_Lengths[Index] = 0;
					}
					/*
					 *	Allocate new storage
					 */
					cp = (char *)LocalAlloc(0, Result_Size + 1);
					if (cp) {
						/*
						 *	Put the new prompt into the cache
						 */
						strcpy(cp, Result);
						Prompt_Cache[Index] = cp;
						Prompt_Cache_Lengths[Index] = Result_Size;
					}
				}
				/*
				 *	Return the prompt
				 */
				if (SizeP) *SizeP = Result_Size;
				return(Result);
			}
		}
	}
	/*
	 *	On error, use default prompts
	 */
	if (SizeP) *SizeP = Default_Prompt_Lengths[Index];
	return(Default_Prompts[Index]);
}

/*
 *	Find the block of input text in the edit window
 */
static void PCCESHELL_Find_Input_Block(int *Block_StartP, int *Block_EndP)
{
	int Current_Line;
	char *PS1, *PS2;
	int PS1_Length, PS2_Length;

	/*
	 *	Find the current line
	 */
	Current_Line = SendMessage(PCCESHELL_Edit_Window, EM_LINEINDEX, -1, 0);
	Current_Line = SendMessage(PCCESHELL_Edit_Window, EM_LINEFROMCHAR, Current_Line, 0);
	/*
	 *	Search backwards for a line starting with the PS1 prompt but
	 *	stop if we find a line that does not have the PS2 prompt
	 */
	PS1 = 0;
	*Block_StartP = -1;
	*Block_EndP = Current_Line + 1;
	while(Current_Line >= 0) {
		int i, n;
		char *cp;
		TCHAR *cp1;
		TCHAR Buffer[512];

		/*
		 *	Make sure we have the prompt information
		 */
		if (!PS1) {
			PS1 = PCCESHELL_Get_Prompt(0, &PS1_Length, 1);
			PS2 = PCCESHELL_Get_Prompt(1, &PS2_Length, 1);
		}
		/*
		 *	Get the next line
		 */
		n = Edit_GetLine(PCCESHELL_Edit_Window, Current_Line, Buffer, sizeof(Buffer)/sizeof(Buffer[0]));
		/*
		 *	Look for the PS1 prompt
		 */
		if (n >= PS1_Length) {
			for(cp = PS1, cp1 = Buffer, i = PS1_Length; i > 0; i--)
				if (*cp++ != (char)*cp1++) break;
			if (i <= 0) {
				/*
				 *	Found it: This is where the block starts
				 */
				*Block_StartP = Current_Line;
				break;
			}
		}
		/*
		 *	Look for the PS2 prompt
		 */
		if (n >= PS2_Length) {
			for(cp = PS2, cp1 = Buffer, i = PS2_Length; i > 0; i--)
				if (*cp++ != (char)*cp1++) break;
			if (i <= 0) {
				/*
				 *	Found it: Try the next line back
				 */
				Current_Line--;
				continue;
			}
		}
		/*
		 *	Didn't find it: Stop the search
		 */
		break;
	}
}

/*
 *	Get input text
 */
static PyObject *PCCESHELL_Get_Input_Text(PyObject *Self, PyObject *Args)
{
	int Block_Start, Block_End;

	/*
	 *	Find the block of input text
	 */
	PCCESHELL_Find_Input_Block(&Block_Start, &Block_End);
	/*
	 *	Get the first line in the block and see if we have a command
	 */
	if (Block_Start >= 0) {
		int n;
		char *PS1, *PS2;
		int PS1_Length, PS2_Length;
		int Prompt_Length;
		TCHAR Buffer[512];

		/*
		 *	Get the prompt information
		 */
		PS1 = PCCESHELL_Get_Prompt(0, &PS1_Length, 0);
		PS2 = PCCESHELL_Get_Prompt(1, &PS2_Length, 0);
		Prompt_Length = PS1_Length;
		/*
		 *	Get the first line and make sure we have a command in it
		 */
		n = Edit_GetLine(PCCESHELL_Edit_Window, Block_Start, Buffer, sizeof(Buffer)/sizeof(Buffer[0]));
		if ((n != Prompt_Length) || ((Block_Start + 1) != Block_End)) {
			PyObject *Result;
			TCHAR *cp;
			char *cp1;
			struct _Line {
				struct _Line *Next;
				int Length;
				char Data[1];
			} *Line, *Lines, *Last_Line;

			/*
			 *	Yes: Get all the lines into _Line structures
			 */
			Lines = 0;
			while(1) {
				n -= Prompt_Length;
				if (n >= 0) {
					/*
					 *	Allocate the _Line structure for this line
					 */
					Line = (struct _Line *)LocalAlloc(0, sizeof(*Line) + n);
					if (!Line) break;
					Line->Length = n;
					/*
					 *	Put the data into it
					 */
					cp = Buffer + Prompt_Length;
					cp1 = Line->Data;
					while(--n >= 0) *cp1++ = (char)*cp++;
					*cp1 = '\0';
					/*
					 *	Put it at the end of the list
					 */
					Line->Next = NULL;
					if (Lines)
						Last_Line->Next = Line;
					else
						Lines = Line;
					Last_Line = Line;
				}
				/*
				 *	Get the next line
				 */
				Block_Start++;
				if (Block_Start >= Block_End) break;
				n = Edit_GetLine(PCCESHELL_Edit_Window, Block_Start, Buffer, sizeof(Buffer)/sizeof(Buffer[0]));
				Prompt_Length = PS2_Length;
			}
			/*
			 *	Is the block at the end of the edit control?
			 */
			if (SendMessage(PCCESHELL_Edit_Window, EM_GETLINECOUNT, 0, 0) == Block_End) {
				/*
				 *	Yes: Only execute if we have a single line or multiple lines
				 *	     with a blank line at the end
				 */
				if (Lines && (!Lines->Next || (Last_Line->Length == 0))) {
					/*
					 *	Allocate a Python string to hold the lines concatenated with "\n"
					 */
					n = 0;
					for (Line = Lines; Line; Line = Line->Next) n += Line->Length + 1;
					Result = PyString_FromStringAndSize(NULL, n);
					if (Result) {
						/*
						 *	Fill in the string
						 */
						cp1 = PyString_AS_STRING(Result);
						n = 0;
						for (Line = Lines; Line; Line = Line->Next) {
							if (Line->Length > 0) {
								CopyMemory(cp1 + n, Line->Data, Line->Length);
								n += Line->Length;
							}
							cp1[n++] = '\n';
						}
					}
				} else {
					/*
					 *	Don't execute
					 */
					PCCESHELL_Write_Text("\n", 0);
					PCCESHELL_Write_Text(PS2, 0);
					Py_INCREF(Py_None);
					Result = Py_None;
				}
			} else {
				char *Prompt = PS1;

				/*
				 *	No: Copy the data to the end of the edit control
				 */
				for (Line = Lines; Line; Line = Line->Next) {
					PCCESHELL_Write_Text("\n", 0);
					PCCESHELL_Write_Text(Prompt, 0);
					PCCESHELL_Write_Text(Line->Data, 0);
					Prompt = PS2;
				}
				/*
				 *	Don't execute
				 */
				Py_INCREF(Py_None);
				Result = Py_None;
			}
			/*
			 *	Free up the lines
			 */
			while(Lines) {
				Line = Lines;
				Lines = Line->Next;
				LocalFree(Line);
			}
			/*
			 *	Return the string
			 */
			return(Result);
		}
	}
	Py_INCREF(Py_None);
	return(Py_None);
}

/*
 *	Get an edit window line
 */
static PyObject *PCCESHELL_Get_Edit_Window_Line(PyObject *Self, PyObject *Args)
{
	int n, Length;
	int Line_Number;
	int Number_Of_Lines;
	TCHAR *cp;
	char *cp1;
	TCHAR Buffer[512];

	/*
	 *	Get the line number
	 */
	if (!PyArg_ParseTuple(Args, "i:Get_Edit_Window_Line", &Line_Number)) return(NULL);
	/*
	 *	Get the number of lines in the window
	 */
	Number_Of_Lines = SendMessage(PCCESHELL_Edit_Window, EM_GETLINECOUNT, 0, 0);
	/*
	 *	A negative number means that number of lines from the end
	 */
	if (Line_Number < 0) Line_Number += Number_Of_Lines;
	/*
	 *	If the Line_Number is past the end, return None
	 */
	if (Line_Number >= Number_Of_Lines) {
		Py_INCREF(Py_None);
		return(Py_None);
	}
	/*
	 *	Get the line and convert it to ascii
	 */
	Length = Edit_GetLine(PCCESHELL_Edit_Window, Line_Number, Buffer, sizeof(Buffer)/sizeof(Buffer[0]));
	cp = Buffer;
	cp1 = (char *)Buffer;
	n = Length;
	while(--n >= 0) *cp1++ = (char)*cp++;
	/*
	 *	Return it as a string
	 */
	return(PyString_FromStringAndSize((char *)Buffer, Length));
}

/*
 *	Do character input for the edit window
 */
static PyObject *PCCESHELL_Character_Input(PyObject *Self, PyObject *Args)
{
	int Character;

	/*
	 *	Get the character
	 */
	if (!PyArg_ParseTuple(Args, "i:CharacteR_Input", &Character)) return(NULL);
	/*
	 *	Send it to the edit window
	 */
	SendMessage(PCCESHELL_Edit_Window, WM_CHAR, Character, 0);
	Py_INCREF(Py_None);
	return(Py_None);
}

/*
 *	PCCESHELL support methods
 */
static PyMethodDef PCCESHELL_Methods[] = {
	{"Terminate",			PCCESHELL_Terminate,		METH_NOARGS},
	{"write",			PCCESHELL_Write,		METH_VARARGS},
	{"flush",			PCCESHELL_Flush,		METH_NOARGS},
	{"Busy",			PCCESHELL_Busy,			METH_VARARGS},
	{"Wait",			PCCESHELL_Wait,			METH_NOARGS},
	{"Get_Input_Text",		PCCESHELL_Get_Input_Text,	METH_NOARGS},
	{"Character_Input",		PCCESHELL_Character_Input,	METH_VARARGS},
	{"Get_Edit_Window_Line",	PCCESHELL_Get_Edit_Window_Line,	METH_VARARGS},
	{NULL, NULL}};

/*
 *	Initialize the PCCESHELL support module
 */
void Initialize_PCCESHELL_Support(void)
{
	/*
	 *	Initialize the module
	 */
	PCCESHELL_Module = Py_InitModule("_pcceshell_support", PCCESHELL_Methods);
	Py_INCREF(PCCESHELL_Module);
	PCCESHELL_Initialize();
}
