/*
 vi:tabstop=8:shiftwidth=8:noexpandtab
 *	Compatibility functions for Windows/CE (makes it as much like Windows as possible)
 *
 *	David kashtan, Validus Medical Systems
 */
#include "Python.h"

#define ISSLASH(c)	((c) == TEXT('\\') || (c) == TEXT('/'))

#ifdef _M_ARM
/*
 *	ldexp() DOES NOT return HUGE_VAL on overflow.  It returns 1.#INF,
 *	which, on StrongARM are the following bytes:
 */
unsigned char _WinCE_Positive_Double_Infinity[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf0, 0x7f};
#else	/* not _M_ARM */
/*
 *	For other architectures this needs to be defined
 */
#error	Need to define the bytes for + infinity for a double on non-arm architectures
#endif	/* not _M_ARM */

/*
 *	PyErr_SetInterrupt does nothing
 */
__declspec(dllexport) void PyErr_SetInterrupt(void)
{
	return;
}

/*
 *	Our current working directory
 */
static WCHAR Default_Current_Directory[] = TEXT("\\Temp");
static WCHAR *Current_Directory = Default_Current_Directory;


/*
 *	WinCE has more restrictive path handling than Windows NT so we
 *	need to do some conversions for compatibility
 */
static void _WinCE_CanonicalizePath(WCHAR *pszPath)
{
	WCHAR *p = pszPath;

	/* Replace forward slashes with backslashes */
	while(*p) {
		if(*p == L'/')
			*p = L'\\';
		p++;
	}
	/* Strip \. and \.. from the beginning of absolute paths */
	p = pszPath;
	while(p[0] == '\\' && p[1] == L'.') {
		if(p[2] == '.') {
			/* Skip \.. */
			p += 3;
		} else {
			/* Skip \. */
			p += 2;
		}
	}
	if(!*p) {
		/* If we stripped everything then return the root \
		 * instead of an empty path
		 */
		wcscpy(pszPath, L"\\");
	} else if(p != pszPath) {
		/* We skipped something so we need to delete it */
		int size = (wcslen(p) + 1) * sizeof(WCHAR);
		memmove(pszPath, p, size);
	}
}

/*
 *	WCHAR version of _WinCE_Absolute_Path
 */
void _WinCE_Absolute_Path_WCHAR(const WCHAR *Path, WCHAR *Buffer, int Buffer_Size)
{
	int Path_Length, Directory_Length;
	WCHAR *cp;
	WCHAR *cp1;

	/*
	 *	Check for already being an absolute path
	 */
	if (ISSLASH(*Path)) {
		/*
		 *	Yes: Just return it
		 */
		Path_Length = wcslen(Path);
		if (Path_Length >= Buffer_Size) Path_Length = Buffer_Size - 1;
		memcpy(Buffer, Path, Path_Length * sizeof(WCHAR));
		Buffer[Path_Length] = '\0';
		_WinCE_CanonicalizePath(Buffer);
		return;
	}
	/*
	 *	Need to turn it into an absolute path: Peel off "." and ".."
	 */
	cp = (WCHAR *)Path;	
	/* cp1 points to the null terminator */
	cp1 = Current_Directory; while(*cp1) cp1++;
	while(*cp == TEXT('.')) {
		if (ISSLASH(cp[1])) {
			/* Strip ".\\" from beginning */
			cp += 2;
			continue;
		}
		if (cp[1] == TEXT('\0')) {
			/* "." is the same as no path */
			cp++;
			break;
		}
		/* Handle filenames like ".abc" */
		if (cp[1] != TEXT('.')) break;
		/* Handle filenames like "..abc" */
		if (!ISSLASH(cp[2]) && (cp[2] != TEXT('\0'))) break;
		/* Skip ".." */
		cp += 2;
		/* Skip backslash following ".." */
		if (*cp) cp++;
		/* Find the final backslash in the current directory path */
		while(cp1 > Current_Directory)
			if (ISSLASH(*--cp1)) break;
	}
	/*
	 *	Now look for device specifications (and get the length of the path
	 */
	Path = cp;
	while(*cp) {
		if (*cp++ == TEXT(':')) {
			/*
			 *	Device: Just return it
			 */
			Path_Length = wcslen(Path);
			if (Path_Length >= Buffer_Size) Path_Length = Buffer_Size - 1;
			memcpy(Buffer, Path, Path_Length * sizeof(WCHAR));
			Buffer[Path_Length] = '\0';
			_WinCE_CanonicalizePath(Buffer);
			return;
		}
	}
	Path_Length = cp - Path;
	/*
	 *	Trim off trailing "\\"
	 */
	if ((Path_Length > 0) && (ISSLASH(cp[-1]))) Path_Length--;
	/*
	 *	If we backed up past the root, we are at the root
	 */
	Directory_Length = cp1 - Current_Directory;
	if (Directory_Length == 0) Directory_Length = 1;
	/*
	 *	Output the directory and the path separator (truncated as necessary)
	 */
	Buffer_Size -= 2; /* Account for the null terminator and the path separator */
	if (Directory_Length >= Buffer_Size) Directory_Length = Buffer_Size;
	memcpy(Buffer, Current_Directory, Directory_Length * sizeof(WCHAR));
	Buffer[Directory_Length] = TEXT('\\');
	Buffer_Size -= Directory_Length;
	/*
	 *	If there is no path, make sure we remove the path separator
	 */
	if (Path_Length == 0) Directory_Length--;
	/*
	 *	Output the path (truncated as necessary)
	 */
	if (Path_Length >= Buffer_Size) Path_Length = Buffer_Size;
	memcpy(Buffer + Directory_Length + 1, Path, Path_Length * sizeof(WCHAR));
	Buffer[Directory_Length + 1 + Path_Length] = '\0';
	_WinCE_CanonicalizePath(Buffer);
	return;
}

/*
 *	Version of _WinCE_Absolute_Path that returns a wide result
 */
void _WinCE_Absolute_Path_To_WCHAR(const char *Path, WCHAR *Buffer, int Buffer_Size)
{
	WCHAR Wide_Path[MAX_PATH + 1];

	/*
	 *	Convert path to wide
	 */
	MultiByteToWideChar(CP_ACP, 0, Path, -1, Wide_Path, (sizeof(Wide_Path)/sizeof(Wide_Path[0])));
	/*
	 *	Get the absolute path
	 */
	_WinCE_Absolute_Path_WCHAR(Wide_Path, Buffer, Buffer_Size);
}

/*
 *	Version of _WinCE_Absolute_Path that returns a wide result from a Python string/unicode object
 */
int _WinCE_Absolute_Path(PyObject *Path, WCHAR *Buffer, int Buffer_Size)
{
	/*
	 *	Is the object a unicode string?
	 */
	if (PyUnicode_Check(Path)) {
		/*
		 *	Yes: Do the absolute path with wide input
		 */
		_WinCE_Absolute_Path_WCHAR((WCHAR *)PyUnicode_AS_UNICODE(Path), Buffer, Buffer_Size);
		return(1);
	}
	/*
	 *	Is the object a string?
	 */
	if (PyString_Check(Path)) {
		/*
		 *	Do the absolute path with wide input
		 */
		_WinCE_Absolute_Path_To_WCHAR(PyString_AS_STRING(Path), Buffer, Buffer_Size);
		return(1);
	}
	PyErr_SetString(PyExc_TypeError, "argument must be a string");
	return(0);
}

/*
 *	There is no concept of the current working directory on Windows/CE,
 *	so we simulate it
 */
__declspec(dllexport) char *getcwd(char *Buffer, int Buffer_Size)
{
	int n;

	/*
	 *	Convert to ascii
	 */
	n = WideCharToMultiByte(CP_ACP, 0, Current_Directory, -1, Buffer, Buffer_Size - 1, NULL, NULL);
	Buffer[n] = '\0';
	return(Buffer);
}

__declspec(dllexport) wchar_t *wgetcwd(wchar_t *Buffer, int Buffer_Size)
{
	int n = wcslen(Current_Directory);
	if(n >= Buffer_Size)
		n = Buffer_Size - 1;
	memcpy(Buffer, Current_Directory, n * sizeof(wchar_t));
	Buffer[n] = L'\0';
	return Buffer;
}

/*
 *	The missing "C" runtime abort() function
 */
__declspec(dllexport) void abort(void)
{
	ExitProcess(3);
}

/*
 *	The missing "C" runtime unlink() function
 */
__declspec(dllexport) int unlink(const char *Path)
{
	WCHAR Wide_Path[MAX_PATH + 1];

	/*
	 *	Delete the file
	 */
	_WinCE_Absolute_Path_To_WCHAR(Path, Wide_Path, MAX_PATH + 1);
	if (!DeleteFile(Wide_Path)) {
		errno = GetLastError();
		return(-1);
	}
	return(0);
}

/*
 *	Time conversion constants
 */
#define FT_EPOCH (116444736000000000i64)
#define	FT_TICKS (10000000i64)

/*
 *	Wide version of the missing "C" runtime stat() function
 */
int _wstat(const WCHAR *Path, struct stat *st)
{
	WIN32_FIND_DATA Data;
	HANDLE hFile;
	WCHAR *p;
	unsigned dmode;
	WCHAR Absolute_Path[MAX_PATH + 1];
#define A_RO	0x1
#define A_D	0x10

	/*
	 *	Get the absolute path
	 */
	_WinCE_Absolute_Path_WCHAR(Path, Absolute_Path, MAX_PATH + 1);
	/*
	 *	Check for stuff we can't deal with
	 */
	p = Absolute_Path;
	while(*p) {
		if ((*p == TEXT('?')) || (*p == TEXT('*'))) return(-1);
		p++;
	}
	hFile = FindFirstFile(Absolute_Path, &Data);
	if (hFile == INVALID_HANDLE_VALUE) {
		errno = GetLastError();
		return(-1);
	}
	FindClose(hFile);
	/*
	 *	Found: Convert the file times
	 */
	st->st_mtime=(time_t)((*(__int64*)&Data.ftLastWriteTime-FT_EPOCH)/FT_TICKS);
	if(Data.ftLastAccessTime.dwLowDateTime || Data.ftLastAccessTime.dwHighDateTime)
		st->st_atime=(time_t)((*(__int64*)&Data.ftLastAccessTime-FT_EPOCH)/FT_TICKS);
	else
		st->st_atime=st->st_mtime ;
	if(Data.ftCreationTime.dwLowDateTime || Data.ftCreationTime.dwHighDateTime )
		st->st_ctime=(time_t)((*(__int64*)&Data.ftCreationTime-FT_EPOCH)/FT_TICKS);
	else
		st->st_ctime=st->st_mtime ;
	/*
	 *	Convert the file modes
	 */
	dmode = Data.dwFileAttributes & 0xff;
	if (Absolute_Path[1] == TEXT(':')) p += 2;
	st->st_mode = (unsigned short)(((ISSLASH(*p) && !Absolute_Path[1]) || (dmode & A_D) || *p) ? S_IFDIR|S_IEXEC : S_IFREG);
	st->st_mode |= (dmode & A_RO) ? S_IREAD : (S_IREAD|S_IWRITE);
	p = Absolute_Path + wcslen(Absolute_Path);
	while(p >= Absolute_Path) {
		if (*--p == TEXT('.')) {
			if(p[1] && ((p[1] == TEXT('e')) || (p[1] == TEXT('c')) || (p[1] == TEXT('b'))) &&
			   p[2] && ((p[2] == TEXT('x')) || (p[2] == TEXT('m')) || (p[2] == TEXT('a')) || (p[2] == TEXT('o'))) &&
			   p[3] && ((p[3] == TEXT('e')) || (p[3] == TEXT('d')) || (p[3] == TEXT('t')) || (p[3] == TEXT('m'))))
				st->st_mode |= S_IEXEC;
			break;
		}
	}
	st->st_mode |= (st->st_mode & 0700) >> 3;
	st->st_mode |= (st->st_mode & 0700) >> 6;
	/*
	 *	Set the other information
	 */
	st->st_nlink=1;
	st->st_size=(unsigned long int)Data.nFileSizeLow;
	st->st_uid=0;
	st->st_gid=0;
	st->st_ino=0 /*Data.dwOID ?*/;
	st->st_dev=0;
	/*
	 *	Return success
	 */
	return(0);
}

/*
 *	The missing "C" runtime stat() function
 */
static int Check_For_ZIP_Resource(const char *Path, HRSRC *ResourceP);
__declspec(dllexport) int stat(const char *Path, struct stat *st)
{
	WCHAR Wide_Path[MAX_PATH+1];

	/*
	 *	Check for ".zip" files
	 */
	if (Check_For_ZIP_Resource(Path, 0)) {
		memset(st, 0, sizeof(*st));
		st->st_mode = S_IFREG;
		return(0);
	}
	/*
	 *	Turn it into a wide string and do the _wstat
	 */
	MultiByteToWideChar(CP_ACP, 0, Path, -1, Wide_Path, (sizeof(Wide_Path)/sizeof(Wide_Path[0])));
	return(_wstat(Wide_Path, st));
}

/*
 *	Helper function for cemodule -- do an fstat() operation on a Win32 File Handle
 */
int WinCE_Handle_fstat(HANDLE Handle, struct stat *st)
{
	unsigned dmode;
	BY_HANDLE_FILE_INFORMATION Data;

	/*
	 *	Get the file information
	 */
	if (!GetFileInformationByHandle(Handle, &Data)) {
		/*
		 *	Return error
		 */
		errno = GetLastError();
		return(-1);
	}
	/*
	 *	Found: Convert the file times
	 */
	st->st_mtime=(time_t)((*(__int64*)&Data.ftLastWriteTime-FT_EPOCH)/FT_TICKS);
	if(Data.ftLastAccessTime.dwLowDateTime || Data.ftLastAccessTime.dwHighDateTime)
		st->st_atime=(time_t)((*(__int64*)&Data.ftLastAccessTime-FT_EPOCH)/FT_TICKS);
	else
		st->st_atime=st->st_mtime ;
	if(Data.ftCreationTime.dwLowDateTime || Data.ftCreationTime.dwHighDateTime )
		st->st_ctime=(time_t)((*(__int64*)&Data.ftCreationTime-FT_EPOCH)/FT_TICKS);
	else
		st->st_ctime=st->st_mtime ;
	/*
	 *	Convert the file modes
	 */
	dmode = Data.dwFileAttributes & 0xff;
	st->st_mode = (unsigned short)((dmode & A_D) ? S_IFDIR|S_IEXEC : S_IFREG);
	st->st_mode |= (dmode & A_RO) ? S_IREAD : (S_IREAD|S_IWRITE);
	st->st_mode |= (st->st_mode & 0700) >> 3;
	st->st_mode |= (st->st_mode & 0700) >> 6;
	/*
	 *	Set the other information
	 */
	st->st_nlink=1;
	st->st_size=(unsigned long int)Data.nFileSizeLow;
	st->st_uid=0;
	st->st_gid=0;
	st->st_ino=0;
	st->st_dev=0;
	/*
	 *	Return success
	 */
	return(0);
}

/*
 *	Wide version of the missing "C" runtime chdir() function
 */
__declspec(dllexport) int _wchdir(const WCHAR *Path)
{
	int Length;
	WCHAR *cp;
	struct stat st;
	WCHAR Absolute_Path[MAX_PATH + 1];

	/*
	 *	Make sure it exists
	 */
	if (_wstat(Path, &st) < 0) return(-1);
	if (!(st.st_mode & S_IFDIR)) {
		errno = EINVAL;
		return(-1);
	}
	/*
	 *	Get the absolute path
	 */
	_WinCE_Absolute_Path_WCHAR(Path, Absolute_Path, MAX_PATH + 1);
	/*
	 *	Put it into dynamic memorhy
	 */
	Length = wcslen(Absolute_Path);
	cp = (WCHAR *)LocalAlloc(0, (Length + 1) * sizeof(WCHAR));
	if (!cp) return(-1);
	memcpy(cp, Absolute_Path, Length * sizeof(WCHAR));
	cp[Length] = TEXT('\0');
	/*
	 *	Free up any old allocation and store the new current directory
	 */
	if (Current_Directory != Default_Current_Directory) LocalFree(Current_Directory);
	Current_Directory = cp;
	return(0);
}

/*
 *	The missing "C" runtime clock() function
 */
__declspec(dllexport) clock_t clock(void)
{
	/*
	 *	Return that the CPU time is unavailable
	 */
	return((clock_t)-1);
}

/*
 *	Convert a time_t to a FILETIME
 */
static void Convert_time_t_To_FILETIME(time_t Time, FILETIME *File_Time)
{
	__int64 Temp;

	/*
	 *	Use 64-bit calculation to convert seconds since 1970 to
	 *	100nSecs since 1601
	 */
	Temp = ((__int64)Time * FT_TICKS) + FT_EPOCH;
	/*
	 *	Put it into the FILETIME structure
	 */
	File_Time->dwLowDateTime = (DWORD)Temp;
	File_Time->dwHighDateTime = (DWORD)(Temp >> 32);
}

/*
 *	Convert a FILETIME to a time_t
 */
static time_t Convert_FILETIME_To_time_t(FILETIME *File_Time)
{
	__int64 Temp;

	/*
	 *	Convert the FILETIME structure to 100nSecs since 1601 (as a 64-bit value)
	 */
	Temp = (((__int64)File_Time->dwHighDateTime) << 32) + (__int64)File_Time->dwLowDateTime;
	/*
	 *	Convert to seconds from 1970
	 */
	return((time_t)((Temp - FT_EPOCH) / FT_TICKS));
}

/*
 *	Convert a FILETIME to a tm structure
 */
static struct tm *Convert_FILETIME_To_tm(FILETIME *File_Time)
{
	SYSTEMTIME System_Time;
	static struct tm tm = {0};
	static const short Day_Of_Year_By_Month[12] = {(short)(0),
						       (short)(31),
						       (short)(31 + 28),
						       (short)(31 + 28 + 31),
						       (short)(31 + 28 + 31 + 30),
						       (short)(31 + 28 + 31 + 30 + 31),
						       (short)(31 + 28 + 31 + 30 + 31 + 30),
						       (short)(31 + 28 + 31 + 30 + 31 + 30 + 31),
						       (short)(31 + 28 + 31 + 30 + 31 + 30 + 31 + 31),
						       (short)(31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
						       (short)(31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
						       (short)(31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30)};


	/*
	 *	Turn the FILETIME into a SYSTEMTIME
	 */
	FileTimeToSystemTime(File_Time, &System_Time);
	/*
	 *	Use SYSTEMTIME to fill in the tm structure
	 */
	tm.tm_sec = System_Time.wSecond;
	tm.tm_min = System_Time.wMinute;
	tm.tm_hour = System_Time.wHour;
	tm.tm_mday = System_Time.wDay;
	tm.tm_mon = System_Time.wMonth - 1;
	tm.tm_year = System_Time.wYear - 1900;
	tm.tm_wday = System_Time.wDayOfWeek;
	tm.tm_yday = Day_Of_Year_By_Month[tm.tm_mon] + tm.tm_mday - 1;
	if (tm.tm_mon >= 2) {
		/*
		 *	Check for leap year (every 4 years but not every 100 years but every 400 years)
		 */
		if ((System_Time.wYear % 4) == 0) {
			/*
			 *	It Is a 4th year
			 */
			if ((System_Time.wYear % 100) == 0) {
				/*
				 *	It is a 100th year
				 */
				if ((System_Time.wYear % 400) == 0) {
					/*
					 *	It is a 400th year: It is a leap year
					 */
					tm.tm_yday++;
				}
			} else {
				/*
				 *	It is not a 100th year: It is a leap year
				 */
				tm.tm_yday++;
			}
		}
	}
	return(&tm);
}

/*
 *	Convert a tm structure to a FILETIME
 */
static FILETIME *Convert_tm_To_FILETIME(struct tm *tm)
{
	SYSTEMTIME System_Time;
	static FILETIME File_Time = {0};

	/*
	 *	Use the tm structure to fill in a SYSTEM
	 */
	System_Time.wYear = tm->tm_year + 1900;
	System_Time.wMonth = tm->tm_mon + 1;
	System_Time.wDayOfWeek = tm->tm_wday;
	System_Time.wDay = tm->tm_mday;
	System_Time.wHour = tm->tm_hour;
	System_Time.wMinute = tm->tm_min;
	System_Time.wSecond = tm->tm_sec;
	System_Time.wMilliseconds = 0;
	/*
	 *	Convert it to a FILETIME and return it
	 */
	SystemTimeToFileTime(&System_Time, &File_Time);
	return(&File_Time);
}

/*
 *	The missing "C" runtime gmtime() function
 */
__declspec(dllexport) struct tm *gmtime(const time_t *TimeP)
{
	FILETIME File_Time;

	/*
	 *	Deal with null time pointer
	 */
	if (!TimeP) return(NULL);
	/*
	 *	time_t -> FILETIME -> tm
	 */
	Convert_time_t_To_FILETIME(*TimeP, &File_Time);
	return(Convert_FILETIME_To_tm(&File_Time));
}

/*
 *	The missing "C" runtime localtime() function
 */
__declspec(dllexport) struct tm *localtime(const time_t *TimeP)
{
	FILETIME File_Time, Local_File_Time;

	/*
	 *	Deal with null time pointer
	 */
	if (!TimeP) return(NULL);
	/*
	 *	time_t -> FILETIME -> Local FILETIME -> tm
	 */
	Convert_time_t_To_FILETIME(*TimeP, &File_Time);
	FileTimeToLocalFileTime(&File_Time, &Local_File_Time);
	return(Convert_FILETIME_To_tm(&Local_File_Time));
}

/*
 *	The missing "C" runtime asctime() function
 */
__declspec(dllexport) char *asctime(const struct tm *tm)
{
	TCHAR Temp[26];
	static const CHAR *Days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	static const CHAR *Months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	static char Buffer[26];

	/*
	 *	Format the string
	 */
	wsprintf(Temp, TEXT("%hs %hs %2d %02d:%02d:%02d %4d\n"),
		 Days[tm->tm_wday],
		 Months[tm->tm_mon],
		 tm->tm_mday,
		 tm->tm_hour,
		 tm->tm_min,
		 tm->tm_sec,
		 tm->tm_year + 1900);
	/*
	 *	Convert to ascii and return it
	 */
	WideCharToMultiByte(CP_ACP, 0, Temp, sizeof(Temp)/sizeof(Temp[0]), Buffer, sizeof(Buffer)/sizeof(Buffer[0]), NULL, NULL);
	return(Buffer);
}

/*
 *	The missing "C" runtime ctime() function
 */
__declspec(dllexport) char *ctime(const time_t *TimeP)
{
	FILETIME File_Time;

	/*
	 *	time_t -> FILETIME -> Local FILETIME -> tm then we can use asctime()
	 */
	Convert_time_t_To_FILETIME(*TimeP, &File_Time);
	FileTimeToLocalFileTime(&File_Time, &File_Time);
	return(asctime(Convert_FILETIME_To_tm(&File_Time)));
}

/*
 *	The missing "C" runtime mktime() function
 */
__declspec(dllexport) time_t mktime(struct tm *tm)
{
	FILETIME *Local_File_Time;
	FILETIME File_Time;

	/*
	 *	tm -> Local FILETIME -> FILETIME -> time_t
	 */
	Local_File_Time = Convert_tm_To_FILETIME(tm);
	LocalFileTimeToFileTime(Local_File_Time, &File_Time);
	return(Convert_FILETIME_To_time_t(&File_Time));
}

/*
 *	Missing "C" runtime time() function
 */
__declspec(dllexport) time_t time(time_t *TimeP)
{
	SYSTEMTIME System_Time;
	FILETIME File_Time;
	time_t Result;

	/*
	 *	Get the current system time
	 */
	GetSystemTime(&System_Time);
	/*
	 *	SYSTEMTIME -> FILETIME -> time_t
	 */
	SystemTimeToFileTime(&System_Time, &File_Time);
	Result = Convert_FILETIME_To_time_t(&File_Time);
	/*
	 *	Return the time_t
	 */
	if (TimeP) *TimeP = Result;
	return(Result);
}

/*
 *	Missing "C" runtime tzset() function
 */
static char Standard_Name[32] = "GMT";
static char Daylight_Name[32] = "GMT";
__declspec(dllexport) char *tzname[2] = {Standard_Name, Daylight_Name};
__declspec(dllexport) long timezone = 0;
__declspec(dllexport) int daylight = 0;
__declspec(dllexport) void tzset(void)
{
	TIME_ZONE_INFORMATION Info;
	int Result;

	/*
	 *	Get our current timezone information
	 */
	Result = GetTimeZoneInformation(&Info);
	switch(Result) {
	  /*
	   *	We are on standard time
	   */
	  case TIME_ZONE_ID_STANDARD:
		  daylight = 0;
		  break;
	  /*
	   *	We are on daylight savings time
	   */
	  case TIME_ZONE_ID_DAYLIGHT:
		  daylight = 1;
		  break;
	  /*
	   *	We don't know the timezone information (leave it GMT)
	   */
	  default: return;
	}
	/*
	 *	Extract the timezone information
	 */
	timezone = Info.Bias * 60;
	if (Info.StandardName[0])
		WideCharToMultiByte(CP_ACP, 0, Info.StandardName, -1, Standard_Name, sizeof(Standard_Name) - 1, NULL, NULL);
	if (Info.DaylightName[0])
		WideCharToMultiByte(CP_ACP, 0, Info.DaylightName, -1, Daylight_Name, sizeof(Daylight_Name) - 1, NULL, NULL);
}

/*
 *	An ASCII version of DebugOutputString
 */
__declspec(dllexport) void __WinCE_OutputDebugStringA(const char *String)
{
	TCHAR Local_String[256];
	int n;

	/*
	 *	Convert to wide string
	 */
	n = MultiByteToWideChar(CP_ACP, 0, String, -1, Local_String, sizeof(Local_String)/sizeof(TCHAR));
	Local_String[n] = _T('\0');
	/*
	 *	Now we can do the OutputDebugStringW
	 */
	OutputDebugStringW(Local_String);
}

/*
 *	Get a file's last modification time
 */
__declspec(dllexport) long PyOS_GetLastModificationTime(char *Path, FILE *fp)
{
	WCHAR Wide_Path[MAX_PATH + 1];
	FILETIME t;
	HANDLE   h;

	_WinCE_Absolute_Path_To_WCHAR(Path, Wide_Path, MAX_PATH + 1);
	h = CreateFile(Wide_Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (h == INVALID_HANDLE_VALUE) return(-1);
	GetFileTime(h, NULL, NULL, (LPFILETIME)&t);
	CloseHandle(h);
	return((long)Convert_FILETIME_To_time_t(&t));
}

/*
 *	On Windows/CE we have a routine to declare a ^C interrupt
 */
static int Interrupted = 0;
__declspec(dllexport) void PyOS_InitInterrupts(void){}
__declspec(dllexport) void PyOS_FiniInterrupts(void){}
__declspec(dllexport) int PyOS_InterruptOccurred(void)
{
	int Return_Value = Interrupted;
	Interrupted = 0;
	return(Return_Value);
}
static int _Do_Py_CheckSignals(void *Arg) {return(PyErr_CheckSignals());}
__declspec(dllexport) void _WinCE_Declare_Interrupt(void)
{
	Interrupted = 1;
	Py_AddPendingCall(_Do_Py_CheckSignals, NULL);
}

/*
 *	Regardless of where fopen/fclose come from, we export them as public symbols from Python
 */
__declspec(dllexport) FILE *Py_fopen(const char *Path, const char *Mode)
{
	return(fopen(Path , Mode));
}

__declspec(dllexport) int Py_fclose(FILE *f)
{
	return(fclose(f));
}

/*
 *	Function to fix Windows/CE broken character classification
 */
__declspec(dllexport) int _isctype(int Character, int Classification)
{
#undef	_isctype

	/*
	 *	Check for EOF
	 */
	if (Character == EOF) return(0);
	/*
	 *	Not EOF: return the classification
	 */
	return(_isctype(Character, Classification));
}

/*
 *	Check a path to see if it is a PYTHONZIP resource
 *
 *	If ResourceP is not NULL return the resource handle in it
 */
static int Check_For_ZIP_Resource(const char *Path, HRSRC *ResourceP)
{
	const char *cp;
	int Length;
	TCHAR *cp1;
	TCHAR Local_Resource_Name[64];
	TCHAR *Resource_Name = Local_Resource_Name;
	HRSRC Resource;
	extern HMODULE PyWin_DLLhModule;

	/*
	 *	If the path does not end in ".zip", it is not a PYTHONZIP resource
	 *	(While we are at it, we get rid of the directory name)
	 */
	cp = Path;
	while(*cp) {
		if (ISSLASH(*cp)) Path = cp + 1;
		cp++;
	}
	Length = cp - Path;
	if (Length < 4) return(0);
	if (((cp[-1] != 'p') && (cp[-1] != 'P')) ||
	    ((cp[-2] != 'i') && (cp[-2] != 'I')) ||
	    ((cp[-3] != 'z') && (cp[-3] != 'Z')) ||
	    (cp[-4] != '.'))
		return(0);
	/*
	 *	Make an uppercase TCHAR of the filename, which becomes the resource name
	 */
	if (Length >= (sizeof(Local_Resource_Name)/sizeof(Local_Resource_Name[0]))) {
		Resource_Name = (TCHAR *)LocalAlloc(0, (Length + 1) * sizeof(TCHAR));
		if (!Resource_Name)
			Length = (sizeof(Local_Resource_Name)/sizeof(Local_Resource_Name[0])) - 1;
	}
	cp1 = Resource_Name + Length + 1;
	while(cp >= Path) {
		*--cp1 = ((*cp >= 'a') && (*cp <= 'z')) ? (TCHAR)(*cp - ('a' - 'A')) : (TCHAR)*cp;
		cp--;
	}
	/*
	 *	Locate the resource (and free up the resource name)
	 */
	Resource = FindResource(GetModuleHandle(NULL), cp1, TEXT("PYTHONZIP"));
	if (!Resource && PyWin_DLLhModule)
		Resource = FindResource(PyWin_DLLhModule, cp1, TEXT("PYTHONZIP"));
	if (Resource_Name != Local_Resource_Name) LocalFree(Resource_Name);
	/*
	 *	See if anybody wants the resource handle
	 */
	if (ResourceP) {
		/*
		 *	Yes: Return it
		 */
		*ResourceP = Resource;
	} else {
		/*
		 *	No: Clean up
		 */
		CloseHandle(Resource);
	}
	/*
	 *	Done
	 */
	return(Resource ? 1 : 0);
}

/*
 *	Structure to hold an open PYTHONZIP resource so we can make it look like a file
 */
struct _ZIP_Resource_File {
	struct _ZIP_Resource_File *Next;
	unsigned char *Data;
	int Size;
	int Position;
	HRSRC Resource;
	HGLOBAL Resource_Data;
	};
static struct _ZIP_Resource_File *_ZIP_Resource_Files;

/*
 *	Check an fopen()/_wfopen() mode string for validity
 */
static int Check_fopen_Mode(const WCHAR *Mode)
{
	/*
	 *	Check each character in the mode string against the allowed characters
	 */
	while(*Mode) {
		switch(*Mode++) {
		  /*
		   *	These are the valid mode characters
		   */
		  case TEXT('r'):
		  case TEXT('w'):
		  case TEXT('a'):
		  case TEXT('+'):
		  case TEXT('t'):
		  case TEXT('b'):
		  case TEXT('c'):
		  case TEXT('n'):
			  break;
		  /*
		   *	Everything else is invalid
		   */
		  default: return(0);
		}
	}
	return(1);
}

/*
 *	Local version of fopen() that knows how to open PYTHONZIP resources
 */
__declspec(dllexport) FILE *WinCE_fopen(const char *Filename, const char *Mode)
{
	FILE *fp;
	HRSRC Resource;
	WCHAR Temp[MAX_PATH + 1];
	WCHAR Wide_Mode[8];
	
	/*
	 *	Convert mode to wide string and check mode validity
	 */
	MultiByteToWideChar(CP_ACP, 0, Mode, -1, Wide_Mode, (sizeof(Wide_Mode)/sizeof(Wide_Mode[0])));
	if (!Check_fopen_Mode(Wide_Mode)) {
		/*
		 *	Return an error that will provoke an "invalid mode" error
		 */
		errno = 0;
		return(0);
	}
	/*
	 *	Try to do the normal fopen()
	 */
	_WinCE_Absolute_Path_To_WCHAR(Filename, Temp, (sizeof(Temp)/sizeof(Temp[0])));
#undef _wfopen
	fp = _wfopen(Temp, Wide_Mode);
	if (fp) return(fp);
	/*
	 *	Not found: Check for .zip files and get the resource
	 */
	if (Check_For_ZIP_Resource(Filename, &Resource)) {
		HGLOBAL Resource_Data;
		DWORD Size;

		/*
		 *	Get its size
		 */
		Size = SizeofResource(NULL, Resource);
		if (Size != 0) {
			/*
			 *	Load it into memory
			 */
			Resource_Data = LoadResource(NULL, Resource);
			if (Resource_Data)  {
				unsigned char *Data;

				/*
				 *	Lock it so we can get a pointer to the data
				 */
				Data = (unsigned char *)LockResource(Resource_Data);
				if (Data) {
					struct _ZIP_Resource_File *p;

					/*
					 *	Create an open ZIP Resource file
					 */
					p = (struct _ZIP_Resource_File *)LocalAlloc(0, sizeof(*p));
					if (p) {
						memset(p, 0, sizeof(*p));
						p->Data = Data;
						p->Size = Size;
						p->Resource_Data = Resource_Data;
						p->Resource = Resource;
						p->Next = _ZIP_Resource_Files;
						_ZIP_Resource_Files = p;
						return((FILE *)p);
					}
				}
				/*
				 *	Couldn't lock the data, so clean up
				 */
				CloseHandle(Resource_Data);
			}
		}
		/*
		 *	At this point we did not succeed in "opening" the zip file resource, so clean up
		 */
		CloseHandle(Resource);
	}
	/*
	 *	Resource not found
	 */
	errno = ENOENT;
	return(0);
}

/*
 *	Local version of _wfopen() that deals with relative paths
 */
__declspec(dllexport) FILE *WinCE__wfopen(const WCHAR *Filename, const WCHAR *Mode)
{
	FILE *fp;
	WCHAR Temp[MAX_PATH + 1];

	/*
	 *	Check mode validity
	 */
	if (!Check_fopen_Mode(Mode)) {
		/*
		 *	Return an error that will provoke an "invalid mode" error
		 */
		errno = 0;
		return(0);
	}
	/*
	 *	Call _wfopen with the absolute path
	 */
	_WinCE_Absolute_Path_WCHAR(Filename, Temp, sizeof(Temp)/sizeof(Temp[0]));
	fp = _wfopen(Temp, Mode);
	if (!fp) errno = ENOENT;
	return(fp);
}

/*
 *	Local version of fseek() that knows about PYTHONZIP resources
 */
__declspec(dllexport) int WinCE_fseek(FILE *fp, long Position, int Whence)
{
	struct _ZIP_Resource_File *p;

	/*
	 *	See if this is a PYTHONZIP resource
	 */
	for (p = _ZIP_Resource_Files; p; p = p->Next) {
		if (p == (struct _ZIP_Resource_File *)fp) {
			/*
			 *	Yes: Just update its position (make sure the new position is legal)
			 */
			if (Whence == SEEK_END) Position += p->Size;
			if ((Position < 0) || (Position > p->Size)) return(1);
			p->Position = Position;
			return(0);
		}
	}
	/*
	 *	No: Do the normal fseek()
	 */
#undef fseek
	return(fseek(fp, Position, Whence));
}

/*
 *	Local version of ftell() that knows about PYTHONZIP resources
 */
__declspec(dllexport) long WinCE_ftell(FILE *fp)
{
	struct _ZIP_Resource_File *p;

	/*
	 *	See if this is a PYTHONZIP resource
	 */
	for (p = _ZIP_Resource_Files; p; p = p->Next) {
		if (p == (struct _ZIP_Resource_File *)fp) {
			/*
			 *	Yes: Return its current position
			 */
			return(p->Position);
		}
	}
	/*
	 *	No: Do the normal ftell()
	 */
#undef	ftell
	return(ftell(fp));
}

/*
 *	Local version of fread() that knows about PYTHONZIP resources
 */
__declspec(dllexport) size_t WinCE_fread(void *Pointer, size_t Size, size_t NMembers, FILE *fp)
{
	struct _ZIP_Resource_File *p;

	/*
	 *	See if this is a PYTHONZIP resource
	 */
	for (p = _ZIP_Resource_Files; p; p = p->Next) {
		if (p == (struct _ZIP_Resource_File *)fp) {
			int Remaining, i;

			/*
			 *	Yes: Calculate the read size (limited by the PYTHONZIP resource size)
			 */
			Remaining = p->Size - p->Position;
			i = Size * NMembers;
			if (i > Remaining) i = Remaining;
			/*
			 *	Copy the data and update the current position
			 */
			if (i > 0) memcpy(Pointer, p->Data + p->Position, i);
			p->Position += i;
			i /= Size;
			if (i == 0) i = EOF; /* If nothing read, return EOF */

			return(i);
		}
	}
	/*
	 *	No: Do the normal fread()
	 */
#undef	fread
	return(fread(Pointer, Size, NMembers, fp));
}

/*
 *	Local version of fgetc() that knows about PYTHONZIP resources
 */
__declspec(dllexport) int WinCE_fgetc(FILE *fp)
{
	struct _ZIP_Resource_File *p;

	/*
	 *	See if this is a PYTHONZIP resource
	 */
	for (p = _ZIP_Resource_Files; p; p = p->Next) {
		if (p == (struct _ZIP_Resource_File *)fp) {
			/*
			 *	Yes: Check for EOF
			 */
			if (p->Position >= p->Size) return(EOF);
			/*
			 *	Return the next character
			 */
			return(p->Data[p->Position++]);
		}
	}
	/*
	 *	No: Do the normal fgetc()
	 */
#undef	fgetc
	return(fgetc(fp));
}

/*
 *	Local version of fclose() that knows about PYTHONZIP resources
 */
__declspec(dllexport) int WinCE_fclose(FILE *fp)
{
	struct _ZIP_Resource_File *p, *q;

	/*
	 *	See if this is a PYTHONZIP resource
	 */
	q = 0;
	for (p = _ZIP_Resource_Files; p; p = p->Next) {
		if (p == (struct _ZIP_Resource_File *)fp) {
			/*
			 *	Yes: Close the resource handle to release its memory
			 */
			CloseHandle(p->Resource_Data);
			CloseHandle(p->Resource);
			/*
			 *	Remove it from the list of open PYTHONZIP resources and free it
			 */
			if (q)
				q->Next = p->Next;
			else
				_ZIP_Resource_Files = p->Next;
			LocalFree(p);
			return(0);
		}
		q = p;
	}
	/*
	 *	No: Do the normal fclose()
	 */
#undef	fclose
	return(fclose(fp));
}

#if 0	/* using _strdup() instead now */
/*
 *	Missing strdup() function
 */
__declspec(dllexport) char *strdup(const char *String)
{
	const char *cp;
	char *New_String;

	/*
	 *	Get the string length
	 */
	cp = String;
	while(*cp++);
	/*
	 *	Allocate storage for it and copy it
	 */
	New_String = (char *)malloc(cp - String);
	if (New_String) memcpy(New_String, String, cp - String);
	return(New_String);
}
#endif

/*
 *	Missing strcoll() funciton
 */
__declspec(dllexport) int strcoll(const char *s1, const char *s2)
{
	/*
	 *	For now just use an ascii comparison (later on try for unicode support)
	 */
	return(strcmp(s1, s2));
}

/*
 *	Missing strxfrm() function
 */
__declspec(dllexport) size_t strxfrm(char *dest, char *src, size_t n)
{
	char *cp;

	/*
	 *	For now just copy the string
	 */
	cp = src;
	while(*cp++);
	if (n < (size_t)(cp - src)) n = (cp - src);
	if (n > 0) memcpy(dest, src, n);
	return(n);
}

#if 0
/*
 *	Pretending that we support locales is futile because the C library doesn't.
 *	For example if we call strtod("2,2") in the Italian locale then it will fail,
 *	though it should succeed (and it does work on the PC).
 */

/*
 *	Get Locale information
 */
#include <locale.h>
static char *Locale_Language = 0;
static struct lconv Locale_Conversion = {0};
static void Get_Locale_Information(void)
{
	/*
	 *	Make sure the conversion structure is initialized to something
	 */
	if (!Locale_Conversion.decimal_point) {
		static char None[] = "", Comma[] = ",", Dot[] = ".";
		static char Default_Grouping[3] = {3, 3, 0};
		static char Negative_Sign = 1;
		static struct _GetLocaleInfo_Description {
			LCTYPE GetLocaleInfo_Type;
			unsigned char lconv_Type;
#define	TYPE_STRING	0
#define	TYPE_INTEGER	1
#define	TYPE_GROUPING	2
			void *Field;
		} Convert[] = {
			{LOCALE_SLANGUAGE,	TYPE_STRING,	&Locale_Language},
			{LOCALE_SDECIMAL,	TYPE_STRING,	&Locale_Conversion.decimal_point},
			{LOCALE_STHOUSAND,	TYPE_STRING,	&Locale_Conversion.thousands_sep},
			{LOCALE_SGROUPING,	TYPE_GROUPING,	&Locale_Conversion.grouping},
			{LOCALE_IDIGITS,	TYPE_INTEGER,	&Locale_Conversion.frac_digits},
			{LOCALE_INEGNUMBER,	TYPE_INTEGER,	&Negative_Sign},
			{LOCALE_SCURRENCY,	TYPE_STRING,	&Locale_Conversion.currency_symbol},
			{LOCALE_SINTLSYMBOL,	TYPE_STRING,	&Locale_Conversion.int_curr_symbol},
			{LOCALE_SMONDECIMALSEP,	TYPE_STRING,	&Locale_Conversion.mon_decimal_point},
			{LOCALE_SMONTHOUSANDSEP,TYPE_STRING,	&Locale_Conversion.mon_thousands_sep},
			{LOCALE_SMONGROUPING,	TYPE_GROUPING,	&Locale_Conversion.mon_grouping},
			{0,			0,		0}};
		struct _GetLocaleInfo_Description *p;


		/*
		 *	Set default values
		 */
		Locale_Conversion.decimal_point = Dot;
		Locale_Conversion.thousands_sep = Comma;
		Locale_Conversion.grouping = Default_Grouping;
		Locale_Conversion.positive_sign = None;
		Locale_Conversion.frac_digits = 2;
		Locale_Conversion.currency_symbol = "$";
		Locale_Conversion.int_curr_symbol = Locale_Conversion.currency_symbol;
		Locale_Conversion.mon_decimal_point = Dot;
		Locale_Conversion.mon_thousands_sep = Comma;
		Locale_Conversion.mon_grouping = Default_Grouping;
		Locale_Conversion.p_cs_precedes = 1;
		Locale_Conversion.p_sep_by_space = 0;
		Locale_Conversion.n_cs_precedes = 1;
		Locale_Conversion.n_sep_by_space = 0;
		Locale_Conversion.p_sign_posn = 1;
		Locale_Conversion.n_sign_posn = 1;
		/*
		 *	Get the locale values from Windows and stuff them into the lconv structure
		 */
		for (p = Convert; p->Field; p++) {
			int n;
			char *cp;
			TCHAR Temp[64];

			/*
			 *	Get the value (always a string)
			 */
			n = GetLocaleInfo(LOCALE_USER_DEFAULT, p->GetLocaleInfo_Type, Temp, sizeof(Temp)/sizeof(Temp[0]));
			if (n > 0) {
				/*
				 *	Convert to ascii
				 */
				WideCharToMultiByte(CP_ACP, 0, Temp, n, (char *)Temp, sizeof(Temp), NULL, NULL);
				/*
				 *	Dispatch on lconv Type
				 */
				switch(p->lconv_Type) {
				  /*
				   *	Ignore anything we don't understand
				   */
				  default: continue;
				  /*
				   *	String
				   */
				  case TYPE_STRING:
					  /*
					   *	Just put it in dynamic memory and store it
					   */
					  break;
				  /*
				   *	Integer
				   */
				  case TYPE_INTEGER:
					  /*
					   *	Convert to a binary integer value into a char
					   */
					  *(char *)p->Field = 0;
					  n = 1;
					  cp = (char *)Temp;
					  while(*cp) {
						  *(char *)p->Field += (*cp++ - '0') * n;
						  n *= 10;
					  }
					  continue;
				  case TYPE_GROUPING: {
					  char *Grouping;
					  int Value;

					  /*
					   *	Convert to a binary representation of a grouping (in place)
					   */
					  Grouping = (char *)Temp;
					  cp = (char *)Temp;
					  while(*cp) {
						  /*
						   *	Convert to integer
						   */
						  Value = 0;
						  n = 1;
						  while(*cp && (*cp != ';')) {
							  Value += (*cp++ - '0') * n;
							  n *= 10;
						  }
						  /*
						   *	Skip the separator
						   */
						  if (*cp) cp++;
						  /*
						   *	Store the value
						   */
						  *Grouping++ = (char)Value;
					  }
					  /*
					   *	Store the Grouping terminator
					   */
					  *Grouping++ = 0;
					  /*
					   *	Put it in dynamic memory and store it
					   */
					  n = Grouping - (char *)Temp;
					  break;
				  }
				}
				/*
				 *	Allocate storage to hold the data
				 */
				cp = (char *)LocalAlloc(0, n + 1);
				if (cp) {
					/*
					 *	Copy to dynamic memory and store
					 */
					memcpy(cp, (char *)Temp, n);
					cp[n] = '\0';
					*(char **)p->Field = cp;
				}
			}
		}
		/*
		 *	Make int_frac_digits the same as frac_digits
		 */
		Locale_Conversion.int_frac_digits = Locale_Conversion.frac_digits;
		/*
		 *	Set the negative sign appropriately
		 */
		Locale_Conversion.negative_sign = Negative_Sign ? "-" : None;
	}
}
#endif	// 0

#include <locale.h>

static char C_LOCALE[] = "C";
static char C_DECIMAL_POINT[] = ".";
static char EMPTY_STRING[] = "";

static struct lconv C_LOCALE_INFO = {
	C_DECIMAL_POINT,	/* decimal_point */
	EMPTY_STRING,		/* thousands_sep */
	EMPTY_STRING,		/* grouping */
	EMPTY_STRING,		/* int_curr_symbol */
	EMPTY_STRING,		/* currency_symbol */
	EMPTY_STRING,		/* mon_decimal_point */
	EMPTY_STRING,		/* mon_thousands_sep */
	EMPTY_STRING,		/* mon_grouping */
	EMPTY_STRING,		/* positive_sign */
	EMPTY_STRING,		/* negative_sign */
	CHAR_MAX,		/* int_frac_digits */
	CHAR_MAX,		/* frac_digits */
	CHAR_MAX,		/* p_cs_precedes */
	CHAR_MAX,		/* p_sep_by_space */
	CHAR_MAX,		/* n_cs_precedes */
	CHAR_MAX,		/* n_sep_by_space */
	CHAR_MAX,		/* p_sign_posn */
	CHAR_MAX		/* n_sign_posn */
};

/*
 *	Missing localeconv() function
 */
struct lconv *localeconv(void)
{
	/*
	if (!Locale_Conversion.decimal_point) Get_Locale_Information();
	return(&Locale_Conversion);
	*/
	return &C_LOCALE_INFO;
}

/*
 *	Missing setlocale() function
 *	We only support the "C" locale.
 */
char *setlocale(int category, const char *locale)
{
	if(locale == NULL) {
		/* Get the current locale */
		return C_LOCALE;
	}
	/* Set the locale */
	if(strcmp(locale, "C") != 0) {
		return NULL;	/* Locale not supported */
	}
	return C_LOCALE;
}

/*
 *	Missing Win32 RegEnumKey() function
 */
__declspec(dllexport) long RegEnumKey(HKEY hKey, DWORD dwIndex, LPTSTR lpName, DWORD cchName)
{
	DWORD Length = cchName;

	/*
	 *	Make it into the RegEnumKeyEx() call
	 */
	return(RegEnumKeyEx(hKey, dwIndex, lpName, &Length, 0, NULL, NULL, NULL));
}


	/************************************************************************/
	/*									*/
	/*	Errno emulation:  There is no errno on Windows/CE and we need	*/
	/*			  to make it per-thread.  So we have a function	*/
	/*			  that returns a pointer to the errno for the	*/
	/*			  current thread.				*/
	/*									*/
	/*			  If there is ONLY the main thread then we can	*/
	/* 			  quickly return some static storage.		*/
	/*									*/
	/*			  If we have multiple threads running, we use	*/
	/*			  Thread-Local Storage to hold the pointer	*/
	/*									*/
	/************************************************************************/

/*
 *	_sys_nerr and _sys_errlist
 */
static char Undefined_Error[] = "Undefined error";
__declspec(dllexport) const char *_sys_errlist[] = {
	Undefined_Error,			/*  0 - ENOERROR */
	Undefined_Error,			/*  1 - EPERM */
	"No such file or directory",		/*  2 - ENOENT */
	Undefined_Error,			/*  3 - ESRCH */
	"Interrupted system call",		/*  4 - EINTR */
	Undefined_Error,			/*  5 - EIO */
	"Device not configured",		/*  6 - ENXIO */
	Undefined_Error,			/*  7 - E2BIG */
	Undefined_Error,			/*  8 - ENOEXEC */
	Undefined_Error,			/*  9 - EBADF */
	Undefined_Error,			/* 10 - ECHILD */
	Undefined_Error,			/* 11 - EDEADLK */
	Undefined_Error,			/* 12 - ENOMEM */
	"Permission denied",			/* 13 - EACCES */
	Undefined_Error,			/* 14 - EFAULT */
	Undefined_Error,			/* 15 - ENOTBLK */
	Undefined_Error,			/* 16 - EBUSY */
	"File exists",				/* 17 - EEXIST */
	Undefined_Error,			/* 18 - EXDEV */
	Undefined_Error,			/* 19 - ENODEV */
	Undefined_Error,			/* 20 - ENOTDIR */
	"Is a directory",			/* 21 - EISDIR */
	"Invalid argument",			/* 22 - EINVAL */
	};
__declspec(dllexport) int _sys_nerr = sizeof(_sys_errlist)/sizeof(_sys_errlist[0]);

/*
 *	Function pointer for returning errno pointer
 */
static int *Initialize_Errno(void);
__declspec(dllexport) int *(*__WinCE_Errno_Pointer_Function)(void) = Initialize_Errno;

/*
 *	Static errno storage for the main thread
 */
static int Errno_Storage = 0;

/*
 *	Thread-Local storage slot for errno
 */
static int TLS_Errno_Slot = 0xffffffff;

/*
 *	Number of threads we have running and critical section protection
 *	for manipulating it
 */
static int Number_Of_Threads = 0;
static CRITICAL_SECTION Number_Of_Threads_Critical_Section;

/*
 *	For the main thread only -- return the errno pointer
 */
static int *Get_Main_Thread_Errno(void)
{
	return(&Errno_Storage);
}

/*
 *	When there is more than one thread -- return the errno pointer
 */
static int *Get_Thread_Errno(void)
{
	return((int *)TlsGetValue(TLS_Errno_Slot));
}

/*
 *	Initialize a thread's errno
 */
static void Initialize_Thread_Errno(int *Errno_Pointer)
{
	/*
	 *	Make sure we have a slot
	 */
	if (TLS_Errno_Slot == 0xffffffff) {
		/*
		 *	No: Get one
		 */
		TLS_Errno_Slot = (int)TlsAlloc();
		if (TLS_Errno_Slot == 0xffffffff) abort();
	}
	/*
	 *	We can safely check for 0 threads, because
	 *	only the main thread will be initializing
	 *	at this point.  Make sure the critical
	 *	section that protects the number of threads
	 *	is initialized
	 */
	if (Number_Of_Threads == 0)
		InitializeCriticalSection(&Number_Of_Threads_Critical_Section);
	/*
	 *	Store the errno pointer
	 */
	if (TlsSetValue(TLS_Errno_Slot, (LPVOID)Errno_Pointer) == 0) abort();
	/*
	 *	Bump the number of threads
	 */
	EnterCriticalSection(&Number_Of_Threads_Critical_Section);
	Number_Of_Threads++;
	if (Number_Of_Threads > 1) {
		/*
		 *	We have threads other than the main thread:
		 *	  Use thread-local storage
		 */
		__WinCE_Errno_Pointer_Function = Get_Thread_Errno;
	}
	LeaveCriticalSection(&Number_Of_Threads_Critical_Section);
}

/*
 *	Initialize errno emulation on Windows/CE (Main thread)
 */
static int *Initialize_Errno(void)
{
	/*
	 *	Initialize the main thread's errno in thread-local storage
	 */
	Initialize_Thread_Errno(&Errno_Storage);
	/*
	 *	Set the errno function to be the one that returns the
	 *	main thread's errno
	 */
	__WinCE_Errno_Pointer_Function = Get_Main_Thread_Errno;
	/*
	 *	Return the main thread's errno
	 */
	return(&Errno_Storage);
}

/*
 *	Initialize errno emulation on Windows/CE (New thread)
 */
void __WinCE_Errno_New_Thread(int *Errno_Pointer)
{
	Initialize_Thread_Errno(Errno_Pointer);
}

/*
 *	Note that a thread has exited
 */
void __WinCE_Errno_Thread_Exit(void)
{
	/*
	 *	Decrease the number of threads
	 */
	EnterCriticalSection(&Number_Of_Threads_Critical_Section);
	Number_Of_Threads--;
	if (Number_Of_Threads <= 1) {
		/*
		 *	We only have the main thread
		 */
		__WinCE_Errno_Pointer_Function = Get_Main_Thread_Errno;
	}
	LeaveCriticalSection(&Number_Of_Threads_Critical_Section);
}

#if 0
	/****************************************************************/
	/*								*/
	/*	Power of 2 memory allocator:  Currently unused		*/
	/*								*/
	/****************************************************************/

struct Allocation_Freelist {
	struct Allocation_Freelist *Next;
	};
static struct Allocation_Freelist *Buckets[10] = {0};
#define	BASE_BUCKET		3	/* We start allocations at 8-bytes */
#define	ALLOCATION_HEADER_SIZE	8
#define	STORE_FREELIST(Allocation, Freelist) \
	*(struct Allocation_Freelist ***)((char *)Allocation - sizeof(long)) = Freelist
#define	GET_FREELIST(Allocation) \
	*(struct Allocation_Freelist ***)((char *)Allocation - sizeof(long))

static struct Allocation_Freelist *More_Allocations(struct Allocation_Freelist **Freelist)
{
	struct Allocation_Freelist *Allocation, *Next_Allocation, *Allocation_Listhead;
	int Bucket;
	int Allocation_Size;
	int i;
	static unsigned char Number_Of_Elements_To_Allocate[sizeof(Buckets)/sizeof(Buckets[0])] =
		{64, /*    8-byte allocations (  16 bytes each = 1024 bytes total) */
		 42, /*   16-byte allocations (  24 bytes each = 1008 bytes total) */
		 25, /*   32-byte allocations (  40 bytes each = 1000 bytes total) */
		 28, /*	  64-byte allocations (  72 bytes each = 2016 bytes total) */
		 15, /*  128-byte allocations ( 136 bytes each = 2040 bytes total) */
		  7, /*  256-byte allocations ( 264 bytes each = 1848 bytes total) */
		  4, /*  512-byte allocations ( 520 bytes each = 2080 bytes total) */
		  3, /* 1024-byte allocations (1032 bytes each = 3097 bytes total) */
		  1, /* 2048-byte allocations (2056 bytes each = 2056 bytes total) */
		  1};/* 4096-byte allocations (4104 bytes each = 4104 bytes total) */

	/*
	 *	Figure out which bucket we are in
	 */
	Bucket = Freelist - Buckets;
	/*
	 *	Calculate the allocation size
	 */
	Allocation_Size = (1 << (Bucket + BASE_BUCKET)) + ALLOCATION_HEADER_SIZE;
	/*
	 *	Do the allocation
	 */
	i = Number_Of_Elements_To_Allocate[Bucket];
	Allocation_Listhead = (struct Allocation_Freelist *)LocalAlloc(0, i * Allocation_Size);
	if (!Allocation_Listhead) return(0);
	/*
	 *	Make a chain of the allocations
	 */
	Allocation = Allocation_Listhead;
	while(--i > 0) {
		Next_Allocation = (struct Allocation_Freelist *)((char *)Allocation + Allocation_Size);
		Allocation->Next = Next_Allocation;
		Allocation = Next_Allocation;
	}
	/*
	 *	Add the chain to the freelist
	 */
	Allocation->Next = *Freelist;
	*Freelist = Allocation_Listhead;
	/*
	 *	Return the new head of the freelist
	 */
	return(Allocation_Listhead);
}

void *WinCE_malloc(int Size)
{
	struct Allocation_Freelist **Freelist, *Allocation;

	/*
	 *	Calculate which power of 2 bucket the size falls into
	 *	using a binary search
	 */
	if (Size <= 1024) {
		if (Size <= 64) {
			if (Size <= 16) {
				if (Size <= 8) {
					Freelist = &Buckets[0];	/* 0-8 bytes */
				} else {
					Freelist = &Buckets[1];	/* 9-16 bytes */
				}
			} else {
				if (Size <= 32) {
					Freelist = &Buckets[2];	/* 17-32 bytes */
				} else {
					Freelist = &Buckets[3];	/* 33-64 bytes */
				}
			}
		} else {
			if (Size <= 256) {
				if (Size <= 128) {
					Freelist = &Buckets[4];	/* 65-128 bytes */
				} else {
					Freelist = &Buckets[5];	/* 129-256 bytes */
				}
			} else {
				if (Size <= 512) {
					Freelist = &Buckets[6];	/* 257-512 bytes */
				} else {
					Freelist = &Buckets[7];	/* 513-1024 bytes */
				}
			}
		}
	} else {
		if (Size <= 4096) {
			if (Size <= 2048) {
				Freelist = &Buckets[8];		/* 1025-2048 bytes */
			} else {
				Freelist = &Buckets[9];		/* 2049-4096 bytes */
			}
		} else {
			/*
			 *	Directly allocate the memory from the O/S
			 *	(LocalAlloc leaves us 8-byte aligned, keep it that way)
			 */
			Allocation = (struct Allocation_Freelist *)LocalAlloc(0, Size + ALLOCATION_HEADER_SIZE);
			if (Allocation) {
				/*
				 *	Advance to the allocation area
				 */
				((char *)Allocation) += ALLOCATION_HEADER_SIZE;
				/*
				 *	Store in the allocation that there is no freelist
				 */
				STORE_FREELIST(Allocation, 0);
			}
			return((void *)Allocation);
		}
	}
	/*
	 *	Get the next allocation
	 */
	Allocation = *Freelist;
	if (!Allocation) {
		/*
		 *	Make more allocations
		 */
		Allocation = More_Allocations(Freelist);
		if (!Allocation) return(Allocation);
	}
	/*
	 *	Take it off the list
	 */
	*Freelist = Allocation->Next;
	/*
	 *	Advance to the allocation area
	 */
	((char *)Allocation) += ALLOCATION_HEADER_SIZE;
	/*
	 *	Store the freelist pointer in the allocation
	 */
	STORE_FREELIST(Allocation, Freelist);
	/*
	 *	Return the allocation
	 */
	return((void *)Allocation);
}

void WinCE_free(void *Ptr)
{
	struct Allocation_Freelist **Freelist, *Allocation;

	/*
	 *	Get the freelist from the pointer
	 */
	Allocation = (struct Allocation_Freelist *)Ptr;
	Freelist = GET_FREELIST(Allocation);
	/*
	 *	Get to the beginning of the allocation
	 */
	((char *)Allocation) -= ALLOCATION_HEADER_SIZE;
	/*
	 *	See if the allocation came from a freelist
	 */
	if (Freelist) {
		/*
		 *	Yes: Put it back on that freelist
		 */
		Allocation->Next = *Freelist;
		*Freelist = Allocation;
	} else {
		/*
		 *	No: Give it back to the O/S allocator
		 */
		LocalFree((void *)Allocation);
	}
}

void *WinCE_realloc(void *Ptr, int Size)
{
	struct Allocation_Freelist **Freelist;
	void *New_Allocation;

	/*
	 *	If the pointer is NULL, turn this into a malloc
	 */
	if (!Ptr) return(WinCE_malloc(Size));
	/*
	 *	Get the freelist from the pointer
	 */
	Freelist = GET_FREELIST(Ptr);
	if (Freelist) {
		int Bucket, Allocation_Size;

		/*
		 *	Figure out which bucket we are in
		 */
		Bucket = Freelist - Buckets;
		/*
		 *	Calculate the allocation space
		 */
		Allocation_Size = (1 << (Bucket + BASE_BUCKET));
		/*
		 *	If there is enough room, just return the pointer
		 */
		if (Size <= Allocation_Size) return(Ptr);
	}
	/*
	 *	Not enough room: Get a new allocation and copy the old data to it
	 */
	New_Allocation = WinCE_malloc(Size);
	if (New_Allocation && (Size > 0)) memcpy(New_Allocation, Ptr, Size);
	/*
	 *	Free the old allocation and return the new one
	 */
	WinCE_free(Ptr);
	return(New_Allocation);
}
#endif	/* 0 */

