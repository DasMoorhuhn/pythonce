/*
 *	On Windows/CE we need to have our own time.h
 */
#ifndef	_TIME_H
#define	_TIME_H	1

#ifndef _TM_DEFINED
#define _TM_DEFINED
struct tm {
        int tm_sec;     /* seconds after the minute - [0,59] */
        int tm_min;     /* minutes after the hour - [0,59] */
        int tm_hour;    /* hours since midnight - [0,23] */
        int tm_mday;    /* day of the month - [1,31] */
        int tm_mon;     /* months since January - [0,11] */
        int tm_year;    /* years since 1900 */
        int tm_wday;    /* days since Sunday - [0,6] */
        int tm_yday;    /* days since January 1 - [0,365] */
        int tm_isdst;   /* daylight savings time flag */
        };
#endif // _TM_DEFINED

__declspec(dllexport) void tzset(void);
__declspec(dllexport) extern char *tzname[2];
__declspec(dllexport) extern long timezone;
__declspec(dllexport) extern int daylight;

#endif	/* _TIME_H */
