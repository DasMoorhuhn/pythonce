/*
 *	On Windows/CE we need to have our own time.h, because even though
 *	the SDK supplies a time.h the functions are not defined in any
 *	SDK libraries (i.e. the SDK headers are buggy)
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

extern DL_IMPORT(void) tzset(void);
extern DL_IMPORT(char *) tzname[2];
extern DL_IMPORT(long) timezone;
extern DL_IMPORT(int) daylight;

#ifndef _CLOCK_T_DEFINED    /* only if the SDK time.h does not define them */
typedef unsigned long clock_t;
#define _CLOCK_T_DEFINED
#endif  /* _CLOCK_T_DEFINED */

#define	CLOCKS_PER_SEC	1000	/* Someday when clock() works we'll get back Milliseconds of CPU time */

extern DL_IMPORT(clock_t) clock(void);
extern DL_IMPORT(struct tm *) gmtime(const time_t *);
extern DL_IMPORT(struct tm *) localtime(const time_t *);
extern DL_IMPORT(time_t) time(time_t *);
extern DL_IMPORT(char *) asctime(const struct tm *);
extern DL_IMPORT(char *) ctime(const time_t *);
extern DL_IMPORT(time_t) mktime(struct tm *);
extern DL_IMPORT(size_t) strftime(char *, size_t, const char *, const struct tm *);

#endif	/* _TIME_H */
