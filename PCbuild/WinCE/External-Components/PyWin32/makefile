#
#	PyWin32 makefile:  Make the individual components
#
#	Requires the following definitions:
#		PYWIN32ROOT - The root of the PyWin32 source hierarchy
#

#
#	How we do makes
#
NMAKE=nmake /nologo
#
#	This is the list of PyWin32 components we know how to build
#
ALL_COMPONENTS= pywintypes \
		win32event \
		win32gui   \
		win32process \
		wince

#
#	The default target is all the components
#
ALL: $(ALL_COMPONENTS)

#
#	Make sure we have the definitions we need
#
!IF "$(PYWIN32ROOT)" == ""
!ERROR PYWIN32ROOT is not defined: Cannot compile PyWin32 without knowing where its sources live
!ENDIF
!IF "$(SWIGROOT)" == ""
SWIGROOT=$(PYWIN32ROOT)\SWIG
!ENDIF

#
#	PyWinTypes:
#
pywintypes:
	@cd ..\..
	@$(NMAKE) -f External-Components\PyWin32\makefile.pywintypes
	@cd External-Components\PyWin32

#
#	Win32gui:
#
win32gui:
	@cd ..\..
	@$(NMAKE) -f External-Components\PyWin32\makefile.win32gui SWIGROOT=$(SWIGROOT)
	@cd External-Components\PyWin32

#
#	Win32event:
#
win32event:
	@cd ..\..
	@$(NMAKE) -f External-Components\PyWin32\makefile.win32event  SWIGROOT=$(SWIGROOT)
	@cd External-Components\PyWin32

#
#	win32process:
#
win32process:
	@cd ..\..
	@$(NMAKE) -f External-Components\PyWin32\makefile.win32process  SWIGROOT=$(SWIGROOT)
	@cd External-Components\PyWin32

#
#	wince:
#
wince:
	@cd ..\..
	@$(NMAKE) -f External-Components\PyWin32\makefile.wince  SWIGROOT=$(SWIGROOT)
	@cd External-Components\PyWin32

#
#	Clean up everything
#
clean:
	@cd ..\..
	@$(NMAKE) -f External-Components\PyWin32\makefile.pywintypes clean
	@$(NMAKE) -f External-Components\PyWin32\makefile.win32gui clean
	@$(NMAKE) -f External-Components\PyWin32\makefile.win32event clean
	@$(NMAKE) -f External-Components\PyWin32\makefile.win32process clean
	@$(NMAKE) -f External-Components\PyWin32\makefile.wince clean
	@cd External-Components\PyWin32


