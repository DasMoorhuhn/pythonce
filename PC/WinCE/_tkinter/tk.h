/*
 *	Minimal stuff we need to interface with TK
 */
#ifndef _TK
#define _TK

/*
 *	Define the Tk constants
 */
#define TK_MAJOR_VERSION   8
#define TK_MINOR_VERSION   4
#define TK_RELEASE_LEVEL   TCL_ALPHA_RELEASE
#define TK_RELEASE_SERIAL  2
#define TK_VERSION	"8.4"
#define TK_PATCH_LEVEL	"8.4a2"

/*
 *	Define the opaque typedefs required for the Tk functions
 */
typedef void *Tk_Window;

/*
 *	Define the dynamically loaded Tk functions
 */
extern struct _Tk_Dynload {
#define	Tk_MainWindow (*_Tk_Dynload._Tk_MainWindow)
Tk_Window (*_Tk_MainWindow)(Tcl_Interp * interp);

#define	Tk_Init (*_Tk_Dynload._Tk_Init)
int (*_Tk_Init)(Tcl_Interp * interp);

#define	Tk_GetNumMainWindows (*_Tk_Dynload._Tk_GetNumMainWindows)
int (*_Tk_GetNumMainWindows)(void);
} _Tk_Dynload;

#endif /* _TK */
