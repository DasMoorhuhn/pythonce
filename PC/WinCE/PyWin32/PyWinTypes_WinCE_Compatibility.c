/*
 *	Windows/CE Compatibility code for PyWinTypes
 */
#include <windows.h>
#include "Python.h"

double PyCE_SystemTimeToCTime(SYSTEMTIME* pstTime)
{
	SYSTEMTIME stBase;
	FILETIME   ftTime;
	FILETIME   ftBase; 
	__int64    iTime;
	__int64    iBase;

	SystemTimeToFileTime(pstTime, &ftTime);

	stBase.wYear         = 1970;
	stBase.wMonth        = 1;
	stBase.wDayOfWeek    = 1;
	stBase.wDay          = 1;
	stBase.wHour         = 0;
	stBase.wMinute       = 0;
	stBase.wSecond       = 0;
	stBase.wMilliseconds = 0;
	SystemTimeToFileTime(&stBase, &ftBase);

	iTime=ftTime.dwHighDateTime;	
	iTime=iTime << 32;				
	iTime |= ftTime.dwLowDateTime;

	iBase=ftBase.dwHighDateTime;
	iBase=iBase << 32;
	iBase |= ftBase.dwLowDateTime;

	return (double)((iTime - iBase) / 10000000L);
}

BOOL PyCE_UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;
	ll = ((__int64)t * 10000000) + 116444736000000000;
//	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = (DWORD)(ll >> 32);
	return TRUE;
}

