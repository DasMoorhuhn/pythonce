/*
 *	On Windows/CE we need to have our own errno.h
 */
#ifndef	_ERRNO_H
#define	_ERRNO_H

/*
 *	Errno values that Python needs
 */
#define	ENOENT	 2
#define	EINTR	 4
#define	ENXIO	 6
#define	EACCES	13
#define	EEXIST	17
#define	EISDIR	21
#define	EINVAL	22
#define	EDOM	33
#define	ERANGE	34

/*
 *	Because we need a per-thread errno, we define a function
 *	pointer that we can call to return a pointer to the errno
 *	for the current thread.  Then we define a macro for errno
 *	that dereferences this function's result.
 *
 *	This makes it syntactically just like the "real" errno.
 *
 *	Using a function pointer allows us to use a very fast
 *	function when there are no threads running and a slower
 *	function when there are multiple threads running.
 */
extern void __WinCE_Errno_New_Thread(int *Errno_Pointer);
extern void __WinCE_Errno_Thread_Exit(void);
extern __declspec(dllexport) int *(*__WinCE_Errno_Pointer_Function)(void);
#define	errno (*(*__WinCE_Errno_Pointer_Function)())

#endif	/* not __ERRNO_H */
