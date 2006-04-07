
/* This code implemented by Mark Hammond (MHammond@skippinet.com.au) */

#include <windows.h>
#include <stdlib.h>
#include <limits.h>
#include <pydebug.h>

long PyThread_get_thread_ident(void);

/*
 * Change all headers to pure ANSI as no one will use K&R style on an
 * NT
 */

/*
 * Initialization of the C package, should not be needed.
 */
static void PyThread__init_thread(void)
{
}

/*
 *	There is only ONE function that is ever called to start
 *	a thread -- so we store it here
 */
static void (*Bootstrap_Function)(void *) = 0;

/*
 *	Here is where we REALLY start the thread
 */
static void Bootstrap_Thread(void *Arg)
{
	int Errno_Storage = 0;

	/*
	 *	Get errno off to a good start
	 */
	__WinCE_Errno_New_Thread(&Errno_Storage);
	/*
	 *	Enter the new thread
	 */
	(*Bootstrap_Function)(Arg);
	/*
	 *	Declare the thread dead
	 */
	__WinCE_Errno_Thread_Exit();
}


/*
 * Thread support.
 */
long PyThread_start_new_thread(void (*func)(void *), void *arg)
{
	long rv;
	int success = -1;

	Bootstrap_Function = func;
	dprintf(("%ld: PyThread_start_new_thread called\n", PyThread_get_thread_ident()));
	if (!initialized)
		PyThread_init_thread();

#if defined(_WIN32_WCE_EMULATION) || defined(WIN32PPC)
	{
		 SECURITY_ATTRIBUTES sa;

		 sa.nLength = sizeof(sa);
		 sa.lpSecurityDescriptor = (LPVOID) NULL;
		 sa.bInheritHandle = TRUE;
		 if(!CreateThread(&sa, 0, (LPTHREAD_START_ROUTINE) Bootstrap_Thread, (LPVOID)arg, 0, &rv)) 
			rv = -1;
	}

#else
	rv = _beginthread(Bootstrap_Thread, 0, Arg); /* use default stack size */
#endif
 
	if (rv != -1) {
		success = 0;
		dprintf(("%ld: PyThread_start_new_thread succeeded:\n", PyThread_get_thread_ident()));
	}

	return success;
}

/*
 * Return the thread Id instead of an handle. The Id is said to uniquely identify the
 * thread in the system
 */
long PyThread_get_thread_ident(void)
{
	if (!initialized)
		PyThread_init_thread();
        
	return GetCurrentThreadId();
}

static void do_PyThread_exit_thread(int no_cleanup)
{
	dprintf(("%ld: do_PyThread_exit_thread called\n", PyThread_get_thread_ident()));
 	__WinCE_Errno_Thread_Exit();	/* Declare the thread dead */
	if (!initialized)
		if (no_cleanup)
			exit(0); /* XXX - was _exit()!! */
		else
			exit(0);
#if defined(_WIN32_WCE_EMULATION) || defined(WIN32PPC)
	{
		unsigned long tExitCode;
 		GetExitCodeThread(GetCurrentThread(), &tExitCode);
		ExitThread(tExitCode);
	}

#else
	_endthread();
#endif
}

void PyThread_exit_thread(void)
{
	do_PyThread_exit_thread(0);
}

void PyThread__exit_thread(void)
{
	do_PyThread_exit_thread(1);
}

#ifndef NO_EXIT_PROG
static void do_PyThread_exit_prog(int status, int no_cleanup)
{
	dprintf(("PyThread_exit_prog(%d) called\n", status));
	if (!initialized)
		if (no_cleanup)
			_exit(status);
		else
			exit(status);
}

void PyThread_exit_prog(int status)
{
	do_PyThread_exit_prog(status, 0);
}

void PyThread__exit_prog(int status)
{
	do_PyThread_exit_prog(status, 1);
}
#endif /* NO_EXIT_PROG */

/*
 * Lock support. It has to be implemented using Mutexes, as
 * CE doesnt support semaphores.  Therefore we use some hacks to
 * simulate the non reentrant requirements of Python locks
 */
PyThread_type_lock PyThread_allocate_lock(void)
{
    HANDLE aLock;

    dprintf(("PyThread_allocate_lock called\n"));
    if (!initialized)
        PyThread_init_thread();

    aLock = CreateEvent(NULL,           /* Security attributes      */
                        0,              /* Manual-Reset               */
						1,              /* Is initially signalled  */
                        NULL);          /* Name of event            */

    dprintf(("%ld: PyThread_allocate_lock() -> %p\n", PyThread_get_thread_ident(), aLock));

    return (PyThread_type_lock) aLock;
}

void PyThread_free_lock(PyThread_type_lock aLock)
{
    dprintf(("%ld: PyThread_free_lock(%p) called\n", PyThread_get_thread_ident(),aLock));

    CloseHandle(aLock);
}

/*
 * Return 1 on success if the lock was acquired
 *
 * and 0 if the lock was not acquired. This means a 0 is returned
 * if the lock has already been acquired by this thread!
 */
int PyThread_acquire_lock(PyThread_type_lock aLock, int waitflag)
{
    int success = 1;
    DWORD waitResult;

    dprintf(("%ld: PyThread_acquire_lock(%p, %d) called\n", PyThread_get_thread_ident(),aLock, waitflag));

#ifndef DEBUG
    waitResult = WaitForSingleObject(aLock, (waitflag ? INFINITE : 0));
#else
	/* To aid in debugging, we regularly wake up.  This allows us to
	break into the debugger */
	while (TRUE) {
		waitResult = WaitForSingleObject(aLock, waitflag ? 3000 : 0);
		if (waitflag==0 || (waitflag && waitResult == WAIT_OBJECT_0))
			break;
	}
#endif

    if (waitResult != WAIT_OBJECT_0) {
		success = 0;    /* We failed */
    }

	dprintf(("%ld: PyThread_acquire_lock(%p, %d) -> %d\n", PyThread_get_thread_ident(),aLock, waitflag, success));

	return success;
}

void PyThread_release_lock(PyThread_type_lock aLock)
{
    dprintf(("%ld: PyThread_release_lock(%p) called\n", PyThread_get_thread_ident(),aLock));

    if (!SetEvent(aLock))
        dprintf(("%ld: Could not PyThread_release_lock(%p) error: %l\n", PyThread_get_thread_ident(), aLock, GetLastError()));
}


