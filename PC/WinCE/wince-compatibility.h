/*
 *	Compatibility definitions for Windows/CE (makes it as much like Windows as possible)
 *
 *	David Kashtan, Validus Medical Systems
 */

#if 0
/*
 *	Use a local version of malloc/free/realloc
 *
 *	(Currently disabled)
 */
#define	malloc  WinCE_malloc
#define free    WinCE_free
#define	realloc WinCE_realloc
#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */
__declspec(dllexport) void *WinCE_malloc(int Size);
__declspec(dllexport) void WinCE_free(void *Ptr);
__declspec(dllexport) void *WinCE_realloc(void *Ptr, int Size);
#ifdef	__cplusplus
}
#endif	/* __cplusplus */
#endif	/* 0 */

/*
 *	The Windows/CE definition of fileno() is bad:
 *
 *	Re-define fileno so we get the integer that Python expects out of it
 */
#ifdef	fileno
#undef	fileno
#endif	/* fileno */
#define	fileno(f) (int)_fileno(f)

/*
 *	Windows/CE doesn't have rewind() so define it in terms of fseek()
 */
#define	rewind(f)	fseek(f,0,SEEK_SET)

/*
 *	Windows/CE <stdio.h> doesn't always define BUFSIZ
 */
#ifndef BUFSIZ
#define	BUFSIZ	4096
#endif	/* not BUFSIZ */

/*
 *	Windows/CE doesn't have a stat() function
 */
struct stat {
	unsigned long st_size;
	unsigned long st_ino;
	int st_mode;
	unsigned long st_atime;
	unsigned long st_mtime;
	unsigned long st_ctime;
	unsigned short st_dev;
	unsigned short st_nlink;
	unsigned short st_uid;
	unsigned short st_gid;
	};
#define S_IFMT   0170000
#define S_IFDIR  0040000
#define S_IFREG  0100000
#define S_IEXEC  0000100
#define S_IWRITE 0000200 
#define S_IREAD  0000400
extern __declspec(dllexport) int stat(const char *, struct stat *);
extern __declspec(dllexport) int _wstat(const WCHAR *, struct stat *);

/*
 *	Windows/CE doesn't have a chdir() function -- provide a wide string one
 */
extern __declspec(dllexport) int _wchdir(const WCHAR *);

/*
 *	Windows/CE doesn't define off_t
 */
typedef long off_t;

/*
 *	Windows/CE doesn't have _sys_nerr or _sys_errlist
 */
extern __declspec(dllexport) int _sys_nerr;
extern __declspec(dllexport) const char *_sys_errlist[];

/*
 *	Windows/CE only has the UNICODE version of OutputDebugString, so
 *	we re-define it in a way that calls our routine which converts
 *	to UNICODE and calls OutputDebugStringW
 */
#ifdef	OutputDebugString
#undef	OutputDebugString
#endif	/* Output_DebugString */
#define	OutputDebugString __WinCE_OutputDebugStringA
extern __declspec(dllexport) void __WinCE_OutputDebugStringA(const char *);

/*
 *	Define a function prototype for the missing unlink() function
 */
int __declspec(dllexport) unlink(const char *path);

/*
 *	Define a function prototype for our getcwd() function
 */
extern __declspec(dllexport) char *getcwd(char *, int);

/*
 *	Regardless of where fopen/fclose come from, we export them as public symbols from Python
 */
__declspec(dllexport) FILE *Py_fopen(const char *path, const char *mode);
__declspec(dllexport) int Py_fclose(FILE *f);

/*
 *	Use local versions of fopen, _wfopen, fseek, ftell, fread, fclose and fgetc
 */
#define fopen WinCE_fopen
extern __declspec(dllexport) FILE *WinCE_fopen(const char *, const char *);
#define _wfopen WinCE__wfopen
extern __declspec(dllexport) FILE *WinCE__wfopen(const WCHAR *, const WCHAR *);
#define fseek WinCE_fseek
extern __declspec(dllexport) int WinCE_fseek(FILE *, long, int);
#define ftell WinCE_ftell
extern __declspec(dllexport) long WinCE_ftell(FILE *);
#define fread WinCE_fread
extern __declspec(dllexport) size_t WinCE_fread(void *, size_t, size_t, FILE *);
#define fclose WinCE_fclose
extern __declspec(dllexport) int WinCE_fclose(FILE *);
#define fgetc WinCE_fgetc
extern __declspec(dllexport) int WinCE_fgetc(FILE *);

/*
 *	Absolute filename path handling
 */
void _WinCE_Absolute_Path_WCHAR(const WCHAR *Path, WCHAR *Buffer, int Buffer_Size);
void _WinCE_Absolute_Path_To_WCHAR(const char *Path, WCHAR *Buffer, int Buffer_Size);

/*
 *	These STDIO internal constants are used by fileobject
 */
#define	_IOFBF	0
#define	_IONBF	2

/*
 *	GetVersion() is only used in Objects/fileobject.c to determine
 *	if wide filenames can be used (enabled if return is < 0x80000000).
 *	(enabled)
 */
#define	GetVersion()		(0)

/*
 *	import_nt uses strnicmp -- but we will use strncmp and not get case-independence
 */
#define	strnicmp strncmp

/*
 *	We already pass a file handle into _get_osfhandle
 */
#define _get_osfhandle(Handle)		(Handle)

/*
 *	Dummy definitions for missing (but unnecessary) Windows/CE functions
 */
#define	GetConsoleCP()			""
#define	GetConsoleOutputCP()		""
#define	SetConsoleCtrlHandler(a,b)
#define	GetFullPathName(a,b,c,d)	(0)
#define setbuf(f, p)
#define	isatty(fd)			(0)
#define	signal(num, handler)		(0)
#define	getenv(cp)			(0)
#if _WIN32_WCE <= 300
#define	getservbyname(name, proto)	(0)
#define	getservbyport(port, proto)	(0)
#define	getprotobyname(name)		(0)
#endif	/* _WIN32_WCE <= 300 */

/*
 *	Define abort() to get rid of compiler warnings
 */
extern __declspec(dllexport) void abort(void);

/*
 *	Make the character classification function call different so we
 *	can compensate for Windows/CE misclassification of EOF
 */
extern __declspec(dllexport) int __WinCE_isctype(int, int);
#define	_isctype	__WinCE_isctype

/*
 *	ldexp() DOES NOT return HUGE_VAL on overflow.  It returns 1.#INF,
 *	so we need to define Py_HUGE_VAL in a way that allows us to correctly
 *	detect overflow.
 */
extern unsigned char _WinCE_Positive_Double_Infinity[];
#define	Py_HUGE_VAL (*(double *)_WinCE_Positive_Double_Infinity)

/*
 *	Define missing time functions
 */
#define	CLOCKS_PER_SEC	1000	/* Someday when clock() works we'll get back Milliseconds of CPU time */
typedef unsigned long clock_t;
extern __declspec(dllexport) clock_t clock(void);
extern __declspec(dllexport) struct tm *gmtime(const time_t *);
extern __declspec(dllexport) struct tm *localtime(const time_t *);
extern __declspec(dllexport) time_t time(time_t *);
extern __declspec(dllexport) char *asctime(const struct tm *);
extern __declspec(dllexport) char *ctime(const time_t *);
extern __declspec(dllexport) time_t mktime(struct tm *);
extern __declspec(dllexport) size_t strftime(char *, size_t, const char *, const struct tm *);

/*
 *	Define missing string functions
 */
extern __declspec(dllexport) char *strdup(const char *);
extern __declspec(dllexport) int strcoll(const char *, const char *);
extern __declspec(dllexport) size_t strxfrm(char *, char *, size_t);
#define _mbstrlen	strlen
#define	strtol		PyOS_strtol

/*
 *	Define missing Win32 registry functions
 */
#define	RegCreateKey(hKey, lpSubKey, phkResult) RegCreateKeyEx(hKey, lpSubKey, 0, NULL, 0, 0, NULL, phkResult, NULL)
#define	RegQueryValue(hKey, lpSubKey, lpValue, lpcbValue) RegQueryValueEx(hKey,lpSubKey, NULL, NULL, lpValue, lpcbValue)
#define	RegSetValue(hKey, lpSubKey, dwType, lpData, cbData) RegSetValueEx(hKey, lpSubKey, 0, dwType, lpData, cbData)
extern __declspec(dllexport) long RegEnumKey(HKEY hKey, DWORD dwIndex, LPTSTR lpName, DWORD cchName);

/*
 *	Define missing Win32 functions/constants used by PyWin32
 */
#define	WaitForInputIdle(hProcess, dwMilliseconds) WAIT_FAILED
#define	WAIT_IO_COMPLETION	(-1)
