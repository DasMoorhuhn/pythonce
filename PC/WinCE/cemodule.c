/*
 *	"ce" Python module (imported by os.py)
 *
 *	O/S-dependent functions for Windows/CE
 *
 *	David Kashtan, Validus Medical Systems
 */
#include "Python.h"
#include "structseq.h"

/*
 *	Internal function for converting a string/unicode Python object into an absolute path wide string
 */
extern int _WinCE_Absolute_Path(PyObject *Path, WCHAR *Buffer, int Buffer_Size);

#define	MODULE_NAME "ce"

/*
 *	OS-Dependent listdir() function
 */
static PyObject *OS_listdir(PyObject *Self, PyObject *Args)
{
	PyObject *Path;
	PyObject *Result, *Value;
	HANDLE File_Handle;
	int Length;
	int Doing_Unicode;
	WIN32_FIND_DATA Data;
	WCHAR Wide_Path[MAX_PATH + 1];

	/*
	 *	Get the args
	 */
	if(!PyArg_ParseTuple(Args, "O", &Path))	return(NULL);
	/*
	 *	Get a wide-string absolute path and format the directory path appropriately
	 */
	if (!_WinCE_Absolute_Path(Path, Wide_Path, MAX_PATH + 1)) return(NULL);
	Length = wcslen(Wide_Path);
	if((Wide_Path[Length - 1] != TEXT('/')) && (Wide_Path[Length - 1] != TEXT('\\')))
		wsprintf(Wide_Path + Length, L"\\*.*");
	else
		wsprintf(Wide_Path + Length, L"*.*");

	/*
	 *	We need to remember whether or not we are dealing with Unicode strings
	 */
	Doing_Unicode = PyUnicode_Check(Path) ? 1 : 0;
	/*
	 *	Get the first file in the directory
	 */
	File_Handle = FindFirstFile(Wide_Path, &Data);
	if(File_Handle == INVALID_HANDLE_VALUE) {
		int Error = GetLastError();

		/*
		 *	Check for empty directory
		 */
		if (Error == ERROR_NO_MORE_FILES) {
			/*
			 *	Yes: Return an empty list
			 */
			return(PyList_New(0));
		}
		/*
		 *	Error:
		 */
		errno = Error;
		PyErr_SetFromErrnoWithFilenameObject(PyExc_OSError, Path);
		return(NULL);
	}
	/*
	 *	Initialize the result list
	 */
	Result = PyList_New(0);
	if (!Result) return(NULL);
	/*
	 *	Process the directory entries
	 */
	do {
		int n;

		/*
		 *	Skip "." and ".."
		 */
		if (((Data.cFileName[0] == L'.')) &&
		    ((Data.cFileName[1] == L'\0') ||
		     (Data.cFileName[1] == L'.') && (Data.cFileName[2] == L'\0')))
			continue;
		/*
		 *	Turn the directory entry into a Python string
		 */
		if (Doing_Unicode) {
			Value = PyUnicode_FromUnicode(Data.cFileName, wcslen(Data.cFileName));
		} else {
			n = WideCharToMultiByte(CP_ACP, 0, Data.cFileName, -1, (char *)Data.cFileName, sizeof(Data.cFileName),
						NULL, NULL);
			Value = PyString_FromStringAndSize((char *)Data.cFileName, n - 1);
		}
		if (!Value) {
			/*
			 *	Out of memory: Dereference everything
			 */
			Py_DECREF(Result);
			Result = NULL;
			break;
		}
		/*
		 *	Append it to the list
		 */
		if(PyList_Append(Result, Value) != 0)
		{
			/*
			 *	Out of memory: Dereference everything
			 */
			Py_DECREF(Value);
			Py_DECREF(Result);
			Result = NULL;
			break;
		}
		/*
		 *	PyList_Append made its own reference to Value, so we need to dereference it
		 */
		Py_DECREF(Value);
	} while(FindNextFile(File_Handle, &Data) == TRUE);
	FindClose(File_Handle);
	/*
	 *	Return the result
	 */
	return(Result);
}

/*
 *	OS-Dependent mkdir() function
 */
static PyObject *OS_mkdir(PyObject *Self, PyObject *Args)
{
	PyObject *Path;
	WCHAR Wide_Path[MAX_PATH + 1];
	LONG  Mode;
	BOOL  Result;

	/*
	 *	Get the args
	 */
	if(!PyArg_ParseTuple(Args, "O|i", &Path, &Mode)) return(NULL);
	if (!_WinCE_Absolute_Path(Path, Wide_Path, MAX_PATH + 1)) return(NULL);
	/*
	 *	Get a wide-string absolute path and create the directory (with other threads enabled)
	 */
	Py_BEGIN_ALLOW_THREADS
	Result = CreateDirectory(Wide_Path, NULL);
	Py_END_ALLOW_THREADS
	/*
	 *	Deal with errors
	 */
	if (!Result) {
		errno = GetLastError();
        /* This check was removed because it is incompatible with WinNT mkdir() and breaks test.test_os
		if (errno != ERROR_ALREADY_EXISTS) {
			PyErr_SetFromErrnoWithFilenameObject(PyExc_OSError, Path);
			return(NULL);
		}
        */
        PyErr_SetFromErrnoWithFilenameObject(PyExc_OSError, Path);
        return(NULL);
	}
	/*
	 *	Return success
	 */
	Py_INCREF(Py_None);
	return(Py_None);
}

/*
 *	OS-Dependent rename() function
 */
static PyObject *OS_rename(PyObject *Self, PyObject *Args)
{
	PyObject *Path1, *Path2;
	WCHAR Wide_Path1[MAX_PATH + 1];
	WCHAR Wide_Path2[MAX_PATH + 1];
	BOOL  Result;

	/*
	 *	Get the args
	 */
	if (!PyArg_ParseTuple(Args, "OO", &Path1, &Path2)) return(NULL);
	if (!_WinCE_Absolute_Path(Path1, Wide_Path1, MAX_PATH + 1)) return(NULL);
	if (!_WinCE_Absolute_Path(Path2, Wide_Path2, MAX_PATH + 1)) return(NULL);
	/*
	 *	Rename the file (with other threads enabled)
	 */
	Py_BEGIN_ALLOW_THREADS
	Result = MoveFile(Wide_Path1, Wide_Path2);
	Py_END_ALLOW_THREADS
	/*
	 *	Deal with errors
	 */
	if (!Result) {
		errno = GetLastError();
		PyErr_SetFromErrnoWithFilenameObject(PyExc_OSError, Path1);
		return(NULL);
	}
	/*
	 *	Return success
	 */
	Py_INCREF(Py_None);
	return(Py_None);
}

/*
 *	OS-Dependent rmdir() function
 */
static PyObject *OS_rmdir(PyObject *Self, PyObject *Args)
{
	PyObject *Path;
	WCHAR Wide_Path[MAX_PATH + 1];
	BOOL  Result;

	/*
	 *	Parse the args
	 */
	if (!PyArg_ParseTuple(Args, "O", &Path)) return(NULL);
	if (!_WinCE_Absolute_Path(Path, Wide_Path, MAX_PATH + 1)) return(NULL);
	/*
	 *	Delete the directory (with other threads enabled)
	 */
	Py_BEGIN_ALLOW_THREADS
	Result = RemoveDirectory(Wide_Path);
	Py_END_ALLOW_THREADS
	/*
	 *	Deal with errors
	 */
	if (!Result) {
		errno = GetLastError();
		PyErr_SetFromErrnoWithFilenameObject(PyExc_OSError, Path);
		return(NULL);
	}
	/*
	 *	Return success
	 */
	Py_INCREF(Py_None);
	return(Py_None);
}

/*
 *	stat() documentation
 */
PyDoc_STRVAR(stat_result__doc__,
"stat_result: Result from stat or lstat.\n\n\
This object may be accessed either as a tuple of\n\
  (mode, ino, dev, nlink, uid, gid, size, atime, mtime, ctime)\n\
or via the attributes st_mode, st_ino, st_dev, st_nlink, st_uid, and so on.\n\
\n\
Posix/windows: If your platform supports st_blksize, st_blocks, or st_rdev,\n\
they are available as attributes only.\n\
\n\
See os.stat for more information.");

/*
 *	stat() result fields
 */
static PyStructSequence_Field Stat_Result_Fields[] = {
	{"st_mode",    "protection bits"},
	{"st_ino",     "inode"},
	{"st_dev",     "device"},
	{"st_nlink",   "number of hard links"},
	{"st_uid",     "user ID of owner"},
	{"st_gid",     "group ID of owner"},
	{"st_size",    "total size, in bytes"},
	/* The NULL is replaced with PyStructSequence_UnnamedField in initce() */
	{NULL,	       "integer time of last access"},
	{NULL,	       "integer time of last modification"},
	{NULL,	       "integer time of last change"},
	{"st_atime",   "time of last access"},
	{"st_mtime",   "time of last modification"},
	{"st_ctime",   "time of last change"},
	{0}};

/*
 *	Description of the stat() result
 */
static PyStructSequence_Desc Stat_Result_Descriptor = {
	"stat_result",		/* Name */
	stat_result__doc__,	/* Documentation */
	Stat_Result_Fields,	/* Fields */
	10};

/*
 *	Pack a system stat C structure into the Python stat tuple
 */
static PyTypeObject Stat_Result_Type;
static newfunc New_Stat_Structure_Sequence;
static PyObject *New_Stat_Result(PyTypeObject *Type, PyObject *Args, PyObject *Keywords)
{
	PyStructSequence *Result;
	int i;

	/*
	 *	Allocate the result
	 */
	Result = (PyStructSequence*)New_Stat_Structure_Sequence(Type, Args, Keywords);
	if (Result) {
		/*
		 *	If we have been initialized from a tuple,
		 *	st_?time might be set to None. Initialize it
		 *	from the int slots.
		 */
		for (i = 7; i <= 9; i++) {
			if (Result->ob_item[i + 3] == Py_None) {
				Py_DECREF(Py_None);
				Py_INCREF(Result->ob_item[i]);
				Result->ob_item[i + 3] = Result->ob_item[i];
			}
		}
	}
	return (PyObject*)Result;
}

/*
 *	Given a stat structure, return a Python stat structure
 */
static PyObject *Create_stat_Result(struct stat *st)
{
	PyObject *Result;

	/*
	 *	Create the result
	 */
	Result = PyStructSequence_New(&Stat_Result_Type);
	if (Result) {
		int i;
		time_t *Times[3];

		/*
		 *	Fill in the values
		 */
		PyStructSequence_SET_ITEM(Result, 0, PyInt_FromLong((long)st->st_mode));
		PyStructSequence_SET_ITEM(Result, 1, PyInt_FromLong((long)st->st_ino));
		PyStructSequence_SET_ITEM(Result, 2, PyInt_FromLong((long)st->st_dev));
		PyStructSequence_SET_ITEM(Result, 3, PyInt_FromLong((long)st->st_nlink));
		PyStructSequence_SET_ITEM(Result, 4, PyInt_FromLong((long)st->st_uid));
		PyStructSequence_SET_ITEM(Result, 5, PyInt_FromLong((long)st->st_gid));
		PyStructSequence_SET_ITEM(Result, 6, PyInt_FromLong(st->st_size));
		/*
		 *	Do the atime/mtime/ctime values at index 7/8/9 and 10/11/12
		 */
		Times[0] = &st->st_atime;
		Times[1] = &st->st_mtime;
		Times[2] = &st->st_ctime;
		for (i = 0; i < 3; i++) {
			PyObject *Value;

			Value = PyInt_FromLong((long)*Times[i]);
			PyStructSequence_SET_ITEM(Result, i + 7, Value);
			Py_INCREF(Value);
			PyStructSequence_SET_ITEM(Result, i + 7 + 3, Value);
		}
		/*
		 *	Check for errors
		 */
		if (PyErr_Occurred()) {
			Py_DECREF(Result);
			Result = NULL;
		}
	}
	return(Result);
}

/*
 *	OS-Dependent stat() function
 */
static PyObject *OS_stat(PyObject *Self, PyObject *Args)
{
	PyObject *Path;
	int i;
	WCHAR Wide_Path[MAX_PATH + 1];
	struct stat st;

	/*
	 *	Parse the args
	 */
	if(!PyArg_ParseTuple(Args, "O", &Path)) return(NULL);
	if (!_WinCE_Absolute_Path(Path, Wide_Path, MAX_PATH + 1)) return(NULL);
	/*
	 *	Do the stat() operation (with other threads enabled)
	 */
	Py_BEGIN_ALLOW_THREADS
	i = _wstat(Wide_Path, &st);
	Py_END_ALLOW_THREADS
	/*
	 *	Check for errors
	 */
	if (i != 0) {
		PyErr_SetFromErrnoWithFilenameObject(PyExc_OSError, Path);
		return(NULL);
	}
	/*
	 *	Return the Python result
	 */
	return(Create_stat_Result(&st));
}

/*
 *	OS-Dependent fstat() function
 */
static PyObject *OS_fstat(PyObject *Self, PyObject *Args)
{
	HANDLE Handle;
	int i;
	struct stat st;
	extern int WinCE_Handle_fstat(HANDLE, struct stat *);

	/*
	 *	Parse the args
	 */
	if(!PyArg_ParseTuple(Args, "i:fstat", &Handle)) return(NULL);
	/*
	 *	Do the WinCE_Handle_fstat() operation (with other threads enabled)
	 */
	Py_BEGIN_ALLOW_THREADS
	i = WinCE_Handle_fstat(Handle, &st);
	Py_END_ALLOW_THREADS
	/*
	 *	Check for errors
	 */
	if (i != 0) {
		PyErr_SetFromErrno(PyExc_OSError);
		return(NULL);
	}
	/*
	 *	Return the Python result
	 */
	return(Create_stat_Result(&st));
}

/*
 *	OS-dependent unlink() function
 */
static PyObject *OS_unlink(PyObject *Self, PyObject *Args)
{
	PyObject *Path;
	WCHAR Wide_Path[MAX_PATH + 1];
	BOOL Result;

	/*
	 *	Parse the args
	 */
	if (!PyArg_ParseTuple(Args, "O", &Path)) return(NULL);
	if (!_WinCE_Absolute_Path(Path, Wide_Path, MAX_PATH + 1)) return(NULL);
	/*
	 *	Get a wide-string path and delete the file (with other threads enabled)
	 */
	Py_BEGIN_ALLOW_THREADS
	Result = DeleteFile(Wide_Path);
	Py_END_ALLOW_THREADS
	/*
	 *	Deal with errors
	 */
	if (!Result) {
		errno = GetLastError();
		PyErr_SetFromErrnoWithFilenameObject(PyExc_OSError, Path);
		return(NULL);
	}
	/*
	 *	Return success
	 */
	Py_INCREF(Py_None);
	return(Py_None);
}

/*
 *	OS-dependent open() function
 */
#define	O_RDWR		(1<<0)
#define	O_RDONLY	(2<<0)
#define	O_WRONLY	(3<<0)
#define	O_MODE_MASK	(3<<0)
#define	O_TRUNC		(1<<2)
#define	O_EXCL		(1<<3)
#define	O_CREAT		(1<<4)
static PyObject *OS_open(PyObject *Self, PyObject *Args)
{
	PyObject *Filename;
	int Flags = O_RDONLY;
	int Mode = 0;
	DWORD Access, ShareMode, CreationDisposition;
	HANDLE Handle;
	WCHAR Wide_Filename[MAX_PATH + 1];
	static int Modes[4] = {0, (GENERIC_READ | GENERIC_WRITE), GENERIC_READ, GENERIC_WRITE};

	/*
	 *	Parse the args
	 */
	if (!PyArg_ParseTuple(Args, "O|ii:open", &Filename, &Flags, &Mode)) return(NULL);
	if (!_WinCE_Absolute_Path(Filename, Wide_Filename, MAX_PATH + 1)) return(NULL);
	/*
	 *	Calculate the CreateFile arguments
	 */
	Access = Modes[Flags & O_MODE_MASK];
	ShareMode = (Flags & O_EXCL) ? 0 : (FILE_SHARE_READ | FILE_SHARE_WRITE);
	if (Flags & O_TRUNC)
		CreationDisposition = (Flags & O_CREAT) ? CREATE_ALWAYS : TRUNCATE_EXISTING;
	else
		CreationDisposition = (Flags & O_CREAT) ? CREATE_NEW : OPEN_EXISTING;
	/*
	 *	Get a wide-string absolute filename and do the create
	 */
	Py_BEGIN_ALLOW_THREADS
	Handle = CreateFile(Wide_Filename, Access, ShareMode, NULL, CreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
	Py_END_ALLOW_THREADS
	/*
	 *	Deal with errors
	 */
	if (Handle == INVALID_HANDLE_VALUE) {
		errno = GetLastError();
		if ((errno == ERROR_ALREADY_EXISTS) || (errno == ERROR_FILE_EXISTS)) errno = EEXIST;
		PyErr_SetFromErrnoWithFilenameObject(PyExc_OSError, Filename);
		return(NULL);
	}
	/*
	 *	Return the handle
	 */
	return(PyInt_FromLong((long)Handle));
}

/*
 *	OS-dependent fdopen() function
 */
static PyObject *OS_fdopen(PyObject *Self, PyObject *Args)
{
	HANDLE Handle;
	char *Mode = "r";
	int Bufsize = -1;
	FILE *fp;
	PyObject *File;
	WCHAR Wide_Mode[8];

	/*
	 *	Parse the args
	 */
	if (!PyArg_ParseTuple(Args, "i|si:fdopen", &Handle, &Mode, &Bufsize)) return NULL;
	/*
	 *	Check the file mode and convert to WCHAR
	 */
	if ((Mode[0] != 'r') && (Mode[0] != 'w') && (Mode[0] != 'a')) {
		PyErr_Format(PyExc_ValueError, "invalid file mode '%s'", Mode);
		return(NULL);
	}
	MultiByteToWideChar(CP_ACP, 0, Mode, -1, Wide_Mode, (sizeof(Wide_Mode)/sizeof(Wide_Mode[0])));
	/*
	 *	Get a stdio FILE pointer to the handle in the given mode
	 */
	Py_BEGIN_ALLOW_THREADS
	fp = _wfdopen(Handle, Wide_Mode);
	Py_END_ALLOW_THREADS
	if (fp == NULL) {
		/*
		 *	Error:
		 */
		errno = GetLastError();
		PyErr_SetFromErrno(PyExc_OSError);
		return(NULL);
	}
	/*
	 *	Return a Python file object
	 */
	File = PyFile_FromFile(fp, "<fdopen>", Mode, fclose);
	if (File) PyFile_SetBufSize(File, Bufsize);
	return(File);
}

/*
 *	OS-dependent read() function
 */
static PyObject *OS_read(PyObject *Self, PyObject *Args)
{
	HANDLE Handle;
	PyObject *Buffer;
	int Size, n;
	BOOL Status;

	/*
	 *	Parse the args
	 */
	if (!PyArg_ParseTuple(Args, "ii:read", &Handle, &Size)) return(NULL);
	/*
	 *	Allocate room for the read
	 */
	Buffer = PyString_FromStringAndSize((char *)NULL, Size);
	if (!Buffer) return(NULL);
	/*
	 *	Do the read (with other threads enabled)
	 */
	Py_BEGIN_ALLOW_THREADS
	Status = ReadFile(Handle, PyString_AsString(Buffer), Size, &n, 0);
	Py_END_ALLOW_THREADS
	if (!Status) {
		/*
		 *	Error:
		 */
		Py_DECREF(Buffer);
		errno = GetLastError();
		PyErr_SetFromErrno(PyExc_OSError);
		return(NULL);
	}
	/*
	 *	If necessary, resize the string we are going to return
	 */
	if (n != Size) _PyString_Resize(&Buffer, n);
	return(Buffer);
}

/*
 *	OS-dependent write() function
 */
static PyObject *OS_write(PyObject *Self, PyObject *Args)
{
	HANDLE Handle;
	char *Buffer;
	int Size;
	BOOL Status;

	/*
	 *	Parse the args
	 */
	if (!PyArg_ParseTuple(Args, "is#:write", &Handle, &Buffer, &Size)) return(NULL);
	/*
	 *	Do the write (with other threads enabled)
	 */
	Py_BEGIN_ALLOW_THREADS
	Status = WriteFile(Handle, Buffer, Size, &Size, 0);
	Py_END_ALLOW_THREADS
	if (!Status) {
		/*
		 *	Error:
		 */
		errno = GetLastError();
		PyErr_SetFromErrno(PyExc_OSError);
		return(NULL);
	}
	return PyInt_FromLong((long)Size);
}

/*
 *	OS-dependent close() function
 */
static PyObject *OS_close(PyObject *Self, PyObject *Args)
{
	HANDLE Handle;
	BOOL Result;

	/*
	 *	Parse the args
	 */
	if (!PyArg_ParseTuple(Args, "i:close", &Handle)) return(NULL);
	/*
	 *	Close the handle
	 */
	Py_BEGIN_ALLOW_THREADS
	Result = CloseHandle(Handle);
	Py_END_ALLOW_THREADS
	if (!Result) {
		/*
		 *	Error:
		 */
		errno = GetLastError();
		PyErr_SetFromErrno(PyExc_OSError);
		return(NULL);
	}
	/*
	 *	Success
	 */
	Py_INCREF(Py_None);
	return(Py_None);
}

/*
 *	OS-dependent getcwd() function
 */
static PyObject *OS_getcwd(PyObject *Self, PyObject *Args)
{
	char Buffer[MAX_PATH + 1];

	getcwd(Buffer, sizeof(Buffer));
	return(PyString_FromString(Buffer));
}
	
/*
 *	OS-dependent getcwdu() function
 */
static PyObject *OS_getcwdu(PyObject *Self, PyObject *Args)
{
	wchar_t Buffer[MAX_PATH + 1];

	wgetcwd(Buffer, sizeof(Buffer));
	return(PyUnicode_FromUnicode(Buffer, wcslen(Buffer)));
}
	
/*
 *	OS-Dependent chmod() function
 */
static PyObject *OS_chmod(PyObject *Self, PyObject *Args)
{
	PyObject *Path;
	int Mode;
	WCHAR Wide_Path[MAX_PATH + 1];
    DWORD attr, newattr;

	/*
	 *	Parse the args
	 */
	if(!PyArg_ParseTuple(Args, "Oi:chmod", &Path, &Mode)) return(NULL);
	if (!_WinCE_Absolute_Path(Path, Wide_Path, MAX_PATH + 1)) return(NULL);
    /* Update the read-only flag */
    attr = GetFileAttributes(Wide_Path);
    if(attr == 0xFFFFFFFF)
    {
		errno = GetLastError();
		PyErr_SetFromErrnoWithFilenameObject(PyExc_OSError, Path);
        return NULL;
    }
    newattr = attr;
    if((Mode & 0222) == 0)
        newattr |= FILE_ATTRIBUTE_READONLY;
    else
        newattr &= ~FILE_ATTRIBUTE_READONLY;
    /* NOTE: setting a RAM file to read-only succeeds but has no effect */
    if(newattr != attr)
    {
        if(!SetFileAttributes(Wide_Path, newattr))
        {
            errno = GetLastError();
            PyErr_SetFromErrnoWithFilenameObject(PyExc_OSError, Path);
            return NULL;
        }
    }
	/*
	 *	Return success
	 */
	return(PyInt_FromLong(0));
}

/*
 *	OS-Dependent chdir() function
 */
static PyObject *OS_chdir(PyObject *Self, PyObject *Args)
{
	PyObject *Path;
	WCHAR Wide_Path[MAX_PATH + 1];

	/*
	 *	Parse the args
	 */
	if(!PyArg_ParseTuple(Args, "O:chdir", &Path)) return(NULL);
	if (!_WinCE_Absolute_Path(Path, Wide_Path, MAX_PATH + 1)) return(NULL);
	/*
	 *	Do the chdir
	 */
	if (_wchdir(Wide_Path) < 0) {
		PyErr_SetFromErrnoWithFilenameObject(PyExc_OSError, Path);
		return(NULL);
	}
	Py_INCREF(Py_None);
	return(Py_None);
}

#define F_OK 00
#define W_OK 02
#define R_OK 04

/*
 *	OS-Dependent access() function
 */
static PyObject *OS_access(PyObject *Self, PyObject *Args)
{
	PyObject *Path;
	int Mode;
	WCHAR Wide_Path[MAX_PATH + 1];
    DWORD attr;

	/*
	 *	Parse the args
	 */
	if(!PyArg_ParseTuple(Args, "Oi:access", &Path, &Mode)) return(NULL);
	if (!_WinCE_Absolute_Path(Path, Wide_Path, MAX_PATH + 1)) return(NULL);

	Py_BEGIN_ALLOW_THREADS
    attr = GetFileAttributes(Wide_Path);
	Py_END_ALLOW_THREADS
    if(attr == 0xFFFFFFFF)
        return PyBool_FromLong(0);
    if((Mode & W_OK) != 0 && (attr & FILE_ATTRIBUTE_READONLY) != 0)
        return PyBool_FromLong(0);  /* not writable */
    if((Mode & R_OK) != 0 && (attr & FILE_ATTRIBUTE_ROMMODULE) != 0)
        return PyBool_FromLong(0);  /* ROM modules are not readable */
    return PyBool_FromLong(1);  /* else it is readable and writable */
}

PyDoc_STRVAR(win32_startfile__doc__,
"startfile(filepath) - Start a file with its associated application.\n\
\n\
This acts like double-clicking the file in Explorer, or giving the file\n\
name as an argument to the DOS \"start\" command:  the file is opened\n\
with whatever application (if any) its extension is associated.\n\
\n\
startfile returns as soon as the associated application is launched.\n\
There is no option to wait for the application to close, and no way\n\
to retrieve the application's exit status.");

static PyObject *
win32_startfile(PyObject *self, PyObject *args)
{
    PyObject *Path;
    WCHAR wszPath[MAX_PATH + 1];
	BOOL rc;
    SHELLEXECUTEINFO sei;
	if (!PyArg_ParseTuple(args, "O", &Path))
        return NULL;
	if (!_WinCE_Absolute_Path(Path, wszPath, MAX_PATH + 1))
        return NULL;
    memset(&sei, 0, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_FLAG_NO_UI;
    sei.lpFile = wszPath;
    sei.nShow = SW_SHOWNORMAL;
	Py_BEGIN_ALLOW_THREADS
	rc = ShellExecuteEx(&sei);
	Py_END_ALLOW_THREADS
	if (!rc) {
		errno = GetLastError();
		PyErr_SetFromErrnoWithFilenameObject(PyExc_OSError, Path);
		return NULL;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

/*
 *	Exported Python methods
 */
static PyMethodDef OS_Methods[]=
{
	{"listdir", OS_listdir, METH_VARARGS},
	{"mkdir",   OS_mkdir,   METH_VARARGS},
	{"remove",  OS_unlink,  METH_VARARGS},
	{"rename",  OS_rename,  METH_VARARGS},
	{"rmdir",   OS_rmdir,   METH_VARARGS},
	{"stat",    OS_stat,    METH_VARARGS},
	{"lstat",   OS_stat,    METH_VARARGS},
	{"fstat",   OS_fstat,   METH_VARARGS},
	{"unlink",  OS_unlink,  METH_VARARGS},
	{"open",    OS_open,	METH_VARARGS},
	{"fdopen",  OS_fdopen,	METH_VARARGS},
	{"read",    OS_read,	METH_VARARGS},
	{"write",   OS_write,	METH_VARARGS},
	{"close",   OS_close,	METH_VARARGS},
	{"getcwd",  OS_getcwd,	METH_NOARGS},
	{"getcwdu", OS_getcwdu,	METH_NOARGS},
	{"chmod",   OS_chmod,	METH_VARARGS},
	{"chdir",   OS_chdir,	METH_VARARGS},
	{"access",  OS_access,	METH_VARARGS},
	{"startfile",	win32_startfile, METH_VARARGS, win32_startfile__doc__},
	{NULL, NULL}};

/*
 *	Initialize the "ce" module
 */
void initce(void)
{
	PyObject *Module, *Dictionary;
	int i;
	static struct {char *Name; int Value;} Constants[] = {
		{"O_RDONLY",	O_RDONLY},
		{"O_WRONLY",	O_WRONLY},
		{"O_RDWR",	O_RDWR},
		{"O_TRUNC",	O_TRUNC},
		{"O_EXCL",	O_EXCL},
		{"O_CREAT",	O_CREAT},
        {"F_OK", F_OK},
        {"R_OK", R_OK},
        {"W_OK", W_OK},
    };

	/*
	 *	Initialize the module
	 */
	Module = Py_InitModule(MODULE_NAME, OS_Methods);
	Dictionary = PyModule_GetDict(Module);
	/*
	 *	Export the "error" and "environ" variables
	 */
	PyDict_SetItemString(Dictionary, "error", PyExc_OSError);
	PyDict_SetItemString(Dictionary, "environ", PyDict_New());
	/*
	 *	Setup "stat" results
	 */
	Stat_Result_Descriptor.name = MODULE_NAME ".stat_result";
	Stat_Result_Descriptor.fields[7].name = PyStructSequence_UnnamedField;
	Stat_Result_Descriptor.fields[8].name = PyStructSequence_UnnamedField;
	Stat_Result_Descriptor.fields[9].name = PyStructSequence_UnnamedField;
	PyStructSequence_InitType(&Stat_Result_Type, &Stat_Result_Descriptor);
	New_Stat_Structure_Sequence = Stat_Result_Type.tp_new;
	Stat_Result_Type.tp_new = New_Stat_Result;
	Py_INCREF((PyObject*) &Stat_Result_Type);
	PyModule_AddObject(Module, "stat_result", (PyObject*) &Stat_Result_Type);
	/*
	 *	Setup constants
	 */
	for (i = 0; i < (sizeof(Constants)/sizeof(Constants[0])); i++)
		PyModule_AddIntConstant(Module, Constants[i].Name, Constants[i].Value);
	/*
	 *	Deal with errors
	 */
	if(PyErr_Occurred()) Py_FatalError("\""MODULE_NAME"\"" " module init failed");
}

